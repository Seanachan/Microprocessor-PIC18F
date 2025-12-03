// PIC18F4520 Configuration Bit Settings

#include <xc.h>
#include <pic18f4520.h>

#define _XTAL_FREQ 125000UL // 125 kHz

// Duty counts for your Timer2 setup (Fosc=125kHz, presc=4, PR2=0x9B)

// #define SERVO_STEP_COUNTS 15 // ~4.3 degrees per step
#define SERVO_COUNTS_M90 11 // ~1.0 ms pulse (? -90�)
#define SERVO_COUNTS_0 41   // ~1.5 ms pulse (center)
#define SERVO_COUNTS_P90 70 // ~2.0 ms pulse (? +90�)

volatile unsigned char button_event = 0;
volatile int SERVO_STEP_COUNTS = 7;
unsigned char deg_array[] = {11, 26, 41, 56, 70};
unsigned char deg_index = 0;
unsigned char stride = 1;

unsigned char current_counts = SERVO_COUNTS_M90; // start at -90
unsigned char target_counts = SERVO_COUNTS_M90;
signed char direction = +1; // +1 toward +90, -1 toward -90
unsigned char moving = 0;   // 0 = idle, 1 = moving

void PWM1_SetCounts(unsigned char counts)
{
    // 10-bit duty: (CCPR1L:DC1B) = counts
    CCPR1L = counts >> 2;             // upper 8 bits
    CCP1CONbits.DC1B = counts & 0x03; // lower 2 bits
}

void Button_Init(void)
{
    ADCON1 = 0x0F; // PORTB digital

    INTCON2bits.RBPU = 0; // enable PORTB internal pull-ups
    TRISBbits.TRISB0 = 1; // RB0 input (button)

    RCONbits.IPEN = 0;       // no priority
    INTCON2bits.INTEDG0 = 0; // INT0 on falling edge (1 -> 0)

    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;
    INTCONbits.GIE = 1;
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

    // CCP2: PWM for LED (RC1)
    CCP2CONbits.CCP2M = 0b1100; // PWM mode
    TRISCbits.TRISC1 = 0;       // RC1 output
    LATCbits.LATC1 = 0;

    // Initial position: -90
    current_counts = SERVO_COUNTS_M90;
    PWM1_SetCounts(current_counts);
    //    PWM2_SetCounts(current_counts); // LED brightness reflects PWM
}
void __interrupt() isr(void)
{
    if (INTCONbits.INT0IF)
    {
        // Edge detected on RB0
        if (PORTBbits.RB0 == 0)
        {
            button_event = 1; // mark event; main will debounce
            INTCONbits.INT0IF = 0;
            SERVO_STEP_COUNTS *= 2;
            if (SERVO_STEP_COUNTS > 56)
            {
                SERVO_STEP_COUNTS = 11;
            }
        }
    }
}

void main(void)
{
    // Debug LED on RD7 (optional)
    TRISDbits.TRISD7 = 0;
    LATDbits.LATD7 = 0;

    Button_Init();

    // Internal oscillator 125 kHz
    OSCCONbits.IRCF = 0b001;

    // Timer2: prescaler 4, ON
    T2CONbits.T2CKPS = 0b01;
    T2CONbits.TMR2ON = 1;

    // PWM mode on CCP1, output on RC2
    CCP1CONbits.CCP1M = 0b1100;
    TRISCbits.TRISC2 = 0;
    LATCbits.LATC2 = 0;

    // ~20 ms PWM period
    PR2 = 0x9B;

    // Initial position: -90
    current_counts = SERVO_COUNTS_M90;
    target_counts = SERVO_COUNTS_M90;
    PWM1_SetCounts(current_counts);

    while (1)
    {
        // ----- Handle button press (edge + debounce) -----
        if (button_event && !moving)
        {
            button_event = 0;

            // Debounce: Wait until button is released (RB0 back high)
            while (PORTBbits.RB0 == 0)
                ;
            __delay_ms(20);

            // Compute new target
            // int tmp = (int)current_counts + (int)direction * SERVO_STEP_COUNTS;
            int next_idx = (int)deg_index + direction * stride;

            if (next_idx > 4)
            {
                // target_counts = SERVO_COUNTS_P90 - (tmp - SERVO_COUNTS_P90); // bounce back
                // direction = -1;                                              // reverse for next press
                next_idx = 4 - (next_idx - 4);
                direction = -1;
            }
            else if (next_idx < 0)
            {
                next_idx = 0 + (0 - next_idx);
                direction = +1;
            }
            deg_index = (unsigned char)next_idx;
            target_counts = deg_array[deg_index];

            moving = 1;          // start gradual motion
            LATDbits.LATD7 ^= 1; // debug: toggle LED per press

            stride *= 2;
            if (stride > 4)
                stride = 1;
        }

        // ----- Gradual motion -----
        if (moving)
        {
            if (current_counts < target_counts)
            {
                current_counts++;
            }
            else if (current_counts > target_counts)
            {
                current_counts--;
            }

            PWM1_SetCounts(current_counts);

            // Small delay between position steps for smooth movement
            __delay_ms(20);

            if (current_counts == target_counts)
            {
                moving = 0; // reached target; wait for next button press
            }
        }
        else
        {
            // idle: small delay to avoid busy-waiting too hard
            __delay_ms(5);
        }
    }
}
