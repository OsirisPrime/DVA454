#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"

/* ADC channels */
#define MIC_ADC_CH ADC_CTL_CH8
#define JOY_X_ADC_CH ADC_CTL_CH9
#define JOY_Y_ADC_CH ADC_CTL_CH0
#define ACC_X_ADC_CH ADC_CTL_CH3
#define ACC_Y_ADC_CH ADC_CTL_CH2
#define ACC_Z_ADC_CH ADC_CTL_CH1

/* Task periods */
#define PERIOD_MIC_MS 5
#define PERIOD_JOY_MS 10
#define PERIOD_ACC_MS 20
#define GATEKEEPER_PERIOD_MS 40

/* Averages required by lab */
#define MIC_REQUIRED_SAMPLES 8
#define JOY_REQUIRED_SAMPLES 4
#define ACC_REQUIRED_SAMPLES 2

/* Global queue handles */
static QueueHandle_t micQueue = NULL;
static QueueHandle_t joyQueue = NULL;
static QueueHandle_t accQueue = NULL;

/* Message structs for queues */
typedef struct {
    uint32_t x;
    uint32_t y;
} JoyMsg_t;

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} AccMsg_t;



#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
 while(1);
}
#endif


/* Configure MIC (SS2, single step CH8) */
void MIC_enable(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)) {}

    /* PE5 as mic ADC input */
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);

    /* Use sequencer 2 (SS2) with one step */
    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH8);
    ADCSequenceEnable(ADC0_BASE, 2);
    ADCIntClear(ADC0_BASE, 2);
}


/* Read one MIC sample (raw 12-bit: 0-4095) */
void MIC_read(uint32_t *micValue)
{
    uint32_t sample;
    ADCProcessorTrigger(ADC0_BASE, 2);
    while(!ADCIntStatus(ADC0_BASE, 2, false)) {}
    ADCSequenceDataGet(ADC0_BASE, 2, &sample);

    *micValue = sample; /* raw value; gatekeeper will scale/avg */
    ADCIntClear(ADC0_BASE, 2);
}


/* Configure JOYSTICK (SS1, two steps: CH9 and CH0) */
void JOYSTICK_enable(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)) {}

    /* PE4 (CH9) and PE3 (CH0) - matches your reference mapping */
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3 | GPIO_PIN_4);

    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    /* Step 0: X (CH9) ; Step 1: Y (CH0) with IE+END */
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH9);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
    ADCSequenceEnable(ADC0_BASE, 1);
    ADCIntClear(ADC0_BASE, 1);
}


/* Read joystick (two values raw 0..4095, we leave conversion to gatekeeper) */
void JOYSTICK_read(uint32_t *joystickXvalue, uint32_t *joystickYvalue)
{
    uint32_t data[2];
    ADCProcessorTrigger(ADC0_BASE, 1);
    while(!ADCIntStatus(ADC0_BASE, 1, false)) {}
    ADCSequenceDataGet(ADC0_BASE, 1, data);

    *joystickXvalue = data[0];
    *joystickYvalue = data[1];

    ADCIntClear(ADC0_BASE, 1);
}


/* Configure ACC (SS0, three steps: CH3, CH2, CH1) */
void ACC_enable(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)) {}

    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);

    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH3); /* x */
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH2); /* y */
    ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH1); /* z */
    ADCSequenceEnable(ADC0_BASE, 0);
    ADCIntClear(ADC0_BASE, 0);
}


/* Read accelerometer (3 values raw 0..4095); convert to 0..100 in gatekeeper */
void ACC_read(uint32_t *x, uint32_t *y, uint32_t *z)
{
    uint32_t data[3];
    ADCProcessorTrigger(ADC0_BASE, 0);
    while(!ADCIntStatus(ADC0_BASE, 0, false)) {}
    ADCSequenceDataGet(ADC0_BASE, 0, data);
    *x = data[0];
    *y = data[1];
    *z = data[2];
    ADCIntClear(ADC0_BASE, 0);
}



/* Microphone producer: samples every PERIOD_MIC_MS and sends raw value to micQueue */
void MicrophoneTask(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(PERIOD_MIC_MS);
    uint32_t sample;
    for(;;)
    {
        MIC_read(&sample);
        /* Non-blocking send; drop sample on full queue */
        if (xQueueSendToBack(micQueue, &sample, 0) != pdPASS) {
            /* optional: track dropped samples; for now ignore */
        }
        vTaskDelayUntil(&lastWake, period);
    }
}


/* Joystick producer: samples every PERIOD_JOY_MS and sends {x,y} */
void JoystickTask(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(PERIOD_JOY_MS);
    uint32_t x, y;
    JoyMsg_t msg;
    for(;;)
    {
        JOYSTICK_read(&x, &y);
        msg.x = x;
        msg.y = y;
        if (xQueueSendToBack(joyQueue, &msg, 0) != pdPASS) {
            /* queue full -> drop */
        }
        vTaskDelayUntil(&lastWake, period);
    }
}


/* Accelerometer producer: samples every PERIOD_ACC_MS and sends {x,y,z} */
void AccelerometerTask(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(PERIOD_ACC_MS);
    uint32_t ax, ay, az;
    AccMsg_t msg;
    for(;;)
    {
        ACC_read(&ax, &ay, &az);
        msg.x = ax;
        msg.y = ay;
        msg.z = az;
        if (xQueueSendToBack(accQueue, &msg, 0) != pdPASS) {
            /* queue full -> drop */
        }
        vTaskDelayUntil(&lastWake, period);
    }
}


/* Helper: scale raw ADC (0-4095) to 0-100 */
static inline uint32_t scaleTo100(uint32_t raw) {
    return (raw * 100UL) / 4095UL;
}


