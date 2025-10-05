#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <stdio.h>
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


// Semaphore (shared resource)
SemaphoreHandle_t xSemaphore;


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


// Simulate workload in ms
void simulate_workload(uint32_t ms) {
    TickType_t startTime = xTaskGetTickCount();
    while((xTaskGetTickCount() - startTime) < pdMS_TO_TICKS(ms));
}


// Low priority with shared resource
void Task1(void *x) {
    volatile uint32_t i;
    TickType_t xLastWakeTime = xTaskGetTickCount();                         // Get task start time

    for(;;) {
        UARTprintf("\nTask 1 started");

        xSemaphoreTake(xSemaphore, portMAX_DELAY);                          // Try to take semaphore
        UARTprintf("\nTask 1 sem take");

        UARTprintf("\nTask 1 started its workload");
        simulate_workload(1000);                                            // Simulate workload for 1s

        UARTprintf("\nTask 1 sem give");
        xSemaphoreGive(xSemaphore);                                         // Give semaphore
        UARTprintf("\nTask 1 finished");
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));               // Do task again with a period of 5s
    }
}


// Medium priority
void Task2(void *x) {
    volatile uint32_t i;
    vTaskDelay(pdMS_TO_TICKS(1000));
    TickType_t xLastWakeTime = xTaskGetTickCount();                         // Get task start time

    for(;;) {
        UARTprintf("\nTask 2 started");

        UARTprintf("\nTask 2 started its workload");
        simulate_workload(1500);                                            // Simulate workload for 1.5s

        UARTprintf("\nTask 2 finished");
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5000));               // Do task again with a period of 5s
    }
}


// High priority with shared resource
void Task3(void *x) {
    volatile uint32_t i;
    vTaskDelay(pdMS_TO_TICKS(500));
    TickType_t xLastWakeTime = xTaskGetTickCount();                         // Get task start time

    for(;;) {
        UARTprintf("\nTask 3 started");

        xSemaphoreTake(xSemaphore, portMAX_DELAY);                          // Try to take semaphore
        UARTprintf("\nTask 3 sem take");

        UARTprintf("\nTask 3 started its workload");
        simulate_workload(500);                                             // Simulate workload for 0.5s

        UARTprintf("\nTask 3 sem give");
        xSemaphoreGive(xSemaphore);                                         // Give semaphore
        UARTprintf("\nTask 3 finished");
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(7000));               // do task again with a period of 7s
    }
}


int main(void)
{
    ConfigureUART();                                                            // Initialize UART
    xSemaphore = xSemaphoreCreateBinary();                                      // Create a binary semaphore
    xSemaphoreGive(xSemaphore);                                                 // Give the semaphore 1

    // Create all tasks
    xTaskCreate(Task1, "Low", configMINIMAL_STACK_SIZE, NULL, 1, NULL);         // Low priority task
    xTaskCreate(Task2, "Medium", configMINIMAL_STACK_SIZE, NULL, 2, NULL);      // Medium priority task
    xTaskCreate(Task3, "High", configMINIMAL_STACK_SIZE, NULL, 3, NULL);        // High priority task

    vTaskStartScheduler();                                                      // Start the scheduler
    for(;;);
}
