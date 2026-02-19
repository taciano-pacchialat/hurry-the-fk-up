#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include <stddef.h>
#include <stdint.h>

#ifndef UART_DEBUG_BAUD
#define UART_DEBUG_BAUD 115200UL
#endif

#ifndef UART_DEBUG_TX_BUFFER_SIZE
#define UART_DEBUG_TX_BUFFER_SIZE 128U
#endif

#ifndef UART_DEBUG_PRINTF_BUFFER_SIZE
#define UART_DEBUG_PRINTF_BUFFER_SIZE 96U
#endif

void uart_debug_init(void);
uint8_t uart_debug_write_char(char value);
size_t uart_debug_write_data(const char *data, size_t length);
size_t uart_debug_write_str(const char *text);
size_t uart_debug_printf(const char *format, ...);

#endif