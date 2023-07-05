#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <stdbool.h>

#include "imu.h"
#include "ei.h"

LOG_MODULE_REGISTER(imu_sensor, LOG_LEVEL_INF);

static const struct device *imu_dev;
float imu_data[IMU_AXIS_SAMPLED];
static bool imu_is_init = IMU_DEFAULT_IS_INIT; 
static bool imu_is_printf_data = IMU_DEFAULT_IS_PRINTF_DATA;
static bool imu_is_buffering_to_ei = IMU_DEFAULT_BUFFER_TO_EI;

//Forward declarations
static bool imu_configure();
static bool imu_fetch_sample_accel();
static void imu_timer_handler(struct k_timer *dummy);
static void imu_work_handler(struct k_work *work);
static void imu_set_buffering_to_ei(bool state);
static void imu_buffer_to_ei_handler(void);
static void imu_printf_data(void);

K_TIMER_DEFINE(imu_timer, imu_timer_handler, NULL);
K_WORK_DEFINE(imu_work, imu_work_handler);

//Submit work to sample for each time period
static void imu_timer_handler(struct k_timer *dummy){
  k_work_submit(&imu_work);
}

static void imu_work_handler(struct k_work *work){
	if(imu_fetch_sample_accel() == false) { //Fetch sample
    imu_data[0] = 0.0f;
    imu_data[1] = 0.0f;
    imu_data[2] = 0.0f;
	}
  
  if (imu_is_buffering_to_ei) imu_buffer_to_ei_handler();
  if (imu_is_printf_data) imu_printf_data();
}

static void imu_buffer_to_ei_handler(){
  if (ei_fill_feature_buffer_cb(imu_data) == true){ //Pass imu_data to EI
    LOG_DBG("Buffer full");
    imu_set_buffering_to_ei(false);
    imu_stop_sampling();
  }
}

bool imu_start_sampling_w_ei(float timer_period_us){
  imu_set_buffering_to_ei(true);
  return imu_start_sampling(timer_period_us);
}



bool imu_start_sampling(float timer_period_us){
  //Start timer with given period and expire on given period
  k_timer_start(&imu_timer, K_USEC(timer_period_us), K_USEC(timer_period_us));
  return true;
}

bool imu_stop_sampling(){
  k_timer_stop(&imu_timer);
  return true;
}

//Update IMU acceleration sample
static bool imu_fetch_sample_accel(){
  struct sensor_value raw_sample[IMU_AXIS_SAMPLED];
  if (imu_is_init == false){
    LOG_ERR("IMU Initialization error");
    return false;
  }

  if (sensor_sample_fetch(imu_dev) < 0){
    LOG_ERR("BMI270 Sample fetch error");
    return false;
  }

  if (sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_XYZ, raw_sample) < 0){
    LOG_ERR("Sensor Channel Get Error: Failed to get acceleration readings");
    return false;
  }
  // !! Conversion to float is done here because of low sampling period in application
  imu_data[0] = (float)sensor_value_to_double(&raw_sample[0]);
  imu_data[1] = (float)sensor_value_to_double(&raw_sample[1]);
  imu_data[2] = (float)sensor_value_to_double(&raw_sample[2]);

  return true;
}


//Initialise IMU
bool imu_init(){
  imu_dev = DEVICE_DT_GET_ONE(bosch_bmi270);
  if (imu_dev == NULL) {
    LOG_ERR("Could not get BMI270 device");
    return false;
  }
  int ret = imu_configure();
  if (ret == false){
    LOG_ERR("IMU Configuration Failed");
    return false;
  }
  imu_is_init = true;
  LOG_INF("IMU Initialized");
  return true;
}


static bool imu_configure(){
  struct sensor_value full_scale, imu_sampling_freq, oversampling;
  int ret0, ret1, ret2;

  //Set sensor configuration
  full_scale.val1 = IMU_FULL_SCALE_RANGE; //G
  full_scale.val2 = 0;
  imu_sampling_freq.val1 = IMU_ACCELEROMETER_SAMPLING_FREQ; //Hz
  imu_sampling_freq.val2 = 0;
  oversampling.val1 = IMU_OPERATION_MODE;
  oversampling.val2 = 0;

  ret0 = sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_FULL_SCALE,
      &full_scale);
  ret1 = sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_OVERSAMPLING,
      &oversampling);
  //Set sampling frequency also sets power mode
  ret2 = sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY,
      &imu_sampling_freq);

  if (ret0 == 0 && ret1 == 0 && ret2 == 0){
    LOG_INF("IMU Configuration:");
    LOG_INF("Scale: %d G", full_scale.val1); 
    LOG_INF("Sampling freq: %d Hz", imu_sampling_freq.val1);
    LOG_INF("Mode (1 = normal): %d", oversampling.val1); 
    return true;
  } else {
    return false;
  }
}

// Utilities
static void imu_printf_data(){
  printf("%.6f\t%.6f\t%.6f\t\r\n ",
          imu_data[0],
          imu_data[1],
          imu_data[2]); 
}

void imu_set_printf_data(bool val){
  LOG_INF("IMU Print Data: %d", val);
  imu_is_printf_data = val; 
}

static void imu_set_buffering_to_ei(bool state){
  imu_is_buffering_to_ei = state;
}