#include <xc.h>
#include <pic18f4520.h>

#define _XTAL_FREQ        125000UL    // 125 kHz internal oscillator

// Servo duty counts for Timer2-based PWM (Fosc=125k, presc=4, PR2=0x9B)
#define SERVO_COUNTS_M90  31          // ~1.0 ms pulse (? -90°)
#define SERVO_COUNTS_0    47          // ~1.5 ms pulse (center)
#define SERVO_COUNTS_P90  63          // ~2.0 ms pulse (? +90°)

// Round-trip timing: 1 count per ~80 ms ? 64 counts ? 5.1 s
#define T0_TICKS_PER_STEP 8           // 8 * 10 ms = 80 ms

// Shared variables with ISR
volatile unsigned char button_event   = 0;
volatile unsigned char running        = 0;    // 0=stopped, 1=running
volatile unsigned char t0_ticks       = 0;
volatile unsigned char current_counts = SERVO_COUNTS_M90;
volatile signed   char direction      = +1;   // +1 toward +90, -1 toward -90

// ---------- PWM helpers ----------

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

// ---------- Button / INT0 init ----------

void Button_Init(void)
{
    ADCON1 = 0x0F;               // all digital

    // RB0 as input with internal pull-up
    INTCON2bits.RBPU = 0;        // enable PORTB pull-ups
    TRISBbits.TRISB0 = 1;        // RB0 input

    // Interrupt configuration
    RCONbits.IPEN   = 0;         // no priority
    INTCON2bits.INTEDG0 = 0;     // falling edge trigger

    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;
    INTCONbits.GIE    = 1;
}

// ---------- Timer0 init (for motion timing) ----------
//
// Fosc = 125 kHz ? instruction clock Fcy = Fosc/4 = 31.25 kHz
// Tcy = 32 us
// We want ~10 ms per overflow:
//   10 ms / 32 us ? 312.5 cycles ? use 312 cycles
//   preload = 65536 - 312 = 65224 = 0xFE78
// Timer0: 16-bit, internal clock, no prescaler

void Timer0_Init(void)
{
    T0CONbits.T08BIT = 0;        // 16-bit
    T0CONbits.T0CS   = 0;        // internal clock
    T0CONbits.T0SE   = 0;        // rising edge (don't care here)
    T0CONbits.PSA    = 1;        // no prescaler (1:1)

    TMR0H = 0xFE;                // preload for ~10 ms
    TMR0L = 0x78;

    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;       // enable Timer0 interrupt

    T0CONbits.TMR0ON = 1;        // start Timer0
}

// ---------- Timer2 & PWM init ----------

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
    CCP1CONbits.CCP1M = 0b1100;   // PWM mode
    TRISCbits.TRISC2  = 0;        // RC2 output
    LATCbits.LATC2    = 0;

    // CCP2: PWM for LED (RC1)
    CCP2CONbits.CCP2M = 0b1100;   // PWM mode
    TRISCbits.TRISC1  = 0;        // RC1 output
    LATCbits.LATC1    = 0;

    // Initial position: -90°
    current_counts = SERVO_COUNTS_M90;
    PWM1_SetCounts(current_counts);
    PWM2_SetCounts(current_counts);    // LED brightness reflects PWM
}

// ---------- Interrupt service routine ----------

void __interrupt() isr(void)
{
    // RB0 INT0 (button edge)
    if (INTCONbits.INT0IF)
    {
        button_event = 1;        // mark event; main will debounce + toggle
        INTCONbits.INT0IF = 0;
    }

    // Timer0 overflow every ~10 ms
    if (INTCONbits.TMR0IF)
    {
        // Reload for ~10 ms
        TMR0H = 0xFE;
        TMR0L = 0x78;

        if (running)
        {
            t0_ticks++;
            if (t0_ticks >= T0_TICKS_PER_STEP)
            {
                t0_ticks = 0;

                // Move servo 1 count toward the current direction
                if (direction > 0)
                {
                    if (current_counts < SERVO_COUNTS_P90)
                        current_counts++;

                    // At +90°, reverse direction
                    if (current_counts >= SERVO_COUNTS_P90)
                        direction = -1;
                }
                else // direction < 0
                {
                    if (current_counts > SERVO_COUNTS_M90)
                        current_counts--;

                    // At -90°, reverse direction
                    if (current_counts <= SERVO_COUNTS_M90)
                        direction = +1;
                }
            }
        }

        INTCONbits.TMR0IF = 0;
    }
}

// ---------- Main ----------

void main(void)
{
    // Extra LED indicator (e.g. on RD7) to show running/idle state (optional)
    TRISDbits.TRISD7 = 0;
    LATDbits.LATD7   = 0;

    Button_Init();
    Timer0_Init();
    PWM_Init();

    while (1)
    {
        // ----- Handle button press (edge-trigger + debounce) -----
        if (button_event)
        {
            button_event = 0;

            // Debounce: wait for bounce to settle
            __delay_ms(20);
            while (PORTBbits.RB0 == 0) ;   // wait until button released
            __delay_ms(20);

            // Toggle running state
            running ^= 1;                  // 0->1 or 1->0

            // Optional: debug LED for running/idle
            LATDbits.LATD7 = running;
        }

        // ----- Apply current PWM values -----
        // Servo position updated in Timer0 ISR via current_counts
        PWM1_SetCounts(current_counts);

        // LED brightness tracks duty cycle continuously
        PWM2_SetCounts(current_counts);

        // Small idle delay so main loop isn't totally tight
        __delay_ms(5);
    }
}
