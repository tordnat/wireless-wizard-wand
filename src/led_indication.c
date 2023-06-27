#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "led_indication.h"
#define RED_LED DT_ALIAS(led0)
#define BLUE_LED DT_ALIAS(led1)

static const struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(RED_LED, gpios);
static const struct gpio_dt_spec conn_led = GPIO_DT_SPEC_GET(BLUE_LED, gpios);

int leds_init(void){
  int err;
  err = gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_ACTIVE);
  err += gpio_pin_configure_dt(&conn_led, GPIO_OUTPUT_ACTIVE);
  if (err < 0) return 0;
  return err;
}

int conn_led_set(int value){
  int ret;
  if (!gpio_is_ready_dt(&conn_led)){
    return 0;
  }
  ret = gpio_pin_set_dt(&conn_led, value);
}

int conn_led_toggle(){
  int ret;
  if (!gpio_is_ready_dt(&conn_led)){
    return 0;
  }
  ret = gpio_pin_toggle_dt(&conn_led);
  return ret;

}

int red_led_set(int value){
  int ret;
  if (!gpio_is_ready_dt(&red_led)){
    return 0;
  }
  ret = gpio_pin_set_dt(&red_led, value);
}

int red_led_toggle(){
  int ret;
  if (!gpio_is_ready_dt(&red_led)){
    return 0;
  }
  ret = gpio_pin_toggle_dt(&red_led);
  return ret;

}
