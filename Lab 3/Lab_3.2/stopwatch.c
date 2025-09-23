#include "stopwatch.h"
#include "uart_functions.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"


// Global stopwatch instance
stopwatch global_stopwatch = {0, 0, 0, 0, 0, 0, true};


static void stopwatch_interrupt_handler(void) {
    char time[16];

    if (global_stopwatch.first_update == true) {
        snprintf(time, sizeof(time), "\r%02u:%02u:%02u", global_stopwatch.hh, global_stopwatch.mm, global_stopwatch.ss);
        UART_Send(time);                                      // Send the time
        UART_Send("                                   ");     // Print over left over
        global_stopwatch.first_update = false;
    } else {
        // Increment seconds
        global_stopwatch.ss++;

        if(global_stopwatch.ss >= 60) {
            global_stopwatch.ss = 0;
            global_stopwatch.mm++;
        }

        if(global_stopwatch.mm >= 60) {
            global_stopwatch.mm = 0;
            global_stopwatch.hh++;
        }

        if(global_stopwatch.hh >= 24) {
            global_stopwatch.hh = 0;
            global_stopwatch.mm = 0;
            global_stopwatch.ss = 0;
        }

        // Format and print time
        snprintf(time, sizeof(time), "\r%02u:%02u:%02u", global_stopwatch.hh, global_stopwatch.mm, global_stopwatch.ss);
        UART_Send(time);                                      // Send the time
        UART_Send("                                   ");     // Print over left over
    }

    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);                 // Clear timer interrupt
}



// Create a stopwatch
void create_stopwatch(void) {
    uint32_t systemClock;
    systemClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);                           // Enable Timer0
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));                    // Wait for Timer0 to be ready

    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);                      // Make a Timer0A run in periodic mode (count down from a value to 0, and repeat)
    TimerLoadSet(TIMER0_BASE, TIMER_A, systemClock);                        // Load systemClock as the starting value (120000000). (Makes it ticks every seconds)

    TimerIntRegister(TIMER0_BASE, TIMER_A, stopwatch_interrupt_handler);    // Register the function as the ISR of Timer0A. (Calls the function every second)
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);                        // Enable timeout interrupt for Timer0A
    IntEnable(INT_TIMER0A);                                                 // Enable interrupts for Timer0A

    initialize_stopwatch(0, 0, 0);                                          // Initialize the stopwatch time
}


// initialize the stopwatch to hh:mm:ss
void initialize_stopwatch(uint8_t hh, uint8_t mm, uint8_t ss) {
    if (hh < 24 && mm < 60 && ss < 60) {
        global_stopwatch.hh = hh;
        global_stopwatch.mm = mm;
        global_stopwatch.ss = ss;
        global_stopwatch.init_hh = hh;
        global_stopwatch.init_mm = mm;
        global_stopwatch.init_ss = ss;
        global_stopwatch.first_update = true;
    }
}


// Start the stopwatch
void start_stopwatch(void) {
    TimerEnable(TIMER0_BASE, TIMER_A);
}


// Stop the stopwatch
void stop_stopwatch(void) {
    TimerDisable(TIMER0_BASE, TIMER_A);
}


// Update current time to hh:mm:ss
void update_stopwatch(uint8_t hh, uint8_t mm, uint8_t ss) {
    if (hh < 24 && mm < 60 && ss < 60) {
        global_stopwatch.hh = hh;
        global_stopwatch.mm = mm;
        global_stopwatch.ss = ss;
        global_stopwatch.first_update = true;
    }
}


// Reset stopwatch to initial time
void reset_stopwatch(void) {
    global_stopwatch.hh = global_stopwatch.init_hh;
    global_stopwatch.mm = global_stopwatch.init_mm;
    global_stopwatch.ss = global_stopwatch.init_ss;
    global_stopwatch.first_update = true;
}
