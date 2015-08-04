[BITS 16]	;tells nasm that this is 16-bit; required for bootloaders (!!)
[ORG 0x7C00]	;Origin, tell the assembler that where the code will
					;be in memory after it is been loaded (7c00 is standard)

CALL ClearScreen	;Clear the screen first
JMP $ 		;Infinite loop, hang it here.

ClearScreen:
MOV AH,0x00 ;set int10h to set video mode 
MOV AL,0x03 ;select video mode
INT 0x10 ;clear the screen and set video mode
RET

TIMES 510 - ($ - $$) db 0	;Fill the rest of sector with 0
DW 0xAA55			;bootloader signature; required to run!
