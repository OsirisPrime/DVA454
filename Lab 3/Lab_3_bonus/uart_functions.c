#include "uart_functions.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"


// UART ISR
void UARTIntHandler(void)
{
    uint32_t status = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE, status);

    while (UARTCharsAvail(UART0_BASE))
    {
        g_latestChar = UARTCharGetNonBlocking(UART0_BASE);
        g_charReceived = true;  // Notify main loop that a new char arrived
    }
}


// Configure the UART
void ConfigureUART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);

    UARTEnable(UART0_BASE);
    // Register ISR and enable UART RX interrupts
    UARTIntRegister(UART0_BASE, UARTIntHandler);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(INT_UART0);
}


// Send UART
void UART_Send(const char *str) {
    UARTprintf("%s", str);
}


// Receive UART
void UART_Receive(char *buffer, uint32_t size) {
    UARTgets(buffer, size);
}
