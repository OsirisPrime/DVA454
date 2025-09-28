#include <uart.h>

int checkString (const char *string) {
    char endStr[] = "end";
    int i;
    for (i = 0; endStr[i] != '\0'; i++) {
        if (string[i] != endStr[i])
            return 0;                   // Not equal
    }
    return 1;
}

int main(void)
{
    uint32_t UART0 = 0; // TODO: Temporary give it a value
    char Buffer[32];

    UART_reset();
    UART_init(UART0);

    while(1)
    {
        UART_getString(Buffer);
        UART_putString(Buffer);

        if(checkString(Buffer))
            break;
    }
    return 0;
}
