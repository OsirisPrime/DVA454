#include <uart.h>

// REG &= ~(1 << n)  --- Clear bit n
// REG |= (1 << n)   --- Set bit n
// REG ^= (1 << n)   --- Toggle bit n

// Initialize UART driver
void UART_init(uint32_t ui32Base) {
    // 9600 Baud. Packet length of 8 bits. no parity bit. one stop bit. operate in normal channel mode.

    SYSCTL_RCGCUART_R |= (1 << 0);          // Enable the UART module using the RCGCUART register
    SYSCTL_RCGCGPIO_R |= (1 << 0);          // Enable the clock to the appropriate GPIO module via the RCGCGPIO register

    GPIO_PORTA_AHB_AFSEL_R = 1 | 2;         // Set the GPIO AFSEL bits for the appropriate pins
    GPIO_PORTA_AHB_PCTL_R = 1 | (1 << 4);   // Configure the PMCn fields in the GPIOPCTL register to assign the UART signals to the appropriate pins

    // Assume 16 MHz system clock and 9600 Baud.
    // BRD = system_clock / (16 * Baud) = 104.166667 -> 104
    // UARTFBRD[DIVFRAC] = integer(0.166667 * 64 + 0.5) = 11.166688 -> 11

    UART0_CTL_R &= ~(1 << 0);               // Disable the UART by clearing the UARTEN bit in the UARTCTL register.
    UART0_IBRD_R = 104;                     // Write the integer portion of the BRD to the UARTIBRD register.
    UART0_FBRD_R = 11;                      // Write the fractional portion of the BRD to the UARTFBRD register.

    UART0_LCRH_R |= (0x3 << 5);             // Write the desired serial parameters to the UARTLCRH register. (Packet length of 8 bits, no parity bit, one stop bit, and operate in normal channel mode (not FIFO).)
    UART0_CC_R = 0x0;                       // Configure the UART clock source by writing to the UARTCC register. (System clock)
    UART0_CTL_R |= (1 << 0);                // Enable the UART by setting the UARTEN bit in the UARTCTL register.
}


// Receive one character
char UART_getChar(void) {
    // Don't try to access the hardware if UART is not Initialized


}


// Transmit one character
void UART_putChar(char c) {
    // Don't try to access the hardware if UART is not Initialized


}


// Reset the driver to a save state (reset all registers)
void UART_reset(void) {
    SYSCTL_SRUART_R &= ~(1 << 0);           // Software sets a bit (or bits) in the SRUART register. While the SRUART bit is 1, the peripheral is held in reset.
    SYSCTL_SRGPIO_R &= ~(1 << 0);           // Software sets a bit (or bits) in the SRGPIO register. While the SRGPIO bit is 1, the peripheral is held in reset.
}


// Uses UART_putChar to write a string
void UART_putString() {



}


// Uses UART_getChar to read a string
void UART_getString() {



}