/* Gatekeeper: periodically collect required samples from queues, compute averages,
   and print them (synchronized output). */
void GatekeeperTask(void *pvParameters)
{
    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(GATEKEEPER_PERIOD_MS);

    uint32_t micSum;
    JoyMsg_t joyMsg;
    AccMsg_t accMsg;

    /* Temp buffers to accumulate samples */
    uint32_t micSamples[MIC_REQUIRED_SAMPLES];
    uint32_t joyXSamples[JOY_REQUIRED_SAMPLES];
    uint32_t joyYSamples[JOY_REQUIRED_SAMPLES];
    uint32_t accXSamples[ACC_REQUIRED_SAMPLES];
    uint32_t accYSamples[ACC_REQUIRED_SAMPLES];
    uint32_t accZSamples[ACC_REQUIRED_SAMPLES];

    for(;;)
    {
        /* Collect MIC_REQUIRED_SAMPLES from micQueue */
        uint32_t got = 0;
        while (got < MIC_REQUIRED_SAMPLES)
        {
            /* Wait a short while for producers to fill queue (avoid blocking forever) */
            if (xQueueReceive(micQueue, &micSamples[got], pdMS_TO_TICKS(5)) == pdPASS) {
                got++;
            } else {
                /* timeout -> continue trying until required count */
            }
        }

        /* Collect joystick samples */
        got = 0;
        while (got < JOY_REQUIRED_SAMPLES)
        {
            if (xQueueReceive(joyQueue, &joyMsg, pdMS_TO_TICKS(5)) == pdPASS) {
                joyXSamples[got] = joyMsg.x;
                joyYSamples[got] = joyMsg.y;
                got++;
            } else {
                /* timeout -> continue trying */
            }
        }

        /* Collect accelerometer samples */
        got = 0;
        while (got < ACC_REQUIRED_SAMPLES)
        {
            if (xQueueReceive(accQueue, &accMsg, pdMS_TO_TICKS(5)) == pdPASS) {
                accXSamples[got] = accMsg.x;
                accYSamples[got] = accMsg.y;
                accZSamples[got] = accMsg.z;
                got++;
            } else {
                /* timeout -> continue trying */
            }
        }

        /* Compute averages (scale raw ADC values to 0-100 where applicable) */
        micSum = 0;
        uint32_t i;
        for (i = 0; i < MIC_REQUIRED_SAMPLES; ++i) {
            micSum += scaleTo100(micSamples[i]);
        }
        uint32_t micAvg = micSum / MIC_REQUIRED_SAMPLES;

        uint32_t joyXsum = 0, joyYsum = 0;
        for (i = 0; i < JOY_REQUIRED_SAMPLES; ++i) {
            joyXsum += scaleTo100(joyXSamples[i]);
            joyYsum += scaleTo100(joyYSamples[i]);
        }
        uint32_t joyXavg = joyXsum / JOY_REQUIRED_SAMPLES;
        uint32_t joyYavg = joyYsum / JOY_REQUIRED_SAMPLES;

        uint32_t accXsum = 0, accYsum = 0, accZsum = 0;
        for (i = 0; i < ACC_REQUIRED_SAMPLES; ++i) {
            accXsum += scaleTo100(accXSamples[i]);
            accYsum += scaleTo100(accYSamples[i]);
            accZsum += scaleTo100(accZSamples[i]);
        }
        uint32_t accXavg = accXsum / ACC_REQUIRED_SAMPLES;
        uint32_t accYavg = accYsum / ACC_REQUIRED_SAMPLES;
        uint32_t accZavg = accZsum / ACC_REQUIRED_SAMPLES;

        /* Synchronized print: gatekeeper is only printer.
           Clear terminal and print three lines (Microphone, Joystick, Accelerometer) */
        UARTprintf("\033[2J\033[H"); /* ANSI clear screen + home cursor */
        UARTprintf("Microphone: %udB\n", (unsigned int)micAvg);
        UARTprintf("Joystick: %u, %u\n", (unsigned int)joyXavg, (unsigned int)joyYavg);
        UARTprintf("Accelerometer: %u, %u, %u\n", (unsigned int)accXavg, (unsigned int)accYavg, (unsigned int)accZavg);

        /* Wait until next period */
        vTaskDelayUntil(&lastWake, period);
    }
}


/* Configure UART */
void ConfigureUART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}


/* Configure all ADCs (call each sensor enable) */
void ConfigureADC_PerSensor(void)
{
    /* Each enable function enables ADC0 and GPIOE as needed; they are idempotent */
    MIC_enable();
    JOYSTICK_enable();
    ACC_enable();
}


int main(void)
{
    /* Set clock frequency to 120 MHz */
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ConfigureUART();
    ConfigureADC_PerSensor();

    /* Create queues
       - micQueue: depth 8 (we need 8), items are raw uint32_t
       - joyQueue: depth 8 (we choose 8 to allow producers to enqueue), item JoyMsg_t
       - accQueue: depth 4, item AccMsg_t
    */
    micQueue = xQueueCreate(8, sizeof(uint32_t));
    joyQueue = xQueueCreate(8, sizeof(JoyMsg_t));
    accQueue = xQueueCreate(4, sizeof(AccMsg_t));

    if (micQueue == NULL || joyQueue == NULL || accQueue == NULL) {
        UARTprintf("Failed to create queues\r\n");
        for(;;);
    }

    /* Create all tasks */
    xTaskCreate(MicrophoneTask, "Mic", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(JoystickTask, "Joy", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(AccelerometerTask, "Acc", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(GatekeeperTask, "Gatekeeper", configMINIMAL_STACK_SIZE + 256, NULL, 2, NULL);

    /* Start scheduler */
    vTaskStartScheduler();

    for(;;);
}
