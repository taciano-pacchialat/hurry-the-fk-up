#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "mef.h"
#include "rgb.h"
#include "buzzer.h"
#include "uart_debug.h"
#include "button.h"

volatile uint8_t tick_10ms_flag = 0;

ISR(TIMER0_COMPA_vect)
{
  tick_10ms_flag = 1;
}

static void setup_timer0_10ms(void)
{
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS02) | (1 << CS00);
  OCR0A = 156;
  TIMSK0 |= (1 << OCIE0A);
}

int main(void)
{
  uart_debug_init();
  setup_timer0_10ms();
  sei();
  mef_init();

  uart_debug_printf("Boot complete.\n");

  while (1)
  {
    if (tick_10ms_flag)
    {
      tick_10ms_flag = 0;
      mef_update_10ms();
    }
  }

  return 0;
}
