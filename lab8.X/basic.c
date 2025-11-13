// PIC18F4520 Configuration Bit Settings
// 'C' source line config statements

#pragma config OSC = INTIO67    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bits (Brown-out Reset enabled and controlled by software (SBOREN is enabled))
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <pic18f4520.h>
#define SERVO_COUNTS_M90   31      // -90°
#define SERVO_COUNTS_0     47      // 0°
#define SERVO_COUNTS_P90   63      // +90°
#define _XTAL_FREQ 125000UL   // 125 kHz oscillator
void PWM1_SetCounts(unsigned char counts);

volatile unsigned char button_event = 0;

void Button_Init(void){
    ADCON1 = 0x0F;               // PORTB digital

    // Enable PORTB internal pull-ups (RBPU = 0)
    INTCON2bits.RBPU = 0;        // 0 = enable pull-ups
    TRISBbits.TRISB0 = 1;        // RB0 input

    RCONbits.IPEN = 0;           // no priority
    INTCON2bits.INTEDG0 = 0;     // falling edge (1->0)

    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;
    INTCONbits.GIE    = 1;
}
void __interrupt() isr(void)
{
     if(INTCONbits.INT0IF){
        LATDbits.LATD7 ^= 1;
        
        button_event = 1;     // Just mark the event
        INTCONbits.INT0IF = 0;
    }
//     __delay_ms(20);
}
void PWM1_SetCounts(unsigned char counts)
{
    // upper bits (counts[9:2]) go to CCPR1L
    CCPR1L = counts >> 2;

    // lower 2 bits (counts[1:0]) go to DC1B1:DC1B0 = CCP1CON<5:4>
    CCP1CONbits.DC1B = counts & 0x03;
}
void main(void){
   TRISDbits.TRISD7 = 0;
    LATDbits.LATD7 = 0;
    Button_Init();
    
    // Timer2 -> On, prescaler -> 4
    T2CONbits.TMR2ON = 0b1;
    T2CONbits.T2CKPS = 0b01;

    // Internal Oscillator Frequency, Fosc = 125 kHz, Tosc = 8 µs
    OSCCONbits.IRCF = 0b001;
    
    // PWM mode, P1A, P1C active-high; P1B, P1D active-high
    CCP1CONbits.CCP1M = 0b1100;
    
    // CCP1/RC2 -> Output
    TRISC = 0;
    LATC = 0;
    
    // Set up PR2, CCP to decide PWM period and Duty Cycle
    /*
     * PWM period
     * = (PR2 + 1) * 4 * Tosc * (TMR2 prescaler)
     * = (0x9B + 1) * 4 * 8µs * 4
     * = 0.019968s ~= 20ms
     */
    PR2 = 0x9B; //expect a pulse every 20ms
    
    /*
     * Duty cycle
     * = (CCPR1L:CCP1CON<5:4>) * Tosc * (TMR2 prescaler)
     * = (0x0B*4 + 0b01) * 8µs * 4
     * = 0.00144s ~= 1450µs
     */
//    //set servo motor to 0 degree
//    CCPR1L = 0x0B;
//    CCP1CONbits.DC1B = 0b01;

    // step = 0: 0°
    // step = 1: +90°
    // step = 2: 0°
    // step = 3: -90°, then repeat
    
    unsigned char step = 0;
    while (1)
    {
        if (button_event)
        {
            button_event = 0;

            // simple debounce
            __delay_ms(20);                // allow bounce to stop
            while (PORTBbits.RB0 == 0);    // wait until button RELEASED
            __delay_ms(20);                // extra debounce

            // finally treat as one clean press
            step = (step + 1) & 0x03;
            switch(step){
            case 0: {PWM1_SetCounts(SERVO_COUNTS_0); break;}
            case 1: {PWM1_SetCounts(SERVO_COUNTS_P90); break;}
            case 2: {PWM1_SetCounts(SERVO_COUNTS_0); break;}
            case 3: {PWM1_SetCounts(SERVO_COUNTS_M90); break;}
            
        }
        // Optional settle delay, not required for control
        __delay_ms(20);
        }

        // existing servo control here...
    }
     
    
//    CCP1CONbits.CCP1M = 0;        // disable PWM module
//
//    TRISCbits.TRISC2 = 0;         // RC2 as output
//
//    while(1)
//    {
//        LATCbits.LATC2 ^= 1;      // toggle RC2
//        __delay_ms(500);
//    }

    return;
}
