#include <xc.h>

void CCP1_Initialize(unsigned char mode)
{
    switch (mode)
    {
    case 0: // PWM Mode
        CCP1CON = 0x0C;
        PR2 = 0x9B;              // Set PWM period (Freq ~ 3.9kHz @ 4MHz)
        T2CONbits.TMR2ON = 0b1;  // Timer2 on, prescaler 1
        T2CONbits.T2CKPS = 0b01; // Prescaler = 4
        TRISCbits.TRISC2 = 0;    // RC2 pin is output.
        break;
    case 1: // Capture Mode
        // Timer 1 or 3 is required for Capture (Assuming Timer 1 here)
        T1CON = 0x80;   // Ensure Timer 1 is configured (16-bit, etc.)
        CCP1CON = 0x05; // 0x05 = Capture mode, every rising edge
                        // (Use 0x04 for falling edge)

        TRISCbits.TRISC2 = 1; // **CRITICAL**: RC2 must be INPUT for Capture
        break;
    case 2: // Compare Mode
        // Timer 1 or 3 is required for Compare (Assuming Timer 1 here)
        T1CON = 0x80;         // Ensure Timer 1 is configured
        CCP1CON = 0x02;       // 0x02 = Compare mode, toggle output on match
                              // (Use 0x08 to force Set, 0x09 to force Clear)
        TRISCbits.TRISC2 = 0; // RC2 must be OUTPUT for Compare
        break;
    default:
        break;
    }

    PIR1bits.CCP1IF = 0;
    IPR1bits.CCP1IP = 1;
}
