#ifndef UART_H_
#define UART_H_

#include <tm4c129encpdt.h>
#include <stdint.h>

void UART_init(uint32_t ui32Base);          // Initialize UART driver
char UART_getChar(void);                    // Receive one character
void UART_putChar(char c);                  // Transmit one character
void UART_reset(void);                      // Reset the driver to a save state (reset all registers)
void UART_putString(char *string);          // Uses UART_putChar to write a string
void UART_getString(char *string);          // Uses UART_getChar to read a string

#endif /* UART_H_ */
