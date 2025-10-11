#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "drivers/pinout.h"
#include "drivers/buttons.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"

#define MAX_VISIBLE 15

volatile int totalCount = 0;
volatile bool showStatus = false;
char uartBuffer[MAX_VISIBLE + 1];
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


// UART task that continuously read and prints last 15 characters
void UARTTask(void *x) {
    char c;
    int len = 0;

    for(;;)
    {
        if(UARTCharsAvail(UART0_BASE))                                      // Check if a char is available in UART0
        {
            c = UARTCharGetNonBlocking(UART0_BASE);                         // Read one char (non-blocking)
            totalCount++;                                                   // Increase total count

            // If the buffer is full, shift all chars one position to the left
            if (len >= MAX_VISIBLE)
            {
                memmove(uartBuffer, uartBuffer + 1, MAX_VISIBLE - 1);       // Shift buffer left by 1
                uartBuffer[MAX_VISIBLE - 1] = c;                            // Store new char at the end
            }
            else
            {
                uartBuffer[len++] = c;                                      // If buffer is not full, append new char
                uartBuffer[len] = '\0';                                     // Add null-terminate
            }

            // Clear line and reprint
            UARTprintf("\r%s", uartBuffer);

            if (showStatus)
            {
                UARTprintf("\n\r%d", totalCount);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void ButtonTask(void *x) {
    unsigned char ucDelta, ucState;

    for(;;)
    {
        ucState = ButtonsPoll(&ucDelta, 0);

        if(BUTTON_PRESSED(LEFT_BUTTON, ucState, ucDelta))                   // Left button press
        {
            xSemaphoreGive(xSemaphore);
            vTaskDelay(pdMS_TO_TICKS(300));                                 // Debounce
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void StatusTask(void *x)
{
    for(;;)
    {
        if(xSemaphoreTake(xSemaphore, portMAX_DELAY))
        {
            showStatus = true;
            uint32_t startTime = xTaskGetTickCount();

            while((xTaskGetTickCount() - startTime) < pdMS_TO_TICKS(10000))
            {
                // Update every 200 ms
                //UARTprintf("\r%s\n%d", uartBuffer, totalCount);
                //vTaskDelay(pdMS_TO_TICKS(200));
            }

            // Clear second line
            UARTprintf("\r%s     ", uartBuffer);
            showStatus = false;
        }
    }
}


int main(void)
{
    // Set clock frequency to 120 MHz
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ConfigureUART();                                                        // Initialize UART
    ButtonsInit();                                                          // Initialize Buttons

    xSemaphore = xSemaphoreCreateBinary();

    // Create all tasks
    xTaskCreate(UARTTask, "UART", 256, NULL, 2, NULL);
    xTaskCreate(ButtonTask, "Button", 128, NULL, 2, NULL);
    xTaskCreate(StatusTask, "Status", 256, NULL, 1, NULL);

    vTaskStartScheduler();                                                  // Start the scheduler

    for(;;);
}
