#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <ram_pwrdn.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zigbee/zigbee_app_utils.h>
#include <zigbee/zigbee_error_handler.h>
#include <zb_nrf_platform.h>

#include "zigbee_memconfig.h"
#include "zigbee_dimmer.h"
#include "zigbee.h"
#include "led_indication.h"


LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

struct bulb_context {
	zb_uint8_t endpoint;
	zb_uint16_t short_addr;
	struct k_timer find_alarm;
};

struct buttons_context {
	uint32_t state;
	atomic_t long_poll;
	struct k_timer alarm;
};

struct zb_device_ctx {
	zb_zcl_basic_attrs_t basic_attr;
	zb_zcl_identify_attrs_t identify_attr;
};

static struct bulb_context bulb_ctx;
static struct buttons_context buttons_ctx;
static struct zb_device_ctx dev_ctx;

//Declare attribute list for Basic cluster (server). 
ZB_ZCL_DECLARE_BASIC_SERVER_ATTRIB_LIST(
	basic_server_attr_list,
	&dev_ctx.basic_attr.zcl_version,
	&dev_ctx.basic_attr.power_source);

//Declare attribute list for Identify cluster (client). 
ZB_ZCL_DECLARE_IDENTIFY_CLIENT_ATTRIB_LIST(
	identify_client_attr_list);

//Declare attribute list for Identify cluster (server). 
ZB_ZCL_DECLARE_IDENTIFY_SERVER_ATTRIB_LIST(
	identify_server_attr_list,
	&dev_ctx.identify_attr.identify_time);

//Declare attribute list for Scenes cluster (client). 
ZB_ZCL_DECLARE_SCENES_CLIENT_ATTRIB_LIST(
	scenes_client_attr_list);

//Declare attribute list for Groups cluster (client). 
ZB_ZCL_DECLARE_GROUPS_CLIENT_ATTRIB_LIST(
	groups_client_attr_list);

//Declare attribute list for On/Off cluster (client). 
ZB_ZCL_DECLARE_ON_OFF_CLIENT_ATTRIB_LIST(
	on_off_client_attr_list);

//Declare attribute list for Level control cluster (client). 
ZB_ZCL_DECLARE_LEVEL_CONTROL_CLIENT_ATTRIB_LIST(
	level_control_client_attr_list);

//Declare cluster list for Dimmer Switch device. 
ZB_DECLARE_DIMMER_SWITCH_CLUSTER_LIST(
	dimmer_switch_clusters,
	basic_server_attr_list,
	identify_client_attr_list,
	identify_server_attr_list,
	scenes_client_attr_list,
	groups_client_attr_list,
	on_off_client_attr_list,
	level_control_client_attr_list);

//Declare endpoint for Dimmer Switch device. 
ZB_DECLARE_DIMMER_SWITCH_EP(
	dimmer_switch_ep,
	LIGHT_SWITCH_ENDPOINT,
	dimmer_switch_clusters);

//Declare application's device context (list of registered endpoints)for Dimmer Switch device.
ZBOSS_DECLARE_DEVICE_CTX_1_EP(dimmer_switch_ctx, dimmer_switch_ep);

//Forward declarations. 
static void light_switch_button_handler(struct k_timer *timer);
static void find_light_bulb_alarm(struct k_timer *timer);
static void find_light_bulb(zb_bufid_t bufid);
static void light_switch_send_on_off(zb_bufid_t bufid, zb_uint16_t on_off);

///Starts identifying the device.
static void start_identifying(zb_bufid_t bufid)
{
	ZVUNUSED(bufid);

	if (ZB_JOINED()) {
		//Check if endpoint is in identifying mode, if not, put desired endpoint in identifying mode.
		 
		if (dev_ctx.identify_attr.identify_time ==
		    ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE) {

			zb_ret_t zb_err_code = zb_bdb_finding_binding_target(LIGHT_SWITCH_ENDPOINT);

			if (zb_err_code == RET_OK) {
				LOG_INF("Enter identify mode");
			} else if (zb_err_code == RET_INVALID_STATE) {
				LOG_WRN("RET_INVALID_STATE - Cannot enter identify mode");
			} else {
				ZB_ERROR_CHECK(zb_err_code);
			}
		} else {
			LOG_INF("Cancel identify mode");
			zb_bdb_finding_binding_target_cancel();
		}
	} else {
		LOG_WRN("Device not in a network - cannot enter identify mode");
	}
}

// Callback for button events.
static void button_handler(uint32_t button_state, uint32_t has_changed)
{
	zb_uint16_t cmd_id;
	zb_ret_t zb_err_code;

	//Inform default signal handler about user input at the device. 
	user_input_indicate();


	if (bulb_ctx.short_addr == 0xFFFF) {
		LOG_DBG("No bulb found yet.");
		return;
	}
}

