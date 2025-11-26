#include "setting_hardaware/setting.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#define _XTAL_FREQ 4000000 // Change to your actual clock frequency
char str[20];
volatile int cnt = 0;
volatile int timer_counter = 0;
volatile int time_interval = 1000;
void Mode1()
{ // Todo : Mode2
    return;
}
void Mode2()
{ // Todo : Mode2
    return;
}
void Timer2_Init(void)
{
    T2CONbits.T2CKPS = 0b01;    // Prescaler 1:4
    T2CONbits.T2OUTPS = 0b0000; // Postscaler 1:1

    PR2 = 249; // Set Timer2 period for 1 second at 4MHz with 1:4 prescaler
    TMR2 = 0;  // Clear Timer2 register

    PIR1bits.TMR2IF = 0; // Clear Timer2 interrupt flag
    IPR1bits.TMR2IP = 1; // Set Timer2 interrupt priority to high
    PIE1bits.TMR2IE = 1; // Enable Timer2 interrupt

    T2CONbits.TMR2ON = 1; // Turn on Timer2
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
    INTCONbits.PEIE = 1;   // Peripheral Interrupt Enable

    // Global Interrupt Enable (MUST use GIEH/GIEL when IPEN=1)
    INTCONbits.GIEH = 1; // Enable HIGH Priority Interrupts (INT0 runs here)
    INTCONbits.GIEL = 1; // Enable LOW Priority Interrupts (Good practice)

    Timer2_Init();
    SYSTEM_Initialize();

PIE1bits.RCIE = 1;      // enable RX interrupt
    UART_Write_Text("Enter time interval in seconds (0.001 to 1.0):\r\n");
    while (1)
    {
        ClearBuffer();
        wait_for_line();
        char *s = GetString();
        float num = atof(s);
        if (num > 0 && num <= 1.0f)
        {
            time_interval = (int)(num * 1000.0f); // num seconds → milliseconds
        }
    }

    return;
}

void __interrupt(high_priority) Hi_ISR(void)
{
    if (PIR1bits.TMR2IF)
    {
        timer_counter++;
        PIR1bits.TMR2IF = 0; // clear flag

        if (timer_counter >= time_interval)
        {
            //            UART_Write_Text("Enter time interval in seconds (0.001 to 1.0):\r\n");
            timer_counter = 0;
            PIR1bits.TMR2IF = 0; // (this second clear is redundant)
            cnt++;
            cnt %= 16;
            LATD = cnt << 4;
            //            timer_flag = 1;

            PIR1bits.TMR2IF = 0; // clear flag

            LATD ^= 0xFF; // toggle RD0 every 1 ms
        }
    }
    if (PIR1bits.RCIF)
    {
        char c = RCREG;
        UART_Write(c); // echo
    }

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
