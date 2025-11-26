#include "setting_hardaware/setting.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#define _XTAL_FREQ 4000000 // Change to your actual clock frequency
char str[20];
volatile int cnt = 0;
volatile int timer_counter = 0;
volatile int time_interval = 1000;

int map(int val)
{
    int output = 0;
    if (val < 85)
        output = 4;
    else if (val < 170)
        output = 5;
    else if (val < 256)
        output = 6;
    else if (val < 341)
        output = 7;
    else if (val < 426)
        output = 8;
    else if (val < 512)
        output = 9;
    else if (val < 597)
        output = 10;
    else if (val < 682)
        output = 11;
    else if (val < 768)
        output = 12;
    else if (val < 853)
        output = 13;
    else if (val < 938)
        output = 14;
    else
        output = 15;

    return output;
}
void main(void)
{
    TRISBbits.RB0 = 1; // Set RB0 as input
    TRISD = 0;
    LATD = 0;

    //    Timer2_Init();
    SYSTEM_Initialize();

    PIE1bits.RCIE = 1;   // enable RX interrupt
    ADCON0bits.ADON = 1; // enable ADC
    UART_Write_Text("Displaying level 4-15:\r\n");
    int val = ADC_Read(0);
    int level = 0, prev_level = 0;
    while (1)
    {
        int adc = ADC_Read(0);
        level = map(adc);
        if (level != prev_level)
        {
            if(prev_level>=10){
                UART_Write('\b');
                UART_Write(' ');
                UART_Write('\b');
                UART_Write('\r');
            }
            else{
                UART_Write('\r');
                
            }
            prev_level = level;
            sprintf(str, "%d", level);
            UART_Write_Text(str);
            LATD = level<<4;
        }
        __delay_ms(20);
    }

    return;
}

void __interrupt(high_priority) Hi_ISR(void)
{
    
}
