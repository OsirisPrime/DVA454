#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "drivers/buttons.h"
#include "drivers/pinout.h"

// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif

// Main Function
int main(void)
{
    unsigned char ucDelta, ucState;
    bool toggleState = false; // tracks LED toggle state

    // Configure the device pins.
    PinoutSet(false, false);

    // Initialize the button driver.
    ButtonsInit();

    // Enable the GPIO pin for the LED (PN0).
    // Set the direction as output, and enable the GPIO pin for digital function.
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

    // Loop forever.
    while(1)
    {
        // Poll the buttons.(check current state of buttons)
        ucState = ButtonsPoll(&ucDelta, 0);

        // LEFT button: LED ON while pressed, OFF while released
        if (ucState & LEFT_BUTTON)
        {
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0); // on
        }
        else // if it has been released
        {
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0); // off
            if (BUTTON_RELEASED(LEFT_BUTTON, ucState, ucDelta)) // toggle state off when the LEFT button is released
                toggleState = false;

        }


        // RIGHT button: toggle LED state on each press
        if (BUTTON_PRESSED(RIGHT_BUTTON, ucState, ucDelta))
        {
            toggleState = !toggleState; // change toggle state (button)
        }

        if (toggleState == true)
        {
            // Turn on LED
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
        }
        else    // if button has been pressed again (changed state)
        {
            // Turn off LED
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
        }
    }
}
