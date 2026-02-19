#include "uart_debug.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdarg.h>
#include <stdio.h>

#if (UART_DEBUG_TX_BUFFER_SIZE < 2U) || (UART_DEBUG_TX_BUFFER_SIZE > 255U)
#error "UART_DEBUG_TX_BUFFER_SIZE must be between 2 and 255"
#endif

static volatile uint8_t uart_debug_tx_head = 0;
static volatile uint8_t uart_debug_tx_tail = 0;
static volatile char uart_debug_tx_buffer[UART_DEBUG_TX_BUFFER_SIZE];

static inline uint8_t uart_debug_next_index(uint8_t index)
{
    index++;
    if (index >= UART_DEBUG_TX_BUFFER_SIZE)
    {
        index = 0;
    }

    return index;
}

void uart_debug_init(void)
{
    uint16_t ubrr_value = (uint16_t)((F_CPU / (8UL * UART_DEBUG_BAUD)) - 1UL);

    uart_debug_tx_head = 0;
    uart_debug_tx_tail = 0;

    UCSR0A = (1 << U2X0);
    UBRR0H = (uint8_t)(ubrr_value >> 8);
    UBRR0L = (uint8_t)(ubrr_value & 0xFF);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    UCSR0B = (1 << TXEN0);
}

uint8_t uart_debug_write_char(char value)
{
    uint8_t next_head;
    uint8_t status_register = SREG;

    cli();
    next_head = uart_debug_next_index(uart_debug_tx_head);
    if (next_head == uart_debug_tx_tail)
    {
        SREG = status_register;
        return 0;
    }

    uart_debug_tx_buffer[uart_debug_tx_head] = value;
    uart_debug_tx_head = next_head;
    UCSR0B |= (1 << UDRIE0);
    SREG = status_register;

    return 1;
}

size_t uart_debug_write_data(const char *data, size_t length)
{
    size_t index;
    size_t bytes_queued = 0;

    if (data == NULL)
    {
        return 0;
    }

    for (index = 0; index < length; index++)
    {
        if (!uart_debug_write_char(data[index]))
        {
            break;
        }

        bytes_queued++;
    }

    return bytes_queued;
}

size_t uart_debug_write_str(const char *text)
{
    size_t bytes_queued = 0;

    if (text == NULL)
    {
        return 0;
    }

    while (*text != '\0')
    {
        if (!uart_debug_write_char(*text))
        {
            break;
        }

        bytes_queued++;
        text++;
    }

    return bytes_queued;
}

size_t uart_debug_printf(const char *format, ...)
{
    va_list args;
    int formatted_length;
    size_t bytes_to_queue;
    char buffer[UART_DEBUG_PRINTF_BUFFER_SIZE];

    if (format == NULL)
    {
        return 0;
    }

    va_start(args, format);
    formatted_length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (formatted_length <= 0)
    {
        return 0;
    }

    bytes_to_queue = (size_t)formatted_length;
    if (bytes_to_queue >= sizeof(buffer))
    {
        bytes_to_queue = sizeof(buffer) - 1U;
    }

    return uart_debug_write_data(buffer, bytes_to_queue);
}

ISR(USART_UDRE_vect)
{
    if (uart_debug_tx_head == uart_debug_tx_tail)
    {
        UCSR0B &= ~(1 << UDRIE0);
        return;
    }

    UDR0 = (uint8_t)uart_debug_tx_buffer[uart_debug_tx_tail];
    uart_debug_tx_tail = uart_debug_next_index(uart_debug_tx_tail);
}