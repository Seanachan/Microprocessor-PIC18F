#include <xc.h>

#pragma config OSC = INTIO67 // Oscillator Selection bits
#pragma config WDT = OFF     // Watchdog Timer Enable bit
#pragma config PWRT = OFF    // Power-up Enable bit
#pragma config BOREN = ON    // Brown-out Reset Enable bit
#pragma config PBADEN = OFF  // Watchdog Timer Enable bit
#pragma config LVP = OFF     // Low Voltage (single -supply) In-Circute Serial Pragramming Enable bit
#pragma config CPD = OFF     // Data EEPROM?Memory Code Protection bit (Data EEPROM code protection off)
unsigned short nums[8]={2,0,2,5,1,1,1,9};

void __interrupt(high_priority)H_ISR(){
    
    //step4
    int value = (ADRESH<<8) | ADRESL;
    
    
    //do things
    unsigned int index = value / 128;
    
    LATC = nums[index];
    
    
    //clear flag bit
    PIR1bits.ADIF = 0;
    
    
    //step5 & go back step3
    /*
    delay at least 2tad
    ADCON0bits.GO = 1;
    */
    ADCON0bits.GO = 1;
    
    return;
}

void main(void) 
{
    /*
     * 4 LEDs to represent decimal numbers 0-9
     * When variable resistor rotate at constant speed
     * 2->0->2->5->1->1->1->9 (according to today's date)
     */
    //configure OSC and port
    OSCCONbits.IRCF = 0b100; //1MHz
    TRISAbits.RA0 = 1;       //analog input port
    
    //Config LED output
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;
    
        
    //step1: Config ADC module
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    ADCON1bits.PCFG = 0b1110; //AN0 as analog input, other digital
    ADCON0bits.CHS = 0b0000;  //AN0 as analog input
    ADCON2bits.ADCS = 0b000;  //look up table , set 000(1MHz < 2.86Mhz)
    ADCON2bits.ACQT = 0b001;  //Tad = 2 \mu s acquisition time set 2Tad = 4 > 2.4
    ADCON0bits.ADON = 1;
    ADCON2bits.ADFM = 1;    //right justified 
    
    
    //step2: Config ADC interrupt
    PIE1bits.ADIE = 1; //allow ADC interrupt
    PIR1bits.ADIF = 0; //ADC interrupty flag = 0
    INTCONbits.PEIE = 1; //Peripheral Enable Interrupt Enable => Allow inputs other than core interrupts
    INTCONbits.GIE = 1; //global interrupt enable

    
    //step3: Start Conversion
    ADCON0bits.GO = 1;
    
    while(1);
    
    return;
}
