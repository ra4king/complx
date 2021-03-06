;@plugin filename=lc3_colorlcd width=8 height=8 initaddr=x4000 startaddr=x6000
;@plugin filename=lc3_random address=xFEED
;@plugin filename=lc3_udiv vector=x30
;@plugin filename=lc3_multiply

; A simple game made for the lc3 showing off many plugins
; A player that is controlled by the keyboard and food
; movement is controlled by WSAD.

.orig x180
	.fill KEYBOARDINTHANDLER
.end

.orig x3000
	LD R6, STACK
	; initialize dat display
	LD R3, DISPLAYINITVALUE
	STI R3, DISPLAYINITADDR
	
	; display starting address
	LD R4, DISPLAYSTART

	JSR INIT
	JSR REFRESH

	; set up interrupts
	LD R3, KBSRIE
	STI R3, KBSR

	LOOP BR LOOP
HALT

INIT
	ADD R6, R6, -1
	STR R7, R6, 0

	; Player is at (0, 0) (R0, R1)
	AND R0, R0, 0
	ST R0, PLAYERX
	ST R0, PLAYERY

	JSR GENFOOD	

	LDR R7, R6, 0
	ADD R6, R6, 1
	RET

GENFOOD
	ADD R6, R6, -1
	STR R7, R6, 0
	; Food is at (1-7, 1-7) (R2, R3)
	LDI R0, RANDOMADDR
	AND R1, R1, 0
	ADD R1, R1, 7
	UDIV
	ADD R0, R1, 7
	AND R1, R1, 0
	ADD R1, R1, 7
	UDIV
	ADD R0, R1, 1
	ST R0, FOODX

	LDI R0, RANDOMADDR
	AND R1, R1, 0
	ADD R1, R1, 7
	UDIV
	ADD R0, R1, 7
	AND R1, R1, 0
	ADD R1, R1, 7
	UDIV
	ADD R0, R1, 1
	ST R0, FOODY
	LDR R7, R6, 0
	ADD R6, R6, 1
	RET

REFRESH
	LD R2, SCREENSIZE
	AND R0, R0, 0
CLEARLOOP
	ADD R1, R4, R2
	STR R0, R1, 0
	ADD R2, R2, -1
	BRZP CLEARLOOP

	ADD R6, R6, -1
	STR R7, R6, 0

	LD R0, FOODY
	AND R1, R1, 0
	ADD R1, R1, 8
	MUL R0, R0, R1
	LD R1, FOODX
	ADD R0, R0, R1

	ADD R1, R0, R4
	LD R0, FOODCOLOR
	STR R0, R1, 0 ; Set Food

	LD R0, PLAYERY
	AND R1, R1, 0
	ADD R1, R1, 8
	MUL R0, R0, R1
	LD R1, PLAYERX
	ADD R0, R0, R1

	ADD R1, R0, R4
	LD R0, PLAYERCOLOR
	STR R0, R1, 0 ; Set Player



	LDR R7, R6, 0
	ADD R6, R6, 1	
	RET

KEYBOARDINTHANDLER
	LDI R0, KBDR ; Get the key
	ADD R0, R0, -16
	ADD R0, R0, -16
	ADD R0, R0, -16
	ADD R0, R0, -16
	ADD R0, R0, -16
	ADD R0, R0, -16
	ADD R0, R0, -1 ; 'a'
	BRZ HANDLE_LEFT
	ADD R0, R0, -3 ; 'd'
	BRZ HANDLE_RIGHT
	ADD R0, R0, -15; 's'
	BRZ HANDLE_DOWN
	ADD R0, R0, -4; 'w'
	BRZ HANDLE_UP
END_HANDLING 
	LD R0, PLAYERX
	LD R1, FOODX
	NOT R1, R1
	ADD R0, R1, R0
	ADD R0, R0, 1
	BRNP DONE_CHECK
	LD R0, PLAYERY
	LD R1, FOODY
	NOT R1, R1
	ADD R0, R1, R0
	ADD R0, R0, 1
	BRNP DONE_CHECK
	JSR GENFOOD
DONE_CHECK
	JSR REFRESH
	RTI

HANDLE_LEFT
	LD R0, PLAYERX
	ADD R0, R0, -1
	BRZP CONT_LEFT
	ADD R0, R0, 1
CONT_LEFT
	ST R0, PLAYERX
	BR END_HANDLING

HANDLE_RIGHT
	LD R0, PLAYERX
	ADD R0, R0, 1
	ADD R5, R0, -8
	BRN CONT_LEFT
	ADD R0, R0, -1
CONT_RIGHT
	ST R0, PLAYERX
	BR END_HANDLING

HANDLE_UP
	LD R0, PLAYERY
	ADD R0, R0, -1
	BRZP CONT_UP
	ADD R0, R0, 1
CONT_UP
	ST R0, PLAYERY
	BR END_HANDLING

HANDLE_DOWN
	LD R0, PLAYERY
	ADD R0, R0, 1
	ADD R5, R0, -8
	BRN CONT_DOWN
	ADD R0, R0, -1
CONT_DOWN
	ST R0, PLAYERY
	BR END_HANDLING

DISPLAYINITADDR .fill x4000
DISPLAYINITVALUE .fill x8000
RANDOMADDR .fill xFEED
DISPLAYSTART .fill x6000
SCREENSIZE .fill 64
COLORMAX .fill x7FFF
PLAYERX .fill 0
PLAYERY .fill 0
PLAYERCOLOR .fill x7C00
FOODX .fill 0
FOODY .fill 0
FOODCOLOR .fill x03E0
KBSRIE .fill x4000
KBSR .fill xFE00
KBDR .fill xFE02
STACK .fill xF000

.end
