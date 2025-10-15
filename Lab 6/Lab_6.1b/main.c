#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "semphr.h"
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

SemaphoreHandle_t emptySlots;                                               // Counting semaphore for empty slots
SemaphoreHandle_t filledSlots;                                              // Counting semaphore for filled slots
SemaphoreHandle_t semaphore;                                                // Binary semaphore for buffer access


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
    byteCount--;                                                            // Decrease byteCount
    return byte;
}


void consumeByte(char x) {
    UARTprintf("Consumer: %c\n", x);
}


// Producer task
void producer(void *x) {
    char byte;

    for(;;)
    {
        byte = produceByte();                                               // Produce a byte

        xSemaphoreTake(emptySlots, portMAX_DELAY);                          // Wait for an empty slot

        xSemaphoreTake(semaphore, portMAX_DELAY);                           // Enter critical section
        putByteIntoBuffer(byte);                                            // Put the byte into the buffer
        byteCount++;                                                        // Increase the byteCount
        UARTprintf("Produced: %c\n", byte);
        xSemaphoreGive(semaphore);                                          // Exit critical section

        xSemaphoreGive(filledSlots);                                        // Signal that a slot is filled

        vTaskDelay(pdMS_TO_TICKS(100));                                     // Simulate delay
    }
}


// Consumer task
void consumer(void* x) {
    char byte;

    for(;;)
    {
        xSemaphoreTake(filledSlots, portMAX_DELAY);                         // Wait for a filled slot

        xSemaphoreTake(semaphore, portMAX_DELAY);                           // Enter critical section
        byte = removeByteFromBuffer();                                      // Removed the byte into the buffer
        xSemaphoreGive(semaphore);                                          // Exit critical section

        consumeByte(byte);                                                  // print byte

        xSemaphoreGive(emptySlots);                                         // Signal that a slot is empty

        vTaskDelay(pdMS_TO_TICKS(100));                                     // Simulate processing delay
    }
}


int main(void)
{
    // Set clock frequency to 120 MHz
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ConfigureUART();                                                        // Initialize UART

    // Create all semaphores
    emptySlots = xSemaphoreCreateCounting(BUFFER_SIZE, BUFFER_SIZE);        // Starts fully filled
    filledSlots = xSemaphoreCreateCounting(BUFFER_SIZE, 0);                 // Starts empty
    semaphore = xSemaphoreCreateBinary();                                   // Starts empty

    if (emptySlots == NULL || filledSlots == NULL || semaphore == NULL) {
        UARTprintf("Semaphore creation failed\n");
        while (1);
    }

    xSemaphoreGive(semaphore);                                              // Start with a semaphore (1)

    // Create all tasks
    xTaskCreate(producer, "Producer", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(consumer, "consumer", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();                                                  // Start the scheduler
    for(;;);
}
