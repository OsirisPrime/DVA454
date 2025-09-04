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
    uint32_t state_of_LED;

    // Configure the device pins.
    PinoutSet(false, false);

    // Initialize the button driver.
    ButtonsInit();

    // Enable the GPIO pin for the LED (PN0).
    // Set the direction as output, and
    // enable the GPIO pin for digital function.
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    // Loop forever.
    while(1) {
        // Poll the buttons.
        ucState = ButtonsPoll(&ucDelta, 0);

        // Check the state of the LED
        LEDRead(&state_of_LED);

        if(BUTTON_PRESSED(RIGHT_BUTTON, ucState, ucDelta)){
            // Turn on D1.
            LEDWrite(CLP_D1, 1);
        } else {
            // Turn off D1.
            LEDWrite(CLP_D1, 0);
        }

        if(BUTTON_PRESSED(LEFT_BUTTON, ucState, ucDelta)){
            // Switch the state of D1.
            LEDWrite(CLP_D1, !state_of_LED);
        }
    }
}
