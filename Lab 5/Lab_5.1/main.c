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
#include "drivers/pinout.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"


TaskHandle_t LED_D1;
TaskHandle_t LED_D2;
TaskHandle_t LED_D3;
TaskHandle_t LED_D4;

// Flag for LED 1 and 2
volatile int forced_LED_1 = 0;
volatile int forced_LED_2 = 0;


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
 while(1);
}
#endif


// Configure the LED
void ConfigureLED(void) {
    PinoutSet(false, false);                                                // Configure the device pins.

    // Enable the GPIO pin for the LEDs. Set the direction as output, and enable the GPIO pin for digital function.
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);        // PN0 (D2) and PN1 (D1)
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);        // PF0 (D4) and PF4 (D3)
}


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


// LED control
void LEDcontrol(void *x) {
    int led = (int) x;
    TickType_t interval;
    uint32_t port;
    uint8_t pin;

    switch(led) {
        case 1: port = GPIO_PORTN_BASE; pin = GPIO_PIN_1; interval = pdMS_TO_TICKS(1000); break;
        case 2: port = GPIO_PORTN_BASE; pin = GPIO_PIN_0; interval = pdMS_TO_TICKS(2000); break;
        case 3: port = GPIO_PORTF_BASE; pin = GPIO_PIN_4; interval = pdMS_TO_TICKS(3000); break;
        case 4: port = GPIO_PORTF_BASE; pin = GPIO_PIN_0; interval = pdMS_TO_TICKS(4000); break;
        default: vTaskDelete(NULL);
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();

    if(led == 2)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    else if(led == 3)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
    else if(led == 4)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));

    for(;;) {
        // Check force flags
        if((led == 1 && forced_LED_1 == 1) || (led == 2 && forced_LED_2 == 1)) {
            // Do nothing, keep LED state handled by ForceLEDTask
        }
        else {
            GPIOPinWrite(port, pin, 0);                                     // Turn off LED
            UARTprintf("LED_D%d off\n", led);
            vTaskDelay(interval);                                           // Wait interval
            GPIOPinWrite(port, pin, pin);                                   // Turn on LED
            UARTprintf("LED_D%d on\n", led);
            vTaskDelay(interval);
        }
        vTaskDelayUntil(&xLastWakeTime, interval*2);                        // Wait until next period
    }

}


void ForceLEDTask(void *x) {
    int led = (int) x;

    if (led == 1) {
        forced_LED_1 = 1;
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
    } else if (led == 2) {
        forced_LED_2 = 1;
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }

    vTaskDelay(pdMS_TO_TICKS(10000));                           // 10 seconds

    if (led == 1) {
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
        forced_LED_1 = 0;
    } else if (led == 2) {
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
        forced_LED_2 = 0;
    }

    vTaskDelete(NULL);
}


void ButtonTask(void *x) {
    unsigned char ucDelta, ucState;

    for (;;) {
        ucState = ButtonsPoll(&ucDelta, 0);

        if (BUTTON_PRESSED(LEFT_BUTTON, ucState, ucDelta)) {
            UARTprintf("\nLeft button pressed -> LED_D1 for 10s\n");
            xTaskCreate(ForceLEDTask, "LED1Force", configMINIMAL_STACK_SIZE, (void *)1, 2, NULL);
        }

        if (BUTTON_PRESSED(RIGHT_BUTTON, ucState, ucDelta)) {
            UARTprintf("\nRight button pressed -> LED_D2 for 10s\n");
            xTaskCreate(ForceLEDTask, "LED2Force", configMINIMAL_STACK_SIZE, (void *)2, 2, NULL);
        }

        vTaskDelay(pdMS_TO_TICKS(50));      // debounce / polling interval
    }
}




int main(void)
{
    // Set clock frequency to 120 MHz
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ConfigureLED();                         // Initialize LEDs
    ConfigureUART();                        // Initialize UART
    ButtonsInit();							// Initialize Buttons

    // Create all tasks
    xTaskCreate(LEDcontrol, "LED_D1", configMINIMAL_STACK_SIZE, (void*)1, 1, &LED_D1);
    xTaskCreate(LEDcontrol, "LED_D2", configMINIMAL_STACK_SIZE, (void*)2, 1, &LED_D2);
    xTaskCreate(LEDcontrol, "LED_D3", configMINIMAL_STACK_SIZE, (void*)3, 1, &LED_D3);
    xTaskCreate(LEDcontrol, "LED_D4", configMINIMAL_STACK_SIZE, (void*)4, 1, &LED_D4);
    xTaskCreate(ButtonTask,"ButtonTask", configMINIMAL_STACK_SIZE, (void*)0, 2, NULL);

    vTaskStartScheduler();                  // Start the scheduler
	for(;;);
}

