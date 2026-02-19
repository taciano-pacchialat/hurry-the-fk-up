#include "rgb.h"
#include <avr/interrupt.h>

volatile uint8_t blue_value = 0;
volatile uint8_t blue_phase = 0;

ISR(TIMER2_COMPA_vect)
{
    if (blue_phase < blue_value)
        PORTB |= (1 << PORTB4);
    else
        PORTB &= ~(1 << PORTB4);

    blue_phase++;
}

void led_set_value(uint8_t led, uint8_t value)
{
    if (value >= 0 && value <= 255)
    {
        if (led == RED)
            // Rojo = PIN 9 (PB1)
            OCR1A = value;
        else if (led == GREEN)
            // Verde = PIN 10 (PB2)
            OCR1B = value;
        else if (led == BLUE)
            // Azul = PIN 12 (PB4)
            blue_value = value;
    }
}

int led_setup(void)
{
    DDRB |= (1 << DDB1) | (1 << DDB2) | (1 << DDB4); // Pin 9, 10 y 12 como salida
    PORTB &= ~(1 << PORTB4);                         // Azul apagado por defecto

    // Timers para PWM
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10); // PWM en PB1 y 2
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);     // Frecuencia de 976Hz

    // Timer2 en CTC a 250kHz para PWM software de 8 bits (250k / 256 = 976Hz)
    TCCR2A = (1 << WGM21);
    TCCR2B = (1 << CS22);
    OCR2A = 0;
    TIMSK2 |= (1 << OCIE2A);

    OCR1A = OCR1B = 0;
    return 1;
}