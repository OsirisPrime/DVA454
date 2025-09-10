//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "drivers/pinout.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"


//***********************************************************************
//                       Configurations
//***********************************************************************

// Configure the UART.
void ConfigureUART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);    // Enable GPIOA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);    // Enable UART0

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);           // Configure at 115200 baud
}

// Configure the ADC for the joystick
void ConfigureJoystick(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);             // Enable ADC0
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}    // Wait until ADC0 is ready

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);            // Enable GPIOE
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);

    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
    ADCSequenceEnable(ADC0_BASE, 0);
}

// Configure the PWM
void ConfigurePWM(uint32_t pwm_word) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);        // Enable port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);         // Enable PWM0

    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);                 // Set PWM clock
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinConfigure(GPIO_PF2_M0PWM2);

    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_DBG_RUN);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, pwm_word);    // Set PWM period/frequency
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);                 // Enable generator
    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);     // Enable PWM output 2
}

//*****************************************************************************
//                      Main
//*****************************************************************************

int main(void)
{
    float pwm_word;
    uint32_t systemClock;
    uint32_t joystick;

    // Set system clock to 16 kHz
    systemClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 16000);
    pwm_word = systemClock / 200;   // Set PWM period

    ConfigureUART();                // initialize UART
    ConfigureJoystick();            // initialize ADC
    ConfigurePWM(pwm_word);         // initialize PWM

    PinoutSet(false, false);

    while(1)
    {
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);
        ADCProcessorTrigger(ADC0_BASE, 0);
        while(!ADCIntStatus(ADC0_BASE, 0, false)) {}

        // Read joystick value (0-4095)
        ADCSequenceDataGet(ADC0_BASE, 0, &joystick);

        // Scale joystick value to 0-100
        joystick = (joystick * 100) / 4095;
        UARTprintf("Joystick value: %d\n", joystick);

        // Case 1: Joystick at minimum -> LED off
        if (joystick <= 0) {
            GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

        // Case 2: Joystick at maximum -> LED fully on
        } else if (joystick >= 100) {
            GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);

        // Case 3: Joystick in between -> LED PWM controlled
        } else {
            GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, (pwm_word * joystick) / 100);
        }
    }
}

