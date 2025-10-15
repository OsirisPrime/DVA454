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

#define BUFFER_SIZE 8                                                       // Number of bytes in the buffer
char buffer[BUFFER_SIZE];                                                   // The buffer of bytes
int byteCount = 0;                                                          // Number of bytes in the buffer

TaskHandle_t producer_h, consumer_h;                                        // Task handles


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
    static char byte = 'A';                                                 // Start byte is 'A'
    char result = byte++;

    if(byte > 'Z')                                                          // If the byte is 'Z' start over with 'A'
        byte = 'A';

    return result;
}


void putByteIntoBuffer(char x) {
    buffer[byteCount] = x;                                                  // Put the produced byte into the buffer
}


char removeByteFromBuffer(void) {
    char byte = buffer[0];                                                  // Takes the first/oldest byte from the buffer
    int i;

    for(i = 0; i < byteCount - 1; i++) {                                    // Shift all bytes in the buffer to left
        buffer[i] = buffer[i + 1];
    }
    return byte;
}


void consumeByte(char x) {
    UARTprintf("Consumer: consumed %c\n", x);
}


// Producer task
void producer(void *x) {
    char byte;

    for(;;)
    {
        byte = produceByte();                                               // Produce a byte
        UARTprintf("Producer: produced byte %c\n", byte);

        if(byteCount == BUFFER_SIZE){                                       // If the buffer is full, sleep/suspend producer
            UARTprintf("Producer: buffer is full, sleep\n");
            vTaskSuspend(NULL);
        }

        putByteIntoBuffer(byte);                                            // Put the byte into the buffer
        byteCount++;                                                        // Increase the byteCount
        UARTprintf("Producer: put byte in buffer. ByteCount: %d\n", byteCount);

        if(byteCount == 1){                                                 // If the buffer is not empty, wake up/resume consumer
            UARTprintf("Producer: buffer is 1, wake up Consumer\n");
            vTaskResume(consumer_h);
        }
        vTaskDelay(pdMS_TO_TICKS(100));                                     // Simulate delay
    }
}


// Consumer task
void consumer(void* x) {
    char byte;

    for(;;)
    {
        if(byteCount == 0) {                                                // If the buffer is empty, sleep/suspend consumer
            UARTprintf("Consumer: buffer is empty, sleep\n");
            vTaskSuspend(NULL);
        }

        byte = removeByteFromBuffer();                                      // Removed the byte into the buffer
        byteCount--;                                                        // Decrease byteCount
        UARTprintf("Consumer: removed byte in buffer. ByteCount: %d\n", byteCount);

        if(byteCount == BUFFER_SIZE - 1){                                   // If the buffer is not full, wake up/resume producer
            UARTprintf("Consumer: buffer is 7, wake up Producer\n");
            vTaskResume(producer_h);
        }

        consumeByte(byte);
        vTaskDelay(pdMS_TO_TICKS(100));                                     // Simulate delay
    }
}


int main(void)
{
    // Set clock frequency to 120 MHz
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ConfigureUART();                                                        // Initialize UART

    // Create all tasks
    xTaskCreate(producer, "Producer", configMINIMAL_STACK_SIZE, NULL, 1, &producer_h);
    xTaskCreate(consumer, "consumer", configMINIMAL_STACK_SIZE, NULL, 1, &consumer_h);

    vTaskStartScheduler();                                                  // Start the scheduler
    for(;;);
}

