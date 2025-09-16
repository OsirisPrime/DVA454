#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "drivers/pinout.h"


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
 while(1);
}
#endif


int main(void)
{
    volatile uint32_t ui32Loop;

    // Configure the device pins.
    PinoutSet(false, false);

    // Enable the GPIO pin for the LEDs. Set the direction as output,
    // and enable the GPIO pin for digital function.
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);

    // Loop forever.
    while(1){
        // Turn on D1.
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++);
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);

        // Turn on D2.
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++);
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

        // Turn on D3.
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);

        // Turn on D4.
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_PIN_0);
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);

        // Turn on D3.
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0);

        // Turn on D2.
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++);
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
    }
}
