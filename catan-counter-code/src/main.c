#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "rgb.h"
#include "buzzer.h"
#include "uart_debug.h"

uint8_t get_button_state(void)
{
  // 1 si está presionado
  return !(PINB & (1 << PINB0));
}

int main(void)
{
  uart_debug_init();
  led_setup();
  buzzer_setup();
  sei();

  uart_debug_printf("Debug UART ready\r\n");

  led_set_value(RED, 255);
  led_set_value(GREEN, 255);
  buzzer_sound();

  while (1)
  {
    buzzer_sound();
    uart_debug_printf("Button state: %u\r\n", get_button_state());
    _delay_ms(250);
    buzzer_silence();
    _delay_ms(250);
  }

  return 0;
}
