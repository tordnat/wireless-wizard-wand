#ifndef _IMU_H_
#define _IMU_H_

// Globals
#define SAMPLING_FREQ 101 //Hz
#define TIME_BETWEEN_SAMPLES_US (1000000 / (SAMPLING_FREQ - 1))

void imu_get_acceleration(struct sensor_value* value);
void imu_init(void);


#endif