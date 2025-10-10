#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"

#define BUFFER_SIZE 8
char buffer[BUFFER_SIZE];
int byteCount = 0;

TaskHandle_t producer_h, consumer_h;


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
 while(1);
}
#endif


// Configure the UART.
void ConfigureUART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);                            // Enable GPIOA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);                            // Enable UART0

    GPIOPinConfigure(GPIO_PA0_U0RX);                                        // Receive pin
    GPIOPinConfigure(GPIO_PA1_U0TX);                                        // Transmit pin

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);                                   // Configure at 115200 baud
}


char produceByte(void) {
    static char byte = 'a';
    char result = byte;
    byte ++;

    if(byte > 'z')
        byte = 'a';

    return result;
}


void putByteIntoBuffer(char x) {
    buffer[byteCount] = x;
}


char removeByteFromBuffer(void) {
    char byte = buffer[0];
    int i;

    for(i = 0; i < byteCount - 1; i++) {
        buffer[i] = buffer[i + 1];
    }
    return byte;
}


void consumeByte(char x) {
    UARTprintf("Consumer: %s\n");
}


// Producer task
void producer(void *x) {
    char byte;

    for(;;)
    {
        byte = produceByte();
        if(byteCount == BUFFER_SIZE)
            vTaskSuspend(NULL);

        putByteIntoBuffer(byte);
        byteCount = byteCount + 1;

        if(byteCount == 1)
            vTaskResume(consumer_h);
    }
}


// Consumer task
void consumer(void* x) {
    char byte;

    for(;;)
    {
        if(byteCount == 0)
            vTaskSuspend(NULL);

        byte = removeByteFromBuffer();
        byteCount = byteCount - 1;

        if(byteCount == BUFFER_SIZE - 1)
            vTaskResume(producer_h);

        consumeByte(byte);
    }
}


int main(void)
{
    // Set clock frequency to 120 MHz
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ConfigureUART();                        // Initialize UART

    // Create all tasks
    xTaskCreate(producer, "Producer", configMINIMAL_STACK_SIZE, NULL, 1, &producer_h);
    xTaskCreate(consumer, "consumer", configMINIMAL_STACK_SIZE, NULL, 1, &consumer_h);

    vTaskStartScheduler();                  // Start the scheduler
	for(;;);
}
