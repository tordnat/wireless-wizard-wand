#ifndef _IMU_H_
#define _IMU_H_

// Globals
#define SAMPLING_FREQ 101 //Hz
#define TIME_BETWEEN_SAMPLES_US (1000000 / (SAMPLING_FREQ - 1)) //microseconds
#define IMU_AXIS_SAMPLED 3
#define SHARED_BUFFER_MUTEX_TIMEOUT 100 //ms

bool imu_start_sampling(float);
bool imu_init(void);
void imu_set_printf_data(bool);
#endif