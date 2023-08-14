#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/mutex.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "imu.h"
#include "zigbee.h"
#include "ei_run_classifier.h"
#include "ei.h"

#define SAMPLE_TIME_MINIMUM (((EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE/IMU_AXIS_SAMPLED))*(TIME_BETWEEN_SAMPLES_US))
#define SAMPLING_TIME_LIMIT_US SAMPLE_TIME_MINIMUM*2

LOG_MODULE_REGISTER(edge_impulse, LOG_LEVEL_INF);
K_THREAD_DEFINE(ei_thread, EI_STACK_SIZE, k_ei,
                NULL, NULL, NULL,
                EI_THREAD_PRIORITY, 0, 0);
K_MUTEX_DEFINE(feature_buffer_mutex);

// to classify 1 frame of data you need EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE values
struct FeatureBuffer {
  int index;
  bool is_full;
  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
};

//Forward declarations
static void dummy_ei_result_cb(ei_impulse_result_t *result);
static void ei_event_handler(bool timeout, ei_impulse_result_t *result, FeatureBuffer *f_buff);
static bool ei_classify(FeatureBuffer *f_buff, ei_impulse_result_t *result);
static bool ei_sampling_timeout();
static void ei_log_predictions(ei_impulse_result_t *result);
static bool copy_to_features_buffer(float* sample, FeatureBuffer *f_buff);
static void clear_feature_buffer(FeatureBuffer *f_buff);


FeatureBuffer features = {0, false}; //Global shared buffer

int k_ei() {
  LOG_INF("Started Edge Impulse thread");
  while (1) {
    ei_impulse_result_t result = { 0 };
    clear_feature_buffer(&features);
    imu_start_sampling_w_ei(TIME_BETWEEN_SAMPLES_US);
    ei_event_handler(ei_sampling_timeout(), &result, &features);
  }
}

static void ei_event_handler(bool timeout, ei_impulse_result_t *result, FeatureBuffer *f_buff){
  if (!timeout && ei_classify(f_buff, result)){
    ei_log_predictions(result);
    dummy_ei_result_cb(result);
  }
}

static bool ei_sampling_timeout(){
  if(k_sleep(K_USEC(SAMPLING_TIME_LIMIT_US)) == 0){
    LOG_ERR("Sampling timed out");
    return true;
  }
  return false;
}

static void dummy_ei_result_cb(ei_impulse_result_t *result){
  LOG_DBG("WARNING: calling DUMMY MODULE, do not deploy.");
  if (result->classification[0].value > (float)0.99){
    send_light_on();
  }
  if(result->classification[2].value > (float)0.99){
    send_light_off();
  }
}

static bool ei_classify(FeatureBuffer *f_buff, ei_impulse_result_t *result){
  k_mutex_lock(&feature_buffer_mutex, K_FOREVER);
  // create signal from features frame
  signal_t signal;
  numpy::signal_from_buffer(f_buff->buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  EI_IMPULSE_ERROR res = run_classifier(&signal, result, false);
  LOG_DBG("run_classifier returned: %d\n", res);
  if(res != 0){
    LOG_ERR("run_classifier failed");
    k_mutex_unlock(&feature_buffer_mutex);
    return false;
  }
  k_mutex_unlock(&feature_buffer_mutex);
  return true;
}

static void ei_log_predictions(ei_impulse_result_t *result){
  LOG_DBG("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
        result->timing.dsp, result->timing.classification, result->timing.anomaly);
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    LOG_DBG("%s:\t%.5f\n", result->classification[ix].label, result->classification[ix].value);
  }
  #if EI_CLASSIFIER_HAS_ANOMALY == 1
    LOG_INF("anomaly:\t%.3f\n", result->anomaly);
  #endif
}

bool ei_fill_feature_buffer_cb(float* sample){
  if (k_mutex_lock(&feature_buffer_mutex, K_MSEC(SHARED_BUFFER_MUTEX_TIMEOUT)) == 0){ //Avoid deadlock
    bool ret = copy_to_features_buffer(sample, &features);
    if(ret == true){
      LOG_DBG("Buffer full");
      k_wakeup(ei_thread);
    }
    k_mutex_unlock(&feature_buffer_mutex);
    return ret;
  } else {
    LOG_ERR("Cannot lock feature buffer mutex");
    return false; //Since buffer is not written to, buffer is not filled
  }
}

static bool copy_to_features_buffer(float* sample, FeatureBuffer *f_buff){
  if (f_buff->is_full){
    return true;
  }
  f_buff->buffer[f_buff->index + 0] = sample[0];
  f_buff->buffer[f_buff->index + 1] = sample[1];
  f_buff->buffer[f_buff->index + 2] = sample[2];

  f_buff->index += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;

  if(f_buff->index >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE){
    f_buff->is_full = true;
    return true; //Buffer is now full
  }
  return false; //Buffer is not full
}

//Clears the feature buffer for overwriting
static void clear_feature_buffer(FeatureBuffer *f_buff){
  f_buff->index = 0;
  f_buff->is_full = false;
}
