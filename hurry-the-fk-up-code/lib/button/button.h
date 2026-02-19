#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>

#define BUTTON_RELEASED 0
#define BUTTON_PRESSED 1

void button_setup(void);
void button_update(void);
uint8_t button_get_state(void);

#endif
