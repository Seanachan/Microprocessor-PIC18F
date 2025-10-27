List p=18f4520 
    #include<p18f4520.inc>
    CONFIG OSC = INTIO67
    CONFIG WDT = OFF
    org 0x00
;    cblock 0x20
;	xh
;	xl
;	yh
;	yl
;    endc
;    MOVLW 0xAB
;    movwf xh
;    movlw 0xCD
;    movwf xl
;    movlw 0x56
;    movwf yh
;    movlw 0x78
;    movwf yl
    
    And_Mul macro xh, xl, yh, yl
	movlw xh
	movwf 0x02
	movlw xl
	movwf 0x03
	movlw yh
	movwf 0x04
	movlw yl
	movwf 0x05
    
    and:
	MOVF 0x02, W
	ANDWF 0x04, w
	MOVWF 0x00
	
	MOVF 0x03, W
	ANDWF 0x05, w
	MOVWF 0x01
	
    mul:
	MOVF 0x00, w
	MULWF 0x01
	
    store:
	MOVFF PRODH, 0x10
	MOVFF PRODL, 0x11
    endm
    
    And_Mul 0x50, 0x6F, 0x3A, 0xBC
    
    NOP
    end                   
