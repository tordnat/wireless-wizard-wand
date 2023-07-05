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

#define STARTUP_DELAY_MS 2000

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void){
  usb_enable(NULL);
  k_sleep(K_MSEC(STARTUP_DELAY_MS));
  LOG_INF("Wireless Wizard Wand started on %s", CONFIG_BOARD);
  imu_init();
  k_zigbee();
	k_sleep(K_FOREVER);
}