#include "buzzer.h"

void buzzer_setup(void)
{
    // Buzzer por pin 11 (PB3)
    DDRB |= (1 << DDB3);
    PORTB &= ~(1 << PORTB3);
}

void buzzer_silence(void)
{
    PORTB &= ~(1 << PORTB3);
}

void buzzer_sound(void)
{
    PORTB |= (1 << PORTB3);
}