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
    volatile uint32_t times_to_blink;   // The number of times the LED will blink
    volatile uint32_t ui32Loop;         // Delay time

    // Configure the device pins.
    PinoutSet(false, false);

    // Initialize the button driver.
    ButtonsInit();

    // Enable the GPIO pin for the LED (PN0).
    // Set the direction as output, and
    // enable the GPIO pin for digital function.
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    while(1) {
        // Poll the buttons.
        ucState = ButtonsPoll(&ucDelta, 0);

        if(BUTTON_PRESSED(RIGHT_BUTTON, ucState, ucDelta)){
            // Blink 5 times
            for(times_to_blink = 0; times_to_blink < 5; times_to_blink++){
                // Turn on D1.
                LEDWrite(CLP_D1, 1);

                // Delay for a bit
                for(ui32Loop = 0; ui32Loop < 100000; ui32Loop++){
                }

                // Turn off D1.
                LEDWrite(CLP_D1, 0);

                // Delay for a bit
                for(ui32Loop = 0; ui32Loop < 100000; ui32Loop++){
                }
            }
        }
    }
}
