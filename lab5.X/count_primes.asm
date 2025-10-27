#include "xc.inc"
GLOBAL _count_primes
    
; cblock 0x20
;	sL -> 20 ;H->21
;	eL-> 22;H->23
;	idxL -> 24 ; idxH -> 25
;	cntL -> 26 ; cntH -> 27
;    endc
PSECT code_count, local, class=CODE, reloc=2
 
_count_primes:
    
    init:
	MOVFF 0x001, 0x20
	MOVFF 0x002 , 0x21
	MOVFF 0x003, 0x22
	MOVFF 0x004, 0x23
	
	MOVFF 0x001, 0x24;start from sL
	MOVFF 0x002, 0x25 ;sH
	
	incf 0x22
	BTFSC STATUS, 0
	INCF 0x23
    big_loop:
	movf 0x24, w
	CPFSEQ 0x22
	goto idx_not_reached
	
	movf 0x25, w
	CPFSEQ 0x23
	goto idx_not_reached
	
	goto finish	
	idx_not_reached:


	MOVFF 0x24, LATA ;original value Low
	MOVFF 0x25, LATB ;original value High

	MOVLW 0x01
	MOVWF LATC ;INCREASING Low
 	CLRF LATD;LATD idx_High

	RCALL loop
	GOTO big_loop
	loop:
	    MOVLW 0x00
	    CPFSEQ 0x25
	    GOTO skip
	    
	    MOVLW 0x01
	    CPFSEQ 0x24
	    GOTO skip
	    
	    GOTO not_prime
	    skip:
    
	    INCF LATC
	    BTFSC STATUS, 0
	    INCF LATD
	    
	   MOVF LATC, W
	    CPFSEQ LATA
	    BRA idx_end
	    
	    
	   MOVF LATD, W
	    CPFSEQ LATB 
	    BRA idx_end
	    
	    
	    GOTO prime
	    idx_end:

	    MOVFF LATA, 0x30;rem_L
	    MOVFF LATB, 0x31 ;rem_H
	    ;rem: 0x03
	    BCF STATUS, 0;carryyyy
	    BCF STATUS, 2 ;zero
	    divide:
    
;		BCF STATUS, 0
		
		MOVF LATC, w
		SUBWF 0x30, 1
		MOVF LATD, w
		SUBWFB 0x31, 1
		
		MOVLW 0x00
		CPFSEQ 0x30
		GOTO not_equal
		
		CPFSEQ 0x31
		GOTO not_equal
		
		GOTO not_prime
		
		not_equal:
		BN loop
		GOTO divide

	    not_prime:
		MOVLW 001
		ADDWF 0x24
		BTFSC STATUS, 0
		INCF 0x25
		RETURN
	    prime:
		MOVLW 001
		ADDWF 0x24
		BTFSC STATUS, 0
		INCF 0x25
		
		BCF STATUS, 0
		ADDWF 0x26
		BTFSC STATUS, 0
		INCF 0x27
		RETURN
    
    finish:
	movff 0x26, 0x01
	MOVFF 0x27, 0x02
	NOP
   
RETURN
