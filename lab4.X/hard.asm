List p=18f4520 
    #include<p18f4520.inc>
    CONFIG OSC = INTIO67
    CONFIG WDT = OFF
    org 0x00

    input macro Nh, Nl, x0h, x0l
	movlw Nh
	movwf 0x20
	movlw Nl
	movwf 0x21
	movlw x0h
	movwf 0x22
	movlw x0l
	movwf 0x23
    endm
    
initial:
	input 0x30, 0x21, 0x26, 0x5D
	MOVFF 0x22, 0x24
	MOVFF 0x23, 0x25
	RCALL newtonSqrt
	GOTO finish
    
	;;N_high=0x20, N_low=0x21
	;;tmp_xn_high = 0x34, tmp_xn_low = 0x35
	;;tmp_x{n+1}_high = 0x24, tmp_x{n+1}_low = 0x25
	
newtonSqrt:
    ;stores sqrt(N)H in 0x24, sqrt(N)L in 0x25
    
    MOVFF 0x24, 0x34
    MOVFF 0x25, 0x35
    div_init:
	CLRF 0x10
	CLRF 0x11
	MOVFF 0x20, 0x12
	MOVFF 0x21, 0x13 
	;0x20(NH),0x21(NL) / 0x34(tmp_xn_H), 0x35(tmp_xn_L)
	;0x10, 0x11 : quotient
	;0x12, 0x13 : remainder
	RCALL division
	GOTO div_fin
    division:
	MOVF 0x35, w	
	SUBWF 0x13
	
	MOVF 0x34,w 
	SUBWFB 0x12
	
	BTFSS STATUS, C ;no borrow
	RETURN;with borrow
	
	INCF 0x11
	BTFSC STATUS, OV ;not exceed 
	INCF 0x00;exceed
	
	GOTO division
    
    div_fin:
    
    ;0x10, 0x11 : N/x_n
    ;0x24, 0x25 : x_n+(N/X_n)
    MOVFF 0x10, 0x24
    MOVFF 0x11, 025
    
    MOVF 0x35, w
    ADDWF 0x25
    
    MOVF 0x34, w
    ADDWFC 0x24
    
    BCF STATUS, C
    RRCF 0x24
    RRCF 0x25
    
    compare:
	MOVF 0x24, w
	CPFSEQ 0x34 ;x_{n}==x_{n+1}
	GOTO newtonSqrt ;x_n!=x_{n+1}

	MOVF 0x25, w
	CPFSEQ 0x35 ;x_{n}==x_{n+1}
	GOTO newtonSqrt ;x_n!=x_{n+1}
    
    RETURN
    
finish:
    NOP
    end                   



