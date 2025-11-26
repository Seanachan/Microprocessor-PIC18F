#include "setting_hardaware/setting.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#define _XTAL_FREQ 4000000 // Change to your actual clock frequency
char str[20];
volatile int cnt = 0;
volatile int button_flag = 0;
void Mode1()
{
    return;
}
void Mode2()
{ // Todo : Mode2
    return;
}
void main(void)
{
    ADCON1 = 0x0F;
    TRISBbits.RB0 = 1; // Set RB0 as input
    TRISD = 0;
    LATD = 0;
    // Set Interrupt
    // Enable PORTB weak pull-ups for the Active-Low button press
    INTCON2bits.NOT_RBPU = 0;

    // Enable Priority System (Mandatory when using GIEH/GIEL)
    RCONbits.IPEN = 1;

    // Set Edge Trigger for INT0
    INTCON2bits.INTEDG0 = 0; // Falling edge trigger (button press)

    // --- 3. INTERRUPT ENABLE ---
    INTCONbits.INT0IF = 0; // Clear the flag initially
    INTCONbits.INT0IE = 1; // Enable INT0 Interrupt

    // Global Interrupt Enable (MUST use GIEH/GIEL when IPEN=1)
    INTCONbits.GIEH = 1; // Enable HIGH Priority Interrupts (INT0 runs here)
    INTCONbits.GIEL = 1; // Enable LOW Priority Interrupts (Good practice)

    SYSTEM_Initialize();

     while (1)
     {
    //     UART_Write_Text("Enter Mode:\r\n");
    //     strcpy(str, GetString());
    //     if (str[0] == 'm' && str[1] == '1')
    //     { // Mode1
    //         Mode1();
    //         ClearBuffer();
    //     }
    //     else if (str[0] == 'm' && str[1] == '2')
    //     { // Mode2
    //         Mode2();
    //         ClearBuffer();
    //     }
     }
    return;
}

void __interrupt(high_priority) Hi_ISR(void)
{
    if (INTCONbits.INT0IF) // Check INT0 interrupt flag
    {
        __delay_ms(20);
        if (PORTBbits.RB0 == 0)
        {
            cnt++;
            cnt %= 16; // 0-15
            LATD = cnt << 4;

            UART_Write('\b');
            UART_Write(' ');
            UART_Write('\b');
            UART_Write(' ');
            UART_Write('\r');

            sprintf(str, "%d", cnt);
            UART_Write_Text(str);
        }
        INTCONbits.INT0IF = 0;
    }
}
