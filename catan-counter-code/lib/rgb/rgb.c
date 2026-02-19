#include "rgb.h"

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
    }
}

int led_setup(void)
{
    DDRB |= (1 << DDB1) | (1 << DDB2); // Pin 9 y 10 como salida
    DDRB &= ~(1 << DDB0);              // Pin 8 como entrada (botón)
    PORTB |= 0x01;                     // Activo R de pull-up
    // Timers para PWM
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10); // PWM en PB1 y 2
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);     // Frecuencia de 976Hz
    OCR1A = OCR1B = 0;
    return 1;
}