static void alarm_timers_init(void)
{
	k_timer_init(&buttons_ctx.alarm, light_switch_button_handler, NULL);
	k_timer_init(&bulb_ctx.find_alarm, find_light_bulb_alarm, NULL);
}

// Function for initializing all clusters attributes. 
static void app_clusters_attr_init(void)
{
	//Basic cluster attributes data. 
	dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
	dev_ctx.basic_attr.power_source = ZB_ZCL_BASIC_POWER_SOURCE_UNKNOWN;

	//Identify cluster attributes data. 
	dev_ctx.identify_attr.identify_time = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;
}

static void toggle_identify_led(zb_bufid_t bufid)
{
	static int blink_status;
  conn_led_toggle();
	ZB_SCHEDULE_APP_ALARM(toggle_identify_led, bufid, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
}

/// Function to handle identify notification events on the first endpoint.
static void identify_cb(zb_bufid_t bufid)
{
	zb_ret_t zb_err_code;

	if (bufid) {
		//Schedule a self-scheduling function that will toggle the LED. 
		ZB_SCHEDULE_APP_CALLBACK(toggle_identify_led, bufid);
	} else {
		//Cancel the toggling function alarm and turn off LED. 
		zb_err_code = ZB_SCHEDULE_APP_ALARM_CANCEL(toggle_identify_led, ZB_ALARM_ANY_PARAM);
		ZVUNUSED(zb_err_code);

		// Update network status LED
		if (ZB_JOINED()) {
      conn_led_set(LED_ON);
		} else {
      conn_led_set(LED_OFF);
		}
	}
}

// Function for sending ON/OFF requests to the light bulb.
static void light_switch_send_on_off(zb_bufid_t bufid, zb_uint16_t cmd_id)
{
	LOG_INF("Send ON/OFF command: %d", cmd_id);

	ZB_ZCL_ON_OFF_SEND_REQ(bufid,
			       bulb_ctx.short_addr,
			       ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
			       bulb_ctx.endpoint,
			       LIGHT_SWITCH_ENDPOINT,
			       ZB_AF_HA_PROFILE_ID,
			       ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
			       cmd_id,
			       NULL);
}

// Function for sending step requests to the light bulb.
static void light_switch_send_step(zb_bufid_t bufid, zb_uint16_t cmd_id)
{
	LOG_INF("Send step level command: %d", cmd_id);

	ZB_ZCL_LEVEL_CONTROL_SEND_STEP_REQ(bufid,
					   bulb_ctx.short_addr,
					   ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
					   bulb_ctx.endpoint,
					   LIGHT_SWITCH_ENDPOINT,
					   ZB_AF_HA_PROFILE_ID,
					   ZB_ZCL_DISABLE_DEFAULT_RESPONSE,
					   NULL,
					   cmd_id,
					   DIMM_STEP,
					   DIMM_TRANSACTION_TIME);
}

// Callback function receiving finding procedure results.
static void find_light_bulb_cb(zb_bufid_t bufid)
{
	//Get the beginning of the response. 
	zb_zdo_match_desc_resp_t *resp =
		(zb_zdo_match_desc_resp_t *) zb_buf_begin(bufid);
	//Get the pointer to the parameters buffer, which stores APS layer response.
	 
	zb_apsde_data_indication_t *ind = ZB_BUF_GET_PARAM(bufid,
							   zb_apsde_data_indication_t);
	zb_uint8_t *match_ep;

	if ((resp->status == ZB_ZDP_STATUS_SUCCESS) &&
	    (resp->match_len > 0) &&
	    (bulb_ctx.short_addr == 0xFFFF)) {

		//Match EP list follows right after response header. 
		match_ep = (zb_uint8_t *)(resp + 1);

		//We are searching for exact cluster, so only 1 EP may be found.
		bulb_ctx.endpoint = *match_ep;
		bulb_ctx.short_addr = ind->src_addr;

		LOG_INF("Found bulb addr: %d ep: %d",
			bulb_ctx.short_addr,
			bulb_ctx.endpoint);

		k_timer_stop(&bulb_ctx.find_alarm);
    red_led_set(LED_ON);
	} else {
		LOG_INF("Bulb not found, try again");
	}

	if (bufid) {
		zb_buf_free(bufid);
	}
}

// Find bulb alarm handler.
static void find_light_bulb_alarm(struct k_timer *timer)
{
	ZB_ERROR_CHECK(zb_buf_get_out_delayed(find_light_bulb));
}

// Function for sending ON/OFF and Level Control find request.
static void find_light_bulb(zb_bufid_t bufid)
{
  // Set red led off to indicate no bulb connected
  red_led_set(LED_OFF);
	zb_zdo_match_desc_param_t *req;
	zb_uint8_t tsn = ZB_ZDO_INVALID_TSN;

	//Initialize pointers inside buffer and reserve space for zb_zdo_match_desc_param_t request.
	req = zb_buf_initial_alloc(bufid,
				   sizeof(zb_zdo_match_desc_param_t) + (1) * sizeof(zb_uint16_t));

	req->nwk_addr = MATCH_DESC_REQ_ROLE;
	req->addr_of_interest = MATCH_DESC_REQ_ROLE;
	req->profile_id = ZB_AF_HA_PROFILE_ID;

	//We are searching for 2 clusters: On/Off and Level Control Server. 
	req->num_in_clusters = 2;
	req->num_out_clusters = 0;
	req->cluster_list[0] = ZB_ZCL_CLUSTER_ID_ON_OFF;
	req->cluster_list[1] = ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL;

	//Set 0xFFFF to reset short address in order to parse only one response.
	bulb_ctx.short_addr = 0xFFFF;
	tsn = zb_zdo_match_desc_req(bufid, find_light_bulb_cb);

	//Free buffer if failed to send a request. 
	if (tsn == ZB_ZDO_INVALID_TSN) {
		zb_buf_free(bufid);

		LOG_ERR("Failed to send Match Descriptor request");
	}
}

// Callback for detecting button press duration.
static void light_switch_button_handler(struct k_timer *timer)
{
	zb_ret_t zb_err_code;
	zb_uint16_t cmd_id;

}

// Zigbee stack event handler
void zboss_signal_handler(zb_bufid_t bufid)
{
	zb_zdo_app_signal_hdr_t *sig_hndler = NULL;
	zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, &sig_hndler);
	zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);

	switch (sig) {
	case ZB_BDB_SIGNAL_DEVICE_REBOOT:
	//fall-through 
	case ZB_BDB_SIGNAL_STEERING:
		//Call default signal handler. 
		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		if (status == RET_OK) {
			//Check the light device address. 
			if (bulb_ctx.short_addr == 0xFFFF) {
				k_timer_start(&bulb_ctx.find_alarm,
					      MATCH_DESC_REQ_START_DELAY,
					      MATCH_DESC_REQ_TIMEOUT);
			}
		}
		break;
	case ZB_ZDO_SIGNAL_LEAVE:
		//If device leaves the network, reset bulb short_addr. 
		if (status == RET_OK) {
			zb_zdo_signal_leave_params_t *leave_params =
				ZB_ZDO_SIGNAL_GET_PARAMS(sig_hndler, zb_zdo_signal_leave_params_t);

			if (leave_params->leave_type == ZB_NWK_LEAVE_TYPE_RESET) {
				bulb_ctx.short_addr = 0xFFFF;
			}
		}
		//Call default signal handler. 
		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		break;

	default:
		//Call default signal handler. 
		ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
		break;
	}

	if (bufid) {
		zb_buf_free(bufid);
	}
}

