#include "xc.inc"
GLOBAL _mul_extended
PSECT mytext, local, class=CODE, reloc=2
 
_mul_extended:
    MOVFF 0x001, 0x10 ;nL
    MOVFF 0x002, 0x11 ;nH
    MOVFF 0x003, 0x12 ;mL
    MOVFF 0x004, 0X13 ;mH
    ; n * m
    ;; nH(0x11), nL(0x10)
    ;; mH(0x13), mL(0x12)
    ;;out:   Large -->  Small
    ;;out: 0x004 0x003 0x002 0x001
    CLRF 0x001
    CLRF 0x002
    CLRF 0x003
    CLRF 0x004
    nL_mL:
	MOVF 0x10, w
	MULWF 0x12
	
	MOVF PRODL, W
	ADDWF 0x001, 1
	MOVF PRODH, W
	ADDWF 0x002, 1
    nH_mL:
	MOVF 0x11, w
	MULWF 0x12
	
	MOVF PRODL, W
	ADDWF 0x002, 1
	MOVF PRODH, W
	ADDWFC 0x003, 1
    nL_mH:
	MOVF 0x10, w
	MULWF 0x13
	
	MOVF PRODL, W
	ADDWF 0x002, 1
	MOVF PRODH, W
	ADDWFC 0x003, 1
    nH_mH:
	MOVF 0x11, w
	MULWF 0x13
	
	MOVF PRODL, W
	ADDWF 0x003, 1
	MOVF PRODH, W
	ADDWFC 0x004, 1

    STEP2:
	; if op1(nH(0x11), nL(0x10)) is negative, subtrasct op2 from upper byte
	BTFSS 0x11, 7 ;is nH is negative
	GOTO STEP3
	
	;upper byte = 0x04, 0x03
	MOVF 0x12, w
	SUBWF 0x03
	MOVF 0x13, w
	SUBWFB 0x04
	
    STEP3:
	; if op2(mH(0x13), mL(0x12)) is negative, subtrasct op1 from upper byte
	BTFSS 0x13, 7 ;mH is negative
	GOTO finish
	
	MOVF 0x10, w
	SUBWF 0x03
	MOVF 0x11, w
	SUBWFB 0x04
	
    finish:
    
RETURN
