//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c129encpdt.h"


//***********************************************************************
//                       Configurations
//***********************************************************************

// Configure the UART.
void ConfigureUART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}

// Configure the PWM
void ConfigurePWM(uint32_t pwm_word) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // Enable F port
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);     // Enable PWM0

    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinConfigure(GPIO_PF2_M0PWM2);

    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_DBG_RUN);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, pwm_word);
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);
}

//*****************************************************************************
//                      Main
//*****************************************************************************

int main(void)
{
    char buffer[16];
    int brightness = 0;
    float pwm_word;
    uint32_t systemClock;

    systemClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 16000);
    pwm_word = systemClock / 200;

    ConfigureUART();
    ConfigurePWM(pwm_word);


    while(1)
    {
        UARTprintf("Enter LED brightness (0-100): ");
        UARTgets(buffer, sizeof(buffer));
        brightness = atoi(buffer);

        // Case 1: 0% -> LED off
        if (brightness <= 0) {
            GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
            UARTprintf("LED off\n\n");

        // Case 2: 100% -> LED fully on
        } else if (brightness >= 100) {
            GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
            UARTprintf("LED fully on\n\n");

        // Case 3: 1-99% -> PMW LED control
        } else {
            GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, (pwm_word*brightness)/100);
            UARTprintf("LED brightness set to %d\n\n", brightness);
        }
    }
}