void send_light_off(void){
      zb_buf_get_out_delayed_ext(light_switch_send_on_off,
				   ZB_ZCL_CMD_ON_OFF_OFF_ID, 0);
}

void send_light_on(void){
      zb_buf_get_out_delayed_ext(light_switch_send_on_off,
				   ZB_ZCL_CMD_ON_OFF_ON_ID, 0);
}

void send_light_dimm_up(void){
  zb_buf_get_out_delayed_ext(light_switch_send_step,
							 ZB_ZCL_LEVEL_CONTROL_STEP_MODE_UP,
							 0);
}

void send_light_dimm_down(void){
  zb_buf_get_out_delayed_ext(light_switch_send_step,
							 ZB_ZCL_LEVEL_CONTROL_STEP_MODE_DOWN,
							 0);
}

int k_zigbee(void)
{
	LOG_INF("Initializing Zigbee");

	//Initialize. 
	alarm_timers_init();
  leds_init();
	zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);
	zb_set_ed_timeout(ED_AGING_TIMEOUT_64MIN);
	zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(3000));

	//Set default bulb short_addr. 
	bulb_ctx.short_addr = 0xFFFF;


	//Register dimmer switch device context (endpoints). 
	ZB_AF_REGISTER_DEVICE_CTX(&dimmer_switch_ctx);

	app_clusters_attr_init();

	//Register handlers to identify notifications 
	ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(LIGHT_SWITCH_ENDPOINT, identify_cb);

	//Start Zigbee default thread. 
	zigbee_enable();
	LOG_INF("Zigbee thread started");
}
