#ifndef UART_FUNCTIONS_H
#define UART_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>
extern volatile char g_latestChar;
extern volatile bool g_charReceived;

void ConfigureUART(void);
void UART_Send(const char *str);
void UART_Receive(char *buffer, uint32_t size);

#endif
