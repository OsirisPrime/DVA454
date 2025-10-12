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
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"

// ADC channels
#define MIC_ADC_CH    ADC_CTL_CH8
#define JOY_X_ADC_CH  ADC_CTL_CH9
#define JOY_Y_ADC_CH  ADC_CTL_CH0
#define ACC_X_ADC_CH  ADC_CTL_CH3
#define ACC_Y_ADC_CH  ADC_CTL_CH2
#define ACC_Z_ADC_CH  ADC_CTL_CH1

// Task periods
#define PERIOD_MIC_MS   5
#define PERIOD_JOY_MS   10
#define PERIOD_ACC_MS   20
#define GATEKEEPER_PERIOD_MS  40

// Queue handles
static QueueHandle_t micQueue = NULL;
static QueueHandle_t joyQueue = NULL;
static QueueHandle_t accQueue = NULL;


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


void ConfigureADC(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);                             // Enable ADC0
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);                            // Enable GPIOE
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);                            // Enable GPIOD
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
}


void MicrophoneTask(void *x) {

}


void JoystickTask(void *x) {

}


void AccelerometerTask(void *x) {

}


void GatekeeperTask(void *x) {

}


int main(void)
{
    // Set clock frequency to 120 MHz
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ConfigureUART();                                                        // Initialize UART
    ConfigureADC();

    // Create queues
    micQueue = xQueueCreate(8, sizeof(uint32_t));
    joyQueue = xQueueCreate(4, sizeof(uint32_t[2]));
    accQueue = xQueueCreate(2, sizeof(uint32_t[3]));

    // Check if it failed to create queues
    if (micQueue == NULL || joyQueue == NULL || accQueue == NULL) {
        UARTprintf("Failed to create queues");
        for(;;);
    }

    // Create tasks
    xTaskCreate(MicrophoneTask, "Mic", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(JoystickTask, "Joy", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(AccelerometerTask, "Acc", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(GatekeeperTask, "Gatekepper", configMINIMAL_STACK_SIZE + 256, NULL, 2, NULL);

    // Start scheduler
    vTaskStartScheduler();
    for(;;);

}

