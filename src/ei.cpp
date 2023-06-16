#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include "ei_run_classifier.h"
#include "imu.h"


// to classify 1 frame of data you need EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE values
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
volatile struct sensor_value imu_acceleration;
int main() {
  // output immediately without buffering
  usb_enable(NULL);
  setvbuf(stdout, NULL, _IONBF, 0);
  
  imu_init();
  struct sensor_value accel[3];

  while (1) {
      for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
          // start a timer that expires when we need to grab the next value
          struct k_timer next_val_timer;
          k_timer_init(&next_val_timer, NULL, NULL);
          k_timer_start(&next_val_timer, K_USEC(TIME_BETWEEN_SAMPLES_US), K_NO_WAIT);

          imu_get_acceleration(accel);

          // fill the features array
          features[ix + 0] = sensor_value_to_double(&accel[0]);
          features[ix + 1] = sensor_value_to_double(&accel[1]);
          features[ix + 2] = sensor_value_to_double(&accel[2]);
          while (k_timer_status_get(&next_val_timer) <= 0);
      }

      // frame full? then classify
      ei_impulse_result_t result = { 0 };

      // create signal from features frame
      signal_t signal;
      numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

      // run classifier
      EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
      printf("run_classifier returned: %d\n", res);
      if (res != 0) return 1;

      // print predictions
      printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
          result.timing.dsp, result.timing.classification, result.timing.anomaly);

      // print the predictions
      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
          printf("%s:\t%.5f\n", result.classification[ix].label, result.classification[ix].value);
      }
  #if EI_CLASSIFIER_HAS_ANOMALY == 1
      printf("anomaly:\t%.3f\n", result.anomaly);
  #endif
    }
}