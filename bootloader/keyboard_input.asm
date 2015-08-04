[BITS 16]	;tells nasm that this is 16-bit; required for bootloaders (!!)
[ORG 0x7C00]	;Origin, tell the assembler that where the code will
					;be in memory after it is been loaded (7c00 is standard)

MOV SI, HelloString ;Store string pointer to SI
CALL ClearScreen	;Clear the screen first
CALL GetKey
CALL PrintString	;Call print string procedure
JMP $ 		;Infinite loop, hang it here.

ClearScreen:
MOV AH,0x00 ;set video mode 
MOV AL,0x03 ;select video function
INT 0x10 	;clear the screen and set video mode
RET

PrintCharacter:	;Procedure to print character on screen
	;Assume that ASCII value is in register AL
MOV AH, 0x0E	;Tell BIOS that we need to print one charater on screen.
MOV BH, 0		;Page no.
MOV BL, 0xA5	;Text attribute 0x07 is lightgrey font on black background

INT 0x10	;Call video interrupt
RET		;Return to calling procedure



PrintString:	;Procedure to print string on screen
	;Assume that string starting pointer is in register SI

next_character:	;Lable to fetch next character from string
MOV AL, [SI]	;Get a byte from string and store in AL register
INC SI		;Increment SI pointer
OR AL, AL	;Check if value in AL is zero (end of string)
JZ exit_function ;If end then return
CALL PrintCharacter ;Else print the character which is in AL register
JMP next_character	;Fetch next character from string
exit_function:	;End label
RET		;Return from procedure

GetKey:
;get input
MOV AH,0x00
INT 0x16		;interrupt for keyboard input; stores input in register AL
;print input
MOV AH,0x0E	;set the value of reg AH to print one character to stdout
MOV BH,0		;set page number
MOV BL,0x1F	;set the color attribute to the font (0x1F is BSOD)
MOV CX,10	;set the value for how many times the character is to be printed
INT 0x10		;video refresh interrupt
CMP AL,0x0D	;cmp key entered (AL) to enter (13d, or 0x0D)
JNE GetKey	;if enter wasn't hit, continue looping.
RET


;Data
HelloString db 'Welcome to our bootloader',10,13,10,13,10,13,'Hit Enter',0

TIMES 510 - ($ - $$) db 0	;Fill the rest of sector with 0
DW 0xAA55			;bootloader signature; required to run!
