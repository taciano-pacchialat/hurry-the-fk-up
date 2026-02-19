#include "button.h"

#define BUTTON_DEBOUNCE_TICKS 10

static uint8_t debounced_state = BUTTON_RELEASED;
static uint8_t last_raw_state = BUTTON_RELEASED;
static uint8_t debounce_counter = 0;

static uint8_t button_read_raw(void)
{
    return (PINB & (1 << PINB0)) ? BUTTON_RELEASED : BUTTON_PRESSED;
}

void button_setup(void)
{
    DDRB &= ~(1 << DDB0);
    PORTB |= (1 << PORTB0);

    debounced_state = button_read_raw();
    last_raw_state = debounced_state;
    debounce_counter = 0;
}

void button_update(void)
{
    uint8_t raw_state = button_read_raw();

    if (raw_state != last_raw_state)
    {
        last_raw_state = raw_state;
        debounce_counter = 0;
        return;
    }

    if (debounce_counter < BUTTON_DEBOUNCE_TICKS)
    {
        debounce_counter++;

        if (debounce_counter >= BUTTON_DEBOUNCE_TICKS)
            debounced_state = raw_state;
    }
}

uint8_t button_get_state(void)
{
    return debounced_state;
}
