#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include "imu.h"
#include "forwarder.h"

LOG_MODULE_REGISTER(data_forwarder, LOG_LEVEL_INF);

int k_forwarder(){
  LOG_DBG("Started Data Forwarder module");
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);
  imu_start_sampling(TIME_BETWEEN_SAMPLES_US);    
  imu_set_printf_data(true);
  k_sleep(K_FOREVER);
  return 0;
}