LIST p=18f4520
#include<p18f4520.inc>

    CONFIG OSC = INTIO67 ; Set internal oscillator to 1 MHz
    CONFIG WDT = OFF     ; Disable Watchdog Timer
    CONFIG LVP = OFF     ; Disable Low Voltage Programming

    L1 EQU 0x14         ; Define L1 memory location
    L2 EQU 0x15         ; Define L2 memory location
    cnt EQU 0x16
    bulb EQU 0x17
    tmp EQU 0x18
    org 0x00            ; Set program start address to 0x00
MOVLF MACRO literal, f
    MOVLW   literal
    MOVWF   f
ENDM
; instruction frequency = 1 MHz / 4 = 0.25 MHz
; instruction time = 1/0.25 = 4 \mu s
; Total_cycles = 2 + (2 + 8 * num1 + 3) * num2 cycles
; num1 = 111, num2 = 70, Total_cycles = 62512 cycles
; Total_delay ~= Total_cycles * instruction time = 0.25 s
DELAY_NO_LISTEN macro num1, num2
    local LOOP1         ; Inner loop
    local LOOP2         ; Outer loop
    
    ; 2 cycles
    MOVLW num2          ; Load num2 into WREG
    MOVWF L2            ; Store WREG value into L2
    
    ; Total_cycles for LOOP2 = 2 cycles
    LOOP2:
    MOVLW num1          
    MOVWF L1  
    
    ; Total_cycles for LOOP1 = 8 cycles
    LOOP1:
    NOP                 ; busy waiting
    NOP
    NOP
    NOP
    NOP
    DECFSZ L1, 1        
    BRA LOOP1           ; BRA instruction spends 2 cycles
    
    ; 3 cycles
    DECFSZ L2, 1        ; Decrement L2, skip if zero
    BRA LOOP2           
endm
DELAY macro num1, num2
    local LOOP1         ; Inner loop
    local LOOP2         ; Outer loop
    
    ; 2 cycles
    MOVLW num2          ; Load num2 into WREG
    MOVWF L2            ; Store WREG value into L2
    
    ; Total_cycles for LOOP2 = 2 cycles
    LOOP2:
    MOVLW num1          
    MOVWF L1  
    
    ; Total_cycles for LOOP1 = 8 cycles
    LOOP1:
    BTFSS PORTB, 0
    return
    NOP                 ; busy waiting
    NOP
    NOP
    NOP
    NOP
    DECFSZ L1, 1        
    BRA LOOP1           ; BRA instruction spends 2 cycles
    
    ; 3 cycles
    DECFSZ L2, 1        ; Decrement L2, skip if zero
    BRA LOOP2           
endm
start:
int:
; let pin can receive digital signal
    MOVLW 0x00
    MOVWF cnt
    MOVLW 0x0f          ; Set ADCON1 register for digital mode
    MOVWF ADCON1        ; Store WREG value into ADCON1 register
    CLRF PORTB          ; Clear PORTB
    BSF TRISB, 0        ; Set RB0 as input (TRISB = 0000 0001)
    BCF INTCON2, RBPU
    CLRF LATA           ; Clear LATA
    BCF TRISA, 0        ; Set RA0 as output (TRISA = 0000 0000)
    BCF TRISA, 1        ; Set RA1 as output (TRISA = 0000 0000)
    BCF TRISA, 2        ; Set RA2 as output (TRISA = 0000 0000)
    BSF STATUS, C

    GOTO    check_process
    
   _state2_3:
	BSF bulb, 2
	MOVFF bulb, LATA
        DELAY d'111', d'70'
        DELAY d'111', d'70'
        BCF     bulb, 1
        MOVFF   bulb, LATA
        DELAY d'111', d'70'
        DELAY d'111', d'70'
        DELAY d'111', d'70'
        DELAY d'111', d'70'
        BSF     bulb, 1
        MOVFF   bulb, LATA

        DECFSZ  tmp
        GOTO    _state2_3

        DELAY d'111', d'70'
        DELAY d'111', d'70'
        RETURN  
    _state0:
        MOVLF 0x00, bulb
        MOVFF   bulb,LATA
        return;

    _state1:
        MOVLF 0x01, bulb
	MOVFF bulb, LATA
        DELAY d'111', d'70'
        DELAY d'111', d'70'
	
        MOVLF 0x02, bulb
	MOVFF bulb, LATA
        DELAY d'111', d'70'
        DELAY d'111', d'70'

        MOVLF 0x04, bulb
	MOVFF bulb, LATA
        DELAY d'111', d'70'
        DELAY d'111', d'70'

        RETURN  
    _state2:
        ;2_1
        MOVLF 0x01, bulb
        MOVFF    bulb, LATA
        DELAY d'111', d'70'
        DELAY d'111', d'70'

        ;2_2
;        MOVLF 0x03, bulb
	BSF bulb, 1
	BSF bulb, 0
        MOVFF    bulb, LATA
        DELAY d'111', d'70'
        DELAY d'111', d'70'
        DELAY d'111', d'70'
        DELAY d'111', d'70'

        MOVLF 0x02, tmp
        RCALL   _state2_3

        RETURN  

lightup:
    MOVLW   0x03
    CPFSLT  cnt
    BRA go_state0;>=3

    BTFSS   cnt, 0
    GOTO skip_0
    RCALL    _state1
    RETURN
    skip_0:

    BTFSS   cnt, 1
    GOTO skip_1
    
    RCALL    _state2
    RETURN
    
    skip_1:
    

go_state0:
    CLRF    cnt
    RCALL    _state0

    RETURN  
   
; Button check
check_process:          
    RCALL   lightup
    BTFSC PORTB, 0      ; Check if PORTB bit 0 is low (button pressed)
    BRA check_process   ; If button is not pressed, branch back to check_process
    INCF    cnt, f
    ; BRA lightup         ; If button is pressed, branch to lightup

    DELAY_NO_LISTEN d'111', d'70' ; Call delay macro to delay for about 0.25 seconds
    BRA check_process   
NOP
end



 