#include <xc.h>
#include "setting_hardaware/setting.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
// CONFIG bits assumed set elsewhere (same as your lab)
#define _XTAL_FREQ 4000000 // Change to your actual clock frequency


void main(void)
{
    UART_Initialize();
    OSCILLATOR_Initialize(); // default 1Mhz

    UART_Write_Text("Echo test ready\r\n");

    while (1)
    {
        // Pure polling RX
        if (PIR1bits.RCIF)
        {
            // Handle overrun
            if (RCSTAbits.OERR) {
                RCSTAbits.CREN = 0;
                Nop();
                RCSTAbits.CREN = 1;
            }

            char c = RCREG;   // get byte from PC
            UART_Write(c);    // echo it back
        }
    }
}
