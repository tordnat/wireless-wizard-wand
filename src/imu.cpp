#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <imu.h>


static const struct device *imu_dev;
static void imu_configure();
static void imu_sample_accel();

//Get IMU acceleration
void imu_get_acceleration(struct sensor_value* value){ //May need error handling
  imu_sample_accel();
  sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_XYZ, value);
}

//Update IMU acceleration sample
static void imu_sample_accel(){
  int err;
  err = sensor_sample_fetch(imu_dev);
  if(err < 0){
    printf("BMI270 Sensor sample accelerometer update error\n");
  }
}

//Initialise IMU
void imu_init(){
    imu_dev = DEVICE_DT_GET_ONE(bosch_bmi270);
    if (imu_dev == NULL) {
        printf("Could not get BMI270 device\n");
    }
  imu_configure();
}

static void imu_configure(){
  struct sensor_value full_scale, imu_sampling_freq, oversampling;
  //Set sensor configuration
  full_scale.val1 = 2; //G
  full_scale.val2 = 0;
  imu_sampling_freq.val1 = SAMPLING_FREQ; //Hz
  imu_sampling_freq.val2 = 0;
  oversampling.val1 = 1; // BMI270: Normal Mode
  oversampling.val2 = 0;

  sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_FULL_SCALE,
      &full_scale);
  sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_OVERSAMPLING,
      &oversampling);
  //Set sampling frequency also sets power mode
  sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY,
      &imu_sampling_freq);
}
