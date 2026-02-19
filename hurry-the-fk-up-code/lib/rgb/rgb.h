#include <avr/io.h>

#define RED 1
#define GREEN 2
#define BLUE 3

void led_set_value(uint8_t led, uint8_t value);
int led_setup(void);
