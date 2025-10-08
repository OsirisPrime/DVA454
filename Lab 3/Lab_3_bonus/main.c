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

// UART interrupt global variables
volatile char g_latestChar = 0;
volatile bool g_charReceived = false;


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
 while(1);
}
#endif


int main(void)
{
    uint8_t hh, mm, ss;                     // Time variable
    char Buffer[BUFFER_SIZE];               // Buffer for UART

    ConfigureUART();                        // initialize and configure UART
    create_stopwatch();                     // Create a stopwatch (starts by default at 00:00:00)
    IntMasterEnable();                      // Enable the processor to respond to interrupts.

    UART_Send(
            "\n\rStopwatch created!\n\r"
            "Available commands:\n\r"
            "1) Set initial time (Default 00:00:00)\n\r"
            "2) Start stopwatch\n\r"
            "3) Stop stopwatch\n\r"
            "4) Update stopwatch\n\r"
            "5) Reset stopwatch\n\n\r");

    while(1)
    {
        if (g_charReceived)
        {
            g_charReceived = false;  // Clear flag after reading
            char c = g_latestChar;

            switch(c)
            {
            case '1':
                stop_stopwatch();
                UART_Send("\rEnter initial time (hh:mm:ss): ");
                UART_Receive(Buffer, BUFFER_SIZE);
                if (sscanf(Buffer, "%hhu:%hhu:%hhu", &hh, &mm, &ss) == 3)
                    initialize_stopwatch(hh, mm, ss);
                UART_Send("\033[A");
                start_stopwatch();
                break;

            case '2':
                start_stopwatch();
                break;

            case '3':
                stop_stopwatch();
                break;

            case '4':
                stop_stopwatch();
                UART_Send("\rEnter new time (hh:mm:ss): ");
                UART_Receive(Buffer, BUFFER_SIZE);
                if (sscanf(Buffer, "%hhu:%hhu:%hhu", &hh, &mm, &ss) == 3)
                    update_stopwatch(hh, mm, ss);
                UART_Send("\033[A");
                start_stopwatch();
                break;

            case '5':
                reset_stopwatch();
                start_stopwatch();
                break;

            default:
                UART_Send("\rInvalid input\r");
                break;
            }
        }
    }
}
