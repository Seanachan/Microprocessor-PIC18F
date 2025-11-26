#include <xc.h>

void ADC_Initialize(void)
{
    TRISA = 0xff;  // Set as input port
    ADCON1 = 0x0e; // Ref vtg is VDD & Configure pin as analog pin
    // ADCON2 = 0x92;
    ADFM = 1;            // Right Justifie
    ADCON2bits.ADCS = 7; //
    ADRESH = 0;          // Flush ADC output Register
    ADRESL = 0;
}

int ADC_Read(int channel)
{
    int digital;

    // basic bounds check (optional but safer)
    if (channel < 0 || channel > 13)
        return 0;

    ADCON0bits.CHS = channel & 0x0F; // select channel
//    __delay_us(20);                  // acquisition delay

    ADCON0bits.GO = 1; // start conversion
    while (ADCON0bits.GO_nDONE)
        ; // wait until complete

    digital = ((int)ADRESH << 8) | ADRESL;
    return digital;
}
