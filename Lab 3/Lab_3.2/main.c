#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <driverlib/timer.h>

#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

#include "stopwatch.h"
#include "uart_functions.h"

#define BUFFER_SIZE 16


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
 while(1);
}
#endif


int main(void)
{
    uint8_t hh, mm, ss;
    char Buffer[BUFFER_SIZE];

    // Initialize UART and create a stopwatch
    ConfigureUART();
    create_stopwatch();
    IntMasterEnable();                      // Enable UART 0.

    UART_SendString(
            "\n\rStopwatch created!\n\r"
            "Available commands:\n\r"
            "1) Set initial time (Default 00:00:00)\n\r"
            "2) Start stopwatch\n\r"
            "3) Stop stopwatch\n\r"
            "4) Update stopwatch\n\r"
            "5) Reset stopwatch\n\n\r");

    while(1)
    {
        switch(UARTCharGet(UART0_BASE))
        {
        case '1':
            stop_stopwatch();
            UART_SendString("\n\rEnter initial time (hh:mm:ss): ");
            UART_ReceiveString(Buffer, BUFFER_SIZE);
            if (sscanf(Buffer, "%hhu:%hhu:%hhu", &hh, &mm, &ss) == 3) {
                initialize_stopwatch(hh, mm, ss);
                //UART_SendFormatted("\rStopwatch initialized to %02d:%02d:%02d\r\n", hh, mm, ss);
            }
            else {
                UART_SendString("\n\rInvalid format. Use: set hh:mm:ss\n\r");
            }
            break;

        case '2':
            start_stopwatch();
            break;

        case '3':
            stop_stopwatch();
            break;

        case '4':
            stop_stopwatch();
            UART_SendString("\n\rEnter new time (hh:mm:ss): ");
            UART_ReceiveString(Buffer, BUFFER_SIZE);
            if (sscanf(Buffer, "%hhu:%hhu:%hhu", &hh, &mm, &ss) == 3) {
                update_stopwatch(hh, mm, ss);
                //UART_SendFormatted("\n\rStopwatch updated to %02d:%02d:%02d\n\r", hh, mm, ss);
            }
            else {
                UART_SendString("\n\rInvalid format. Use: set hh:mm:ss\n\r");
            }
            break;

        case '5':
            reset_stopwatch();
            start_stopwatch();
            break;

        default:
            UART_SendString("\n\rInvalid input\n\r");
            break;

        }
    }
}
