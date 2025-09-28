#include <uart.h>

// REG &= ~(1 << n)  --- Clear bit n
// REG |= (1 << n)   --- Set bit n
// REG ^= (1 << n)   --- Toggle bit n

// Initialize UART driver
void UART_init(uint32_t ui32Base) {
    SYSCTL_RCGCUART_R |= (1 << 0);                      // Enable UART0 module
    SYSCTL_RCGCGPIO_R |= (1 << 0);                      // Enable clock to GPIO Port A

    // Allow time for the modules to be ready
    while ((SYSCTL_PRUART_R & (1 << 0)) == 0);
    while ((SYSCTL_PRGPIO_R & (1 << 0)) == 0);

    GPIO_PORTA_AHB_AFSEL_R |= (1 << 0) | (1 << 1);      // Enable alternate function for PA0 and PA1
    GPIO_PORTA_AHB_DEN_R |= (1 << 0) | (1 << 1);        // Enable digital I/O pins
    GPIO_PORTA_AHB_PCTL_R |= (1 << 0) | (1 << 4);       // Configure PA0 to U0Rx, and PA1 to U0Tx

    // Assume 16 MHz system clock and 9600 Baud.
    // BRD = system_clock / (16 * Baud) = 104.166667 -> 104
    // UARTFBRD[DIVFRAC] = integer(0.166667 * 64 + 0.5) = 11.166688 -> 11

    UART0_CTL_R &= ~(1 << 0);                           // Disable UART before configuration
    UART0_IBRD_R = 104;                                 // Write the integer portion of the BRD
    UART0_FBRD_R = 11;                                  // Write the calculated fractional portion of the BRD

    UART0_LCRH_R |= (0x3 << 5);                         // 8-bits length, no parity bit, one stop bit, and operate in normal channel mode (FIFO disabled)
    UART0_CC_R = 0x0;                                   // Use system clock as the UART clock source
    UART0_CTL_R |= (1 << 0) | (1 << 8) | (1 << 9);      // Enable UART, TXE, and RXE
}


// Receive one character
char UART_getChar(void) {
    while (UART0_FR_R & (1 << 4));                      // Check if the receiver is empty, if not get the data from register
    return (char)(UART0_DR_R & 0xFF);                   // Return the data from register (first 8 bits)

}


// Transmit one character
void UART_putChar(char c) {
    while (UART0_FR_R & (1 << 5));                      // Check if the transmitter is full, if not put the data in register
    UART0_DR_R = c;                                     // Put the data in register

}


// Reset the driver to a save state (reset all registers)
void UART_reset(void) {
    SYSCTL_SRUART_R |= (1 << 0);                        // Set UART0 into reset
    SYSCTL_SRGPIO_R |= (1 << 0);                        // Set GPIOA into reset
    SYSCTL_SRUART_R &= ~(1 << 0);                       // Release reset
    SYSCTL_SRGPIO_R &= ~(1 << 0);                       // Release reset

    // Allow time for the modules to be ready
    while ((SYSCTL_PRUART_R & (1 << 0)) == 0);
    while ((SYSCTL_PRGPIO_R & (1 << 0)) == 0);
}


// Uses UART_putChar to write a string
void UART_putString(char *string) {
    while (*string) {                                   // Loop until the end of the string
        UART_putChar(*string++);                        // Send character
    }
}


// Uses UART_getChar to read a string
void UART_getString(char *string) {
    char c;
    uint32_t i = 0;

    while (i < 32 - 1) {                                // Get 31 characters (buffer is 32 and leave space for null terminator)
        c = UART_getChar();                             // Get character
        if (c == '\r' || c == '\n') {                   // End on enter
            break;
        }
        string[i++] = c;
    }
    string[i] = '\0';                                   // Add null terminator at the end of the string
}





