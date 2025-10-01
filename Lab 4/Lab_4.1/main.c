#include <uart.h>

// Helper function to check string
int checkString (const char *string) {
    char endStr[] = "end";
    int i = 0;

    while (endStr[i] != '\0') {         // Go thought the whole endStr
        if (string[i] != endStr[i])     // Check if all characters are the same
            return 0;                   // If not the same, return 0
        i++;
    }

    if (string[i] == '\0')
        return 1;

    return 0;
}

int main(void)
{
    uint32_t UART0_BASE = 0;
    char Buffer[32];

    UART_reset();                       // Reset UART
    UART_init(UART0_BASE);              // Initiate UART

    while(1)
    {
        UART_getString(Buffer);         // Receive string from user
        UART_putString(Buffer);         // Print the string from user

        if(checkString(Buffer)) {       // Check if it is the end string
            UART_putString(Buffer);
            break;
        }
    }

    UART_reset();                       // Should not get or put if not initiated
    UART_getString(Buffer);
    UART_putString(Buffer);

    return 0;
}
