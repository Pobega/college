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
MOV BH, 0x00	;Page no.
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
mov ah,00h
int 16h	;interrupt for keyboard input; stores input in register AL
;print input
mov ah,9h
mov bh,0
mov bl,5h
mov cx,10
int 10h
CALL GetKey

; Found online (http://www.daniweb.com/forums/thread333365.html)
; Shows how to move the pointer; may be useful for entering boot options later.
;.cursor_pointer2:
;	mov ah,2h		;set the value to "ah" to move the cursor pointer
;	mov bh,0		;select page 
;	mov dh,23		;set row position of the cursor
;	mov dl,0		;set column position of the cursor
;	int 10h			;tell bios "hey dude now we are done,take action!!!"
;	jmp .character_read

;Data
HelloString db 'Welcome to our bootloader',10,13,10,13,10,13,'Hit Enter',0

TIMES 510 - ($ - $$) db 0	;Fill the rest of sector with 0
DW 0xAA55			;bootloader signature; required to run!
