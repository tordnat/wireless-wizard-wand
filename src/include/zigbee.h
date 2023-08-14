#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ZIGBEE_H_
#define _ZIGBEE_H_

#define LIGHT_SWITCH_ENDPOINT      1
//Delay between the light switch startup and light bulb finding procedure. 
#define MATCH_DESC_REQ_START_DELAY K_SECONDS(2)
//Timeout for finding procedure. 
#define MATCH_DESC_REQ_TIMEOUT     K_SECONDS(5)
//Find only non-sleepy device. 
#define MATCH_DESC_REQ_ROLE        ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE

/*Do not erase NVRAM to save the network parameters after device reboot or
 * power-off. NOTE: If this option is set to ZB_TRUE then do full device erase
 * for all network devices before running other samples.
*/ 
#define ERASE_PERSISTENT_CONFIG    ZB_FALSE
//Dim step size - increases/decreses current level (range 0x000 - 0xfe). 
#define DIMM_STEP                  15
//Transition time for a single step operation in 0.1 sec units. 0xFFFF - immediate change.
#define DIMM_TRANSACTION_TIME      2
//Time after which the button state is checked again to detect button hold, the dimm command is sent again.
#define BUTTON_LONG_POLL_TMO       K_MSEC(500)

int k_zigbee(void);
void send_light_on(void);
void send_light_off(void);

#endif

#ifdef __cplusplus
}
#endif
