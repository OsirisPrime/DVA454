//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"


//***********************************************************************
//                       Configurations
//***********************************************************************

// Configure the UART.
void ConfigureUART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);    // Enable GPIOA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);    // Enable UART0

    GPIOPinConfigure(GPIO_PA0_U0RX);                // Receive pin
    GPIOPinConfigure(GPIO_PA1_U0TX);                // Transmit pin

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);           // Configure at 115200 baud
}

//*****************************************************************************
//                      Main
//*****************************************************************************

int main(void)
{
    char buffer[32];
    ConfigureUART();    // initialize UART

    while(1)
    {
        UARTprintf("Send a string: ");          // Write a string to the terminal
        UARTgets(buffer, sizeof(buffer));       // Wait for a string from the terminal
        UARTprintf("You sent: %s\n", buffer);   // Write a string to the terminal
    }
}
