#ifndef _IMU_H_
#define _IMU_H_

// IMU Config
#define IMU_ACCELEROMETER_SAMPLING_FREQ 100 //Hz
#define IMU_FULL_SCALE_RANGE 2 //G
#define IMU_OPERATION_MODE 1 // normal = 1

#define TIME_BETWEEN_SAMPLES_US (1000000 / (IMU_ACCELEROMETER_SAMPLING_FREQ)) //microseconds
#define IMU_AXIS_SAMPLED 3
#define SHARED_BUFFER_MUTEX_TIMEOUT 100 //ms

bool imu_start_sampling(float);
bool imu_init(void);
void imu_set_printf_data(bool);
#endif