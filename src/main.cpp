#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/usb_device.h>
#include "imu.h"
#include "ei.h"
#include "forwarder.h"
#include "zigbee.h"
#include "led_indication.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void){
  usb_enable(NULL);
  k_sleep(K_MSEC(2000));
  LOG_INF("Wireless Wizard Wand started on %s", CONFIG_BOARD);
  imu_init();
  k_sleep(K_MSEC(2000));
  k_forwarder();
  // k_zigbee();
  // k_ei();
	return 0;
}