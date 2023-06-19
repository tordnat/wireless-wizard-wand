#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/drivers/sensor.h>
#include "imu.h"
#include "forwarder.h"

int k_forwarder(){
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);
  struct sensor_value imu_acceleration[3];
	while (1) {
		// Wait between samples
		struct k_timer next_val_timer;
    k_timer_init(&next_val_timer, NULL, NULL);
    k_timer_start(&next_val_timer, K_USEC(TIME_BETWEEN_SAMPLES_US), K_NO_WAIT);
    
    imu_get_acceleration(imu_acceleration);
		printf("%.6f\t%.6f\t%.6f\t\r\n ",
		        sensor_value_to_double(&imu_acceleration[0]),
		        sensor_value_to_double(&imu_acceleration[1]),
            sensor_value_to_double(&imu_acceleration[2]));
    //Wait
    while (k_timer_status_get(&next_val_timer) <= 0);

	}
	return 0;
}