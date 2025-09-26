#include <uart.h>


int main(void)
{
    uint32_t UART0 = 0; // TODO: Temporary give it a value
    char Buffer[32];

    UART_reset();
    UART_init(UART0);

    while(1)
    {
        UART_getString();
        UART_putString();

        if(Buffer == "end")
            break;
    }
    return 0;
}
