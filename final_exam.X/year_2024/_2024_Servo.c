#include "setting_hardaware/setting.h"
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

// Keep System at 125kHz for UART stability
#define _XTAL_FREQ 125000
#define SERVO_COUNTS_M90 12 // ~1.0 ms pulse (? -90�)
#define SERVO_COUNTS_0 44   // ~1.5 ms pulse (center)
#define SERVO_COUNTS_P90 69 // ~2.0 ms pulse (? +90�)
// Servo connected to RC2
#define SERVO_PIN LATCbits.LATC2

char str[20];
volatile int cnt = 0;
volatile int button_flag = 0;
volatile int tick_limit = 5;
volatile int tick_count = 0;
volatile int motor_mode = 0;
volatile unsigned char current_counts = SERVO_COUNTS_M90;
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
void PWM1_SetCounts(unsigned char counts)
{
    // CCP1: servo on RC2
    CCPR1L = counts >> 2;
    CCP1CONbits.DC1B = counts & 0x03;
}

void PWM2_SetCounts(unsigned char counts)
{
    // CCP2: LED on RC1 (same Timer2)
    CCPR2L = counts >> 2;
    CCP2CONbits.DC2B = counts & 0x03;
}
void PWM_Init(void)
{
    // Internal oscillator 125 kHz
    OSCCONbits.IRCF = 0b001;

    // Timer2: prescaler=4, ON
    T2CONbits.T2CKPS = 0b01;
    T2CONbits.TMR2ON = 1;

    // PWM period ? 20 ms
    PR2 = 0x9B;

    // CCP1: PWM for servo (RC2)
    CCP1CONbits.CCP1M = 0b1100; // PWM mode
    TRISCbits.TRISC2 = 0;       // RC2 output
    LATCbits.LATC2 = 0;

    // // CCP2: PWM for LED (RC1)
    // CCP2CONbits.CCP2M = 0b1100; // PWM mode
    // TRISCbits.TRISC1 = 0;       // RC1 output
    // LATCbits.LATC1 = 0;

    // Initial position: -90°
    current_counts = SERVO_COUNTS_M90;
    PWM2_SetCounts(current_counts); // LED brightness reflects PWM
}
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
    // UART_Initialize();
    CCP1_Initialize(0);
    // ADC_Initialize();
    PWM_Init();
    // PIE1bits.RCIE = 1;

    while (1)
    {
        if (button_flag)
        {
            button_flag = 0;
            motor_mode ^= 1;
        }
        PWM2_SetCounts(current_counts);
    }
}

// ==========================================
//               INTERRUPTS
// ==========================================

void __interrupt(high_priority) Hi_ISR(void)
{
    // Check if TMR0 overflowed
    // if (INTCONbits.TMR0IF)
    // {
    //     // Reset Timer Preload for 50ms interval (Calculation below)
    //     // 4MHz -> 1us cycle. Prescaler 16 -> 16us per tick.
    //     // 25ms/16us = 1563 counts.
    //     // 50ms / 16us = 3125 counts.
    //     // 65536 - 3125 = 62411 = 0xF3CB
    //     INTCONbits.TMR0IF = 0;
    //     TMR0H = 0xF3;
    //     TMR0L = 0xCB;

    //     tick_count++;
    //     if (tick_count >= tick_limit)
    //     {
    //         tick_count = 0;
    //         LED--;
    //         if (LED < 0)
    //             LED = count_from;
    //         LATD = LED << 4;
    //     }
    // }
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
            button_flag = 1;
        }
    }
}