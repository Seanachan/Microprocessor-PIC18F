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
    input macro xh, xl, yh, yl
	movlw xh
	movwf 0x00
	movlw xl
	movwf 0x01
	movlw yh
	movwf 0x02
	movlw yl
	movwf 0x03
    endm
    
    initial:
	input 0xFA, 0x9F, 0x03, 0x45
	MOVFF 0x00, 0x12
	MOVFF 0x01, 0x13 
	;0x00,0x01 / 0x02, 0x03
	;0x10, 0x11 : quotient
	RCALL division
	GOTO finish
    division:
	MOVF 0x03, w	
	SUBWF 0x13
	
	MOVF 0x02,w 
	SUBWFB 0x12
	
	BTFSS STATUS, C ;no borrow
	RETURN;with borrow
	
	INCF 0x11
	BTFSC STATUS, OV ;not exceed 
	INCF 0x00;exceed
	
	GOTO division
    
    finish:
	movf 0x03, w
	addwf 0x13
	movf 0x02, w
	addwfc 0x12
    
    NOP
    end                   
