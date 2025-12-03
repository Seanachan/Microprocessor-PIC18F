#include "setting_hardaware/setting.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

// Keep System at 4MHz for UART stability
#define _XTAL_FREQ 4000000

// Servo connected to RD7
#define SERVO_PIN LATCbits.LATC2

char str[20];
volatile int cnt = 0;
volatile int button_flag = 0;
volatile int tick_limit = 5;
volatile int tick_count = 0;
volatile int motor_mode = 0;
int state = 0, count_from = 4;
int LED = 0;
// ==========================================
//              MODE FUNCTIONS
// ==========================================

void Mode1()
{
    // Mode 1 is just the menu/idle state
    return;
}

void Mode2()
{
    return;
}

void Mode3()
{
    return;
}

// ==========================================
//               HELPERS
// ==========================================

// ==========================================
//                 MAIN
// ==========================================

void main(void)
{
    ADCON1 = 0x0F;
    TRISBbits.RB0 = 1;
    TRISD = 0;
    LATD = 0xFF;

    INTCON2bits.NOT_RBPU = 0;
    INTCON2bits.INTEDG0 = 0;
    INTCONbits.INT0IF = 0;
    // INTCONbits.INT0IE = 1;

    // Timer0 Config
    T0CON = 0b10000011;

    // Preload Timer0 for 1 second interrupt at 4MHz
    TMR0H = 0xF3;
    TMR0L = 0xCB;

    INTCONbits.TMR0IE = 1; // Enable Timer0 interrupt
    INTCONbits.TMR0IF = 0; // Clear Timer0 interrupt flag
    INTCONbits.GIE = 1;    // Enable all interrupts

    OSCILLATOR_Initialize();
    INTERRUPT_Initialize();
    UART_Initialize();
    // CCP1_Initialize(0);
    // ADC_Initialize();
    PIE1bits.RCIE = 1;

    UART_Write_Text("System Ready. Enter Mode:\r\n");
    while (1)
    {
        if (get_line_ready())
        {
            char *s = GetString();
            state = s[0] - '0';
            count_from = s[2] - '0';

            char debug_msg[30];
            sprintf(debug_msg, "\r\nGot: %d and %d\r\n", state, count_from);
            UART_Write_Text(debug_msg);

            if (state == 0)
            {
                tick_limit = 5;
            }
            else if (state == 1)
            {
                tick_limit = 10;
            }
            else if (state == 2)
            {
                tick_limit = 20;
            }
            else
            {
                tick_limit = 20;
            }
            ClearBuffer();
        }
        __delay_ms(20);
    }
}

// ==========================================
//               INTERRUPTS
// ==========================================

void __interrupt(high_priority) Hi_ISR(void)
{
    // Check if TMR0 overflowed
    if (INTCONbits.TMR0IF)
    {
        // Reset Timer Preload for 50ms interval (Calculation below)
        // 4MHz -> 1us cycle. Prescaler 16 -> 16us per tick.
        // 25ms/16us = 1563 counts.
        // 50ms / 16us = 3125 counts.
        // 65536 - 3125 = 62411 = 0xF3CB
        INTCONbits.TMR0IF = 0;
        TMR0H = 0xF3;
        TMR0L = 0xCB;

        tick_count++;
        if (tick_count >= tick_limit)
        {
            tick_count = 0;
            LED--;
            if (LED < 0)
                LED = count_from;
            LATD = LED << 4;
        }
    }
    if (INTCONbits.INT0IF)
    {
        if (PORTBbits.RB0 == 0)
        {
            // 1a set Time Interval of LED change
            // button_flag = 1;
            // INTCONbits.INT0IF = 0;
            // tick_limit *= 2;
            // if (tick_limit > 20)
            // {
            //     tick_limit = 5;
            // }

            // Toggle Motor Mode
            motor_mode ^= 1;
            
        }
    }
}