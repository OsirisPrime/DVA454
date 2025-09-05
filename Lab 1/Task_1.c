#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "drivers/buttons.h"
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
    unsigned char ucDelta, ucState;
    volatile uint32_t Loop;         // Delay time

    // Configure the device pins.
    PinoutSet(false, false);

    // Initialize the button driver.
    ButtonsInit();

    // Enable the GPIO pin for the LED (PN1).
    // Set the direction as output, and
    // enable the GPIO pin for digital function.
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

    while(1) {
        // Poll the buttons.
        ucState = ButtonsPoll(&ucDelta, 0);

        if(BUTTON_PRESSED(RIGHT_BUTTON, ucState, ucDelta)){
            while(1){
                // Turn on LED.
                GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);

                // Delay for a bit
                for(Loop = 0; Loop < 100000; Loop++){
                }

                // Turn off LED.
                GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);

                // Delay for a bit
                for(Loop = 0; Loop < 100000; Loop++){
                }
            }
        }
    }
}
