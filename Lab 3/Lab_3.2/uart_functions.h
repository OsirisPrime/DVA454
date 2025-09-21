#ifndef UART_FUNCTIONS_H
#define UART_FUNCTIONS_H

#include <stdint.h>
#include <stdarg.h>

void ConfigureUART(void);
void UART_SendString(const char *str);
void UART_ReceiveString(char *buffer, uint32_t size);
void UART_SendFormatted(const char *fmt, ...);

#endif
