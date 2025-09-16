//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "drivers/pinout.h"
#include "driverlib/adc.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"
#include "CF128x128x16_ST7735S.h"
#include "grlib/grlib.h"

#define NUM_SAMPLES 8               // Number of samples for the moving average filter
#define MIC_CH       ADC_CTL_CH8
#define ACC_X_CH     ADC_CTL_CH3
#define ACC_Y_CH     ADC_CTL_CH2
#define ACC_Z_CH     ADC_CTL_CH1
#define JOY_X_CH     ADC_CTL_CH9
#define JOY_Y_CH     ADC_CTL_CH0


//***********************************************************************
//                       Configurations
//***********************************************************************

// Configure the LCD
void ConfigureLCD(uint32_t systemClock, tContext *sContext) {
    CF128x128x16_ST7735SInit(systemClock);                  // Initialize the LCD
    CF128x128x16_ST7735SClear(ClrBlack);                    // Set the background to black

    GrContextInit(sContext, &g_sCF128x128x16_ST7735S);
    GrContextFontSet(sContext, &g_sFontCm14);              // Set the font size
    GrContextBackgroundSet(sContext, ClrBlack);            // Black background
    GrContextForegroundSet(sContext, ClrWhite);            // White text
    GrFlush(sContext);                                     // Update the display
}

// Configure the ADC
void ConfigureADC(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);             // Enable ADC0
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));      // Wait until ADC0 is ready

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);            // Enable GPIOE
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));     // Wait until GPIOE is ready

    // Configure as analog inputs
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                   | GPIO_PIN_4 | GPIO_PIN_5);

    // Configure to capture 6 channels
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, MIC_CH);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ACC_X_CH);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ACC_Y_CH);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ACC_Z_CH);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 4, JOY_X_CH);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_IE | ADC_CTL_END | JOY_Y_CH);

    ADCSequenceEnable(ADC0_BASE, 0);
    ADCIntClear(ADC0_BASE, 0);
}

//*****************************************************************************
//                      Functions
//*****************************************************************************

// Moving average filter
uint32_t moving_average(uint32_t *buffer) {
    uint32_t sum = 0;
    int i;
    for (i = 0; i < NUM_SAMPLES; i++)
        sum += buffer[i];
    return (uint32_t)(sum / NUM_SAMPLES);
}

//*****************************************************************************
//                      Main
//*****************************************************************************

int main(void)
{
    uint32_t systemClock;
    tContext sContext;

    uint32_t adcValues[6];
    uint32_t microphone[NUM_SAMPLES] = {0};
    uint32_t accelerometer_x[NUM_SAMPLES] = {0};
    uint32_t accelerometer_y[NUM_SAMPLES] = {0};
    uint32_t accelerometer_z[NUM_SAMPLES] = {0};
    uint32_t joystick_x[NUM_SAMPLES] = {0};
    uint32_t joystick_y[NUM_SAMPLES] = {0};

    uint32_t micAvg = 0, accXAvg = 0, accYAvg = 0, accZAvg = 0, joyXAvg = 0, joyYAvg = 0;
    uint32_t bufferIndex = 0;
    char str[32];

    // Set system clock to 16 kHz (maybe try different clock speed)
    systemClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 16000);

    ConfigureLCD(systemClock, &sContext);   // Initialize LCD
    ConfigureADC();                         // Initialize ADC
    PinoutSet(false, false);
    CF128x128x16_ST7735SClear(0x0000);

    while(1)
    {
        ADCProcessorTrigger(ADC0_BASE, 0);                  // Trigger sequence 0 (Read all channels)
        while(!ADCIntStatus(ADC0_BASE, 0, false));          // Wait for conversion to complete

        ADCSequenceDataGet(ADC0_BASE, 0, adcValues);        // Read sensor values (array)
        ADCIntClear(ADC0_BASE, 0);                          // Clear interrupt flag

        // Update circular buffers at current index
        microphone[bufferIndex]         = adcValues[0];
        accelerometer_x[bufferIndex]    = adcValues[1];
        accelerometer_y[bufferIndex]    = adcValues[2];
        accelerometer_z[bufferIndex]    = adcValues[3];
        joystick_x[bufferIndex]         = adcValues[4];
        joystick_y[bufferIndex]         = adcValues[5];

        // Advance circular index
        bufferIndex++;
        if (bufferIndex >= NUM_SAMPLES)
            bufferIndex = 0;

        // Compute the average values from each sensors
        micAvg  = moving_average(microphone);
        accXAvg = moving_average(accelerometer_x);
        accYAvg = moving_average(accelerometer_y);
        accZAvg = moving_average(accelerometer_z);
        joyXAvg = moving_average(joystick_x);
        joyYAvg = moving_average(joystick_y);


        snprintf(str, sizeof(str), "MIC: %u", micAvg);
        GrStringDrawCentered(&sContext, str, -1, 64, 10, 1);

        snprintf(str, sizeof(str), "ACC X:%u", accXAvg);
        GrStringDrawCentered(&sContext, str, -1, 64, 30, 1);

        snprintf(str, sizeof(str), "ACC Y:%u", accYAvg);
        GrStringDrawCentered(&sContext, str, -1, 64, 45, 1);

        snprintf(str, sizeof(str), "ACC Z:%u", accZAvg);
        GrStringDrawCentered(&sContext, str, -1, 64, 60, 1);

        snprintf(str, sizeof(str), "JOY X:%u", joyXAvg);
        GrStringDrawCentered(&sContext, str, -1, 64, 80, 1);

        snprintf(str, sizeof(str), "JOY Y:%u", joyYAvg);
        GrStringDrawCentered(&sContext, str, -1, 64, 95, 1);

        GrFlush(&sContext);
    }
}
