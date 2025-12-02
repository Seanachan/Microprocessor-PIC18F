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
int current_mode = 1;

// --- Function Prototypes ---
void PWM_Set_DutyCycle(unsigned int dutyCycle);
void Delay_Variable_us(int us);
void Servo_Delay(int us);
// ==========================================
//           OSCILLATOR HELPERS
// ==========================================
void Switch_Oscillator_125kHz(void)
{
    // Switch to 125 kHz for Servo Hardware PWM
    OSCCONbits.IRCF = 0b001;
    while (!OSCCONbits.IOFS)
        ; // Wait for stability
}

void Switch_Oscillator_4MHz(void)
{
    // Switch back to 4 MHz for UART
    OSCCONbits.IRCF = 0b110;
    while (!OSCCONbits.IOFS)
        ; // Wait for stability
}
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
    // STATIC variables remember their value even after function returns
    static int duty = 0;
    static int fading_in = 1;

    unsigned int adc_val;
    unsigned int step_delay;

    // 1. Set Brightness
    PWM_Set_DutyCycle(duty);

    // 2. Control Speed
    adc_val = ADC_Read(0);

    // --- CALIBRATION FIX ---
    // Previous tests showed 16 loops = 1.0 second.
    // Target 0.5s (8 loops) to 2.0s (32 loops).
    // Formula: Base 8 + (ADC scaled to ~24 range)
    step_delay = 8 + (adc_val / 42);

    Delay_Variable_us(step_delay);

    // 3. Logic Step
    // Limit duty to 500 because PR2=0x7C (124) gives max resolution of 500
    if (fading_in)
    {
        duty++;
        if (duty >= 500)
            fading_in = 0;
    }
    else
    {
        duty--;
        if (duty == 0)
            fading_in = 1;
    }
}
unsigned char servo_pos[] = {15, 15, 30, 44, 58, 73};
// --- HELPER BLOCK: MOVE SERVO TO START POSITION ---
// We wrap this logic here so we can call it initially
void Move_Servo_Start(int sec)
{
    // 1. Disable UART Interrupts (Critical!)

    Switch_Oscillator_125kHz(); // Go to Low Speed

    // Configure PWM Hardware (125kHz settings)
    T2CONbits.T2CKPS = 0b01; // Prescaler 4
    T2CONbits.TMR2ON = 1;
    PR2 = 0x9B;           // 20ms Period
    CCP1CON = 0x0C;       // PWM Mode
    TRISCbits.TRISC2 = 0; // Output on RC2

    // Set Duty Cycle for the specific second
    CCPR1L = servo_pos[sec] >> 2;
    CCP1CONbits.DC1B = servo_pos[sec] & 0x03;

    // Wait ~0.5 seconds for servo to travel
    // Note: At 125kHz (32x slower than 4MHz), __delay_ms(15) ~= 480ms real time
    for (int k = 0; k < 15; k++)
        __delay_ms(1);

    Switch_Oscillator_4MHz(); // Return to High Speed
}
void Mode3()
{
    UART_Write_Text("Mode 3: Countdown Clock.\r\n");
    UART_Write_Text("1. Enter Seconds (1-5) via UART\r\n");
    UART_Write_Text("2. Press Button (RB0) to Start.\r\n");

    // 2. Configure TIMER 0 (1 Second @ 4MHz)
    // Prescaler 1:32 -> T0CON = 0x84
    T0CON = 0x84;

    int seconds_to_count = 3; // Default
    // Servo Positions (Counts for 125kHz Clock)
    // 31 = -90deg, 47 = 0deg, 63 = +90deg

    // 1. Move to default (3s) immediately upon entering mode
    Move_Servo_Start(seconds_to_count);

    // Reset Hardware for Button Wait
    CCP1CON = 0;          // Disable PWM temporarily
    TRISCbits.TRISC2 = 0; // Output

    while (1)
    {
        // --- A. UART Input Check ---
        if (get_line_ready())
        {
            char *s = GetString();
            if (strstr(s, "m1"))
                return; // Exit to Menu

            if (s[0] >= '1' && s[0] <= '5')
            {
                seconds_to_count = s[0] - '0';
                UART_Write_Text("Set.\r\n");
            }
            ClearBuffer();
        }

        // --- B. Button Start Check ---
        if (button_flag)
        {

            button_flag = 0;
            UART_Write_Text("Go!\r\n");
            PIE1bits.RCIE = 0;
            while (PORTBbits.RB0 == 0)
                ; // Wait for button release

            Switch_Oscillator_125kHz();
            // A. Configure Hardware PWM for Servo
            // Matches the example code you provided
            T2CONbits.T2CKPS = 0b01; // Prescaler 4
            T2CONbits.TMR2ON = 1;
            PR2 = 0x9B;     // 20ms Period
            CCP1CON = 0x0C; // PWM Mode

            // B. Configure Timer 0 for 1 Second (at 125kHz)
            // Fcy = 31.25kHz. 1 sec = 31250 ticks.
            // 16-bit, No Prescaler (1:1)
            T0CON = 0x88;
            // Countdown Loop
            for (int i = seconds_to_count; i >= 0; i--)
            {
                char buffer[5];
                sprintf(buffer, "%d\r\n", i);
                UART_Write_Text(buffer);

                // Set Servo Angle using HW PWM
                CCPR1L = servo_pos[i] >> 2;
                CCP1CONbits.DC1B = servo_pos[i] & 0x03;

                // Wait 1 Second using Timer 0
                TMR0H = 0x85;
                TMR0L = 0xEE; // Load 34286
                INTCONbits.TMR0IF = 0;
                while (INTCONbits.TMR0IF == 0)
                    ; // Blocking wait
            }
            // Set Servo Angle using HW PWM
            CCPR1L = servo_pos[0] >> 2;
            CCP1CONbits.DC1B = servo_pos[0] & 0x03;
            Switch_Oscillator_4MHz();
            // Re-Initialize UART (Baud rate generator needs reset)
            UART_Initialize();
            if (RCSTAbits.OERR)
            {
                RCSTAbits.CREN = 0;
                NOP();
                RCSTAbits.CREN = 1;
            }
            // Flush any garbage characters currently in the buffer
            unsigned char dummy;
            while (PIR1bits.RCIF)
            {
                dummy = RCREG;
            }
            PIE1bits.RCIE = 1; // Re-enable Interrupts
            // Stop PWM
            UART_Write_Text("Done.\r\n");
        }
    }
}

