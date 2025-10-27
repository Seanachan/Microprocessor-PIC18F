#include "xc.inc"
GLOBAL _is_prime
PSECT mytext, local, class=CODE, reloc=2
 
 
_is_prime:
    MOVWF LATD ;original value
    
    MOVLW 0x01
    MOVWF 0x24 ;INCREASING INDEX
    
    RCALL loop
    GOTO finish
    loop:
	INCF 0x24
	
	MOVF LATD, w ;increasing value
	CPFSEQ 0x24 ;if 0x01 < 0x02
	GOTO skip1
	
	GOTO prime; >=
	skip1:
	
	MOVFF LATD, 0x20 ;remainder
	;rem: 0x03

	divide:
	    BCF STATUS, 4
	    MOVF 0x24, w
	    SUBWF 0x20, 1
    
	    MOVLW 0x00
	    CPFSEQ 0x20 ;if borrow(C=0), rem<0x02
	    GOto skip
	    
	    GOTO not_prime
	    skip:
    
	    movlw 0x24
	    CPFSLT 0x20 ;0x20<0x24
	    goto skip2 ;nothing happen
	    
	    GOTO loop
	    
	    skip2:
	    GOTO divide
    
	not_prime:
	    MOVLW 0xFF
	    MOVWF 0x001
	    RETURN
	prime:
	    MOVLW 0x01
	    MOVWF 0x001
	    RETURN
    
    finish:
	NOP
   
RETURN
