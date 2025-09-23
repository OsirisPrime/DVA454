#ifndef UART_FUNCTIONS_H
#define UART_FUNCTIONS_H

#include <stdint.h>

void ConfigureUART(void);
void UART_Send(const char *str);
void UART_Receive(char *buffer, uint32_t size);

#endif