// ==========================================
//               HELPERS
// ==========================================

void Delay_Variable_us(int us)
{
    while (us--)
    {
        __delay_us(1);
    }
}

void Servo_Delay(int us)
{
    // Calibrated for 4MHz loop overhead (Divide by 5)
    int loops = us / 5;
    while (loops--)
    {
        __delay_us(1);
    }
}

void PWM_Set_DutyCycle(unsigned int dutyCycle)
{
    if (dutyCycle > 1023)
        dutyCycle = 1023;
    CCP1CONbits.DC1B = dutyCycle & 0x03;
    CCPR1L = (dutyCycle >> 2) & 0xFF;
}

// ==========================================
//                 MAIN
// ==========================================

void main(void)
{
    ADCON1 = 0x0E;
    TRISBbits.RB0 = 1;
    TRISD = 0;
    LATD = 0;

    INTCON2bits.NOT_RBPU = 0;
    RCONbits.IPEN = 1;
    INTCON2bits.INTEDG0 = 0;
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    OSCILLATOR_Initialize();
    INTERRUPT_Initialize();
    UART_Initialize();
    CCP1_Initialize(0);
    ADC_Initialize();
    PIE1bits.RCIE = 1;

    UART_Write_Text("System Ready. Enter Mode:\r\n");

    while (1)
    {
        // --- 1. INPUT HANDLER ---
        if (get_line_ready())
        {
            char *s = GetString();

            if (strstr(s, "m1") != NULL)
            {
                current_mode = 1;
                UART_Write_Text("Mode 1\r\n");
            }
            else if (strstr(s, "m2") != NULL)
            {
                current_mode = 2;
                UART_Write_Text("Mode 2 (Breathing)\r\n");
                // Restore PWM settings for 4MHz Breathing
                CCP1_Initialize(0);
            }
            else if (strstr(s, "m3") != NULL)
            {
                current_mode = 3;
                UART_Write_Text("Mode 3 (Countdown)\r\n");
            }
            ClearBuffer();
        }

        // --- 2. TASK RUNNER ---
        switch (current_mode)
        {
        case 1:
            break;
        case 2:
            Mode2();
            break;
        case 3:
            Mode3();
            break;
        }
    }
}

// ==========================================
//               INTERRUPTS
// ==========================================

void __interrupt(high_priority) Hi_ISR(void)
{
    // FIX: Uncommented the IF check!
    // Otherwise UART interrupts would trigger button logic.
    if (INTCONbits.INT0IF)
    {
        __delay_ms(20); // Simple Debounce

        if (PORTBbits.RB0 == 0)
        {
            button_flag = 1;
        }

        INTCONbits.INT0IF = 0;
    }
}
