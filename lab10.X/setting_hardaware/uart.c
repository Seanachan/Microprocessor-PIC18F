#include <xc.h>
// setting TX/RX

char mystring[20];
int lenStr = 0;
int line_ready = 0;

void UART_Initialize()
{
    /*TODObasic
           Serial Setting
        1.   Setting Baud rate
        2.   choose sync/async mode
        3.   enable Serial port (configures RX/DT and TX/CK pins as serial port pins)
        3.5  enablea Tx, Rx Interrupt(optional)
        4.   Enable Tx & RX
    */

    // Setting UART pins
    TRISCbits.TRISC6 = 1; // Set TX pin as output
    TRISCbits.TRISC7 = 1; // Set RX pin as input
    RCSTAbits.SPEN = 1;   // Serial port enable

    //  Setting baud rate
    TXSTAbits.SYNC = 0;    // Asynchronous mode
    BAUDCONbits.BRG16 = 0; // 8-bit Baud Rate Generator
    TXSTAbits.BRGH = 0;    // Low speed
    SPBRG = 51;            // 1200bps at 4MHz

    //   Serial enable
    TXSTAbits.TXEN = 1; // Transmit enable, allow chip to send to TX pin
    // PIR1bits.TXIF = 0;  // Transmit interrupt flag, set to 1 force interrupt
    PIE1bits.TXIE = 0; // Transmit interrupt enable, will check TXIF flag and trigger interrupt
    IPR1bits.TXIP = 0; // Transmit interrupt priority, set to low priority

    //   Receive enable
    PIR1bits.RCIF = 0;  // Receive interrupt flag
    RCSTAbits.CREN = 1; // Continuous receive enable, actively listen to RX pin
    PIE1bits.RCIE = 0;  // Receive interrupt enable, check RCIF flag and trigger interrupt
    IPR1bits.RCIP = 0;  // Receive interrupt priority, set to high priority
}

void UART_Write(unsigned char data) // Output on Terminal
{
    while (!TXSTAbits.TRMT)
        ;
    TXREG = data; // write to TXREG will send data
}

void UART_Write_Text(char *text)
{ // Output on Terminal, limit:10 chars
    for (int i = 0; text[i] != '\0'; i++)
        UART_Write(text[i]);
}

void ClearBuffer()
{
    for (int i = 0; i < 10; i++)
        mystring[i] = '\0';
    lenStr = 0;
    line_ready = 0;
}

void MyusartRead()
{
    char received = RCREG;

    // If user presses Enter, finish the string
    if (received == '\r' || received == '\n')
    {
        mystring[lenStr] = '\0'; // terminate C-string
        UART_Write('\r');
        UART_Write('\n');
        line_ready = 1;
        return;
    }

    // Store character if there is space
    if (lenStr < (int)(sizeof(mystring) - 1))
    {
        mystring[lenStr++] = received;
        UART_Write(received); // echo back
    }
}

char *GetString()
{
    return mystring;
}

// void interrupt low_priority Lo_ISR(void)
void __interrupt(low_priority) Lo_ISR(void)
{
    if (PIR1bits.RCIF)
    {
        if (RCSTAbits.OERR)
        {
            RCSTAbits.CREN = 0;
            Nop();
            RCSTAbits.CREN = 1;
        }

        MyusartRead();
    }

    // process other interrupt sources here, if required
    return;
}

void wait_for_line()
{
    while (!line_ready)
        ;
}