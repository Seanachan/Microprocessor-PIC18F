#include "p18f4520.inc"

; CONFIG1H
  CONFIG  OSC = INTIO67         ; Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
  CONFIG  FCMEN = OFF           ; Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
  CONFIG  IESO = OFF            ; Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

; CONFIG2L
  CONFIG  PWRT = OFF            ; Power-up Timer Enable bit (PWRT disabled)
  CONFIG  BOREN = SBORDIS       ; Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
  CONFIG  BORV = 3              ; Brown Out Reset Voltage bits (Minimum setting)

; CONFIG2H
  CONFIG  WDT = OFF             ; Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
  CONFIG  WDTPS = 32768         ; Watchdog Timer Postscale Select bits (1:32768)

; CONFIG3H
  CONFIG  CCP2MX = PORTC        ; CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
  CONFIG  PBADEN = ON           ; PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
  CONFIG  LPT1OSC = OFF         ; Low-Power Timer1 Oscillator Enable bit (Timer1 configured for higher power operation)
  CONFIG  MCLRE = ON            ; MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

; CONFIG4L
  CONFIG  STVREN = ON           ; Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
  CONFIG  LVP = OFF             ; Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
  CONFIG  XINST = OFF           ; Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

; CONFIG5L
  CONFIG  CP0 = OFF             ; Code Protection bit (Block 0 (000800-001FFFh) not code-protected)
  CONFIG  CP1 = OFF             ; Code Protection bit (Block 1 (002000-003FFFh) not code-protected)
  CONFIG  CP2 = OFF             ; Code Protection bit (Block 2 (004000-005FFFh) not code-protected)
  CONFIG  CP3 = OFF             ; Code Protection bit (Block 3 (006000-007FFFh) not code-protected)

; CONFIG5H
  CONFIG  CPB = OFF             ; Boot Block Code Protection bit (Boot block (000000-0007FFh) not code-protected)
  CONFIG  CPD = OFF             ; Data EEPROM Code Protection bit (Data EEPROM not code-protected)

; CONFIG6L
  CONFIG  WRT0 = OFF            ; Write Protection bit (Block 0 (000800-001FFFh) not write-protected)
  CONFIG  WRT1 = OFF            ; Write Protection bit (Block 1 (002000-003FFFh) not write-protected)
  CONFIG  WRT2 = OFF            ; Write Protection bit (Block 2 (004000-005FFFh) not write-protected)
  CONFIG  WRT3 = OFF            ; Write Protection bit (Block 3 (006000-007FFFh) not write-protected)

; CONFIG6H
  CONFIG  WRTC = OFF            ; Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
  CONFIG  WRTB = OFF            ; Boot Block Write Protection bit (Boot block (000000-0007FFh) not write-protected)
  CONFIG  WRTD = OFF            ; Data EEPROM Write Protection bit (Data EEPROM not write-protected)

; CONFIG7L
  CONFIG  EBTR0 = OFF           ; Table Read Protection bit (Block 0 (000800-001FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR1 = OFF           ; Table Read Protection bit (Block 1 (002000-003FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR2 = OFF           ; Table Read Protection bit (Block 2 (004000-005FFFh) not protected from table reads executed in other blocks)
  CONFIG  EBTR3 = OFF           ; Table Read Protection bit (Block 3 (006000-007FFFh) not protected from table reads executed in other blocks)

; CONFIG7H
  CONFIG  EBTRB = OFF           ; Boot Block Table Read Protection bit (Boot block (000000-0007FFh) not protected from table reads executed in other blocks)

    org 0x00
    bulb EQU 0x15
    state EQU 0x16
    
goto Initial	
 
 org 0x08
GOTO ISR_high
 
org 0x18
GOTO ISR_low
 
ISR_low:
    MOVLW 0x02
    ADDWF bulb, f
    MOVLW 0x0F
    ANDWF bulb, f
    SWAPF bulb, w
    MOVWF LATD               ; interrupt???LATA??
    
;    __retfie:
	BCF PIR1, TMR2IF        ; ??????TMR2IF?? (??flag bit)
	RETFIE 
ISR_high:				
;    org 0x08                ; ????: ?0.5??????interrupt
    BTG state, 0
    
    BTFSC state, 0
    GOTO to_state2
    
    GOTO to_state1
    
    to_state2:
	MOVLW d'244'
	MOVWF PR2
	MOVLW 0x01
	MOVWF bulb
	GOTO _retfie
    to_state1:
	MOVLW d'61'
	MOVWF PR2
	CLRF bulb
	GOTO _retfie
    
    _retfie:
	BCF INTCON, INT0IF        ; ??????TMR2IF?? (??flag bit)
	RETFIE  
 
Initial:
    CLRF bulb
    CLRF state
    
    MOVLW 0x0F
    MOVWF ADCON1
    CLRF TRISD
    CLRF LATD
    BSF RCON, IPEN		;Enable 2-level priority mode
    
    BSF INTCON, GIEH            ; Enable high-prior interrupts
    BSF INTCON, GIEL            ; Enable low-prior interrupts
    BSF INTCON, PEIE		; Enable peripheral interrupt
    
    ; PIR: Peripheral Interrupt Request
    BCF PIR1, TMR2IF		; Clear Timer2 Interrupt Flag, IF=Interrupt Flag
    BCF INTCON, INT0IF		; Clear External Interrupt 0
    
    ; IPR: Interrupt Priority
;    BSF IPR1, INT0IP            ; Set External Interrupt Priority = 1
			        ; IP = Interrupt Priority 1=> High Prior, 0=>low Prior
    BCF IPR1, TMR2IP		; Explicitly set timer2 prior = 0
				
    BSF PIE1, TMR2IE            ; Enable Timer 2 Interrupt 
    BSF INTCON, INT0IE		; Enable External Interrupt
    
    MOVLW b'11111111'	        ; Set both pre/post scale to 1:16, TIMER2++ after 256 cycles
    MOVWF T2CON		    ; TIMER = OSC/4
    MOVLW D'61'		; 256 * 4 = 1024 cycles to TIMER2 ++
    MOVWF PR2			; 250khz => Delay 0.5 s == 125000cycles
				; set PR2 = 125000 / 1024 = 122.0703125 == 122
    MOVLW D'00100000'
    MOVWF OSCCON	        ; set osc to 250kHz
    
main:		
    bra main	    

    
end
