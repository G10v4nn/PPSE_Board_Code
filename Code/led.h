#ifndef LED_INCLUDED
#define LED_INCLUDED

void init_led(void);
void turn_on_led(void);
void set_sequential_led(uint8_t values[], int n_leds);
#endif