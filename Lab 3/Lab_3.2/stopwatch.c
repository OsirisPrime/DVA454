#include "stopwatch.h"
#include "uart_functions.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// TivaWare peripheral headers
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

// Global stopwatch instance
stopwatch global_stopwatch = {0, 0, 0, 0, 0, 0};

static void stopwatch_interrupt_handler(void) {
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
    char timeString[9];
    timeString[0] = '0' + global_stopwatch.hh / 10;
    timeString[1] = '0' + global_stopwatch.hh % 10;
    timeString[2] = ':';
    timeString[3] = '0' + global_stopwatch.mm / 10;
    timeString[4] = '0' + global_stopwatch.mm % 10;
    timeString[5] = ':';
    timeString[6] = '0' + global_stopwatch.ss / 10;
    timeString[7] = '0' + global_stopwatch.ss % 10;
    timeString[8] = '\0';

    UART_SendString("\r");          // Print time on a new line
    UART_SendString(timeString);    // Send the time

    // Clear timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void create_stopwatch(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());

    TimerIntRegister(TIMER0_BASE, TIMER_A, stopwatch_interrupt_handler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);

    initialize_stopwatch(0, 0, 0);
}


// Initilize the stopwatch to hh:mm:ss
void initialize_stopwatch(uint8_t hh, uint8_t mm, uint8_t ss) {
    TimerDisable(TIMER0_BASE, TIMER_A);
    global_stopwatch.hh = hh;
    global_stopwatch.mm = mm;
    global_stopwatch.ss = ss;
    global_stopwatch.init_hh = hh;
    global_stopwatch.init_mm = mm;
    global_stopwatch.init_ss = ss;
}


// Start the stopwatch
void start_stopwatch(void) {
    TimerEnable(TIMER0_BASE, TIMER_A);
}


// Stop the stopwatch
void stop_stopwatch(void) {
    TimerDisable(TIMER0_BASE, TIMER_A);
}


// Update the stopwatch to hh:mm:ss
void update_stopwatch(uint8_t hh, uint8_t mm, uint8_t ss) {
    TimerDisable(TIMER0_BASE, TIMER_A);
    if (hh < 24 && mm < 60 && ss < 60) {
        global_stopwatch.hh = hh;
        global_stopwatch.mm = mm;
        global_stopwatch.ss = ss;
    }
}


// Reset stopwatch to initial time
void reset_stopwatch(void) {
    TimerDisable(TIMER0_BASE, TIMER_A);
    global_stopwatch.hh = global_stopwatch.init_hh;
    global_stopwatch.mm = global_stopwatch.init_mm;
    global_stopwatch.ss = global_stopwatch.init_ss;
}
