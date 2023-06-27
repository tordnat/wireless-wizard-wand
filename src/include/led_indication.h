#ifdef __cplusplus
extern "C" {
#endif
#ifndef _LED_INDICATION_H_
#define _LED_INDICATION_H_
#define LED_ON 1
#define LED_OFF 0

int leds_init(void);

int conn_led_set(int);
int conn_led_toggle(void);

int red_led_set(int);
int red_led_toggle(void);

#endif
#ifdef __cplusplus
}
#endif