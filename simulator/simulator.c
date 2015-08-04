// PC441 Emulator
/*

	Written by Michael Pobega
	CS441 - William Roberts
	Architecture Simulator

	Formatted to be viewed with monospace font and 4-space tabs

*/

#include <stdio.h>
#include <math.h>
#include <signal.h>

typedef unsigned char uint8; // Shorthand for unsigned 8-bit value
typedef unsigned int uint32; // Shorthand for unsigned 32-bit value

#define MEMSIZE 16384 // maximum size of memory

int SIG; // signal variable

void setSignal();
void printBinary(int n);
void printBinLength(int n, int length);

int main(int argc, char *argv[])
{
	printf("Welcome to the PC441 architecture simulator\n"); // Hello, World :)
	uint8 mem[MEMSIZE] = {};	// create zero-initialized RAM
	uint32 r[16] = {}; 			/* create zero-initialized register array
													of 16 four-byte (32-bit) registers */

	SIG = 1;							// initializes the signal at 1, meaning "run"
	signal(SIGINT, setSignal); // tell signal.h which function unsets signal
	
	// Make sure at least one argument was passed
	if(argc <= 1)		
	{
		printf("PC441 simulator requires an input file to run\n");
		return 1;
	}

	printf("Loading binary file: %s\n",argv[1]);
	FILE *file;
	char instruction_byte[4];
	int c = 1, totalMemUsed = 24;
	file = fopen(argv[1],"r");

	if(!file)	// Make sure the file opens successfully
	{
		printf("Failed to open: %s\n",argv[1]);
		return 2;
	}
	r[15] = 28;		// Start PC counter at 28

		/* Reads the file byte by byte, storing the contents in memory.
		   Also makes sure that we are getting a full 32-bit instruction,
			as to not end up with a half-instruction */

	while(fread(&instruction_byte[3],1,1,file) >= 1 
		&& fread(&instruction_byte[2],1,1,file) >= 1
		&& fread(&instruction_byte[1],1,1,file) >= 1
		&& fread(&instruction_byte[0],1,1,file) >= 1)
	{
		mem[r[15]+3] = instruction_byte[0];
		mem[r[15]+2] = instruction_byte[1];
		mem[r[15]+1] = instruction_byte[2];
		mem[r[15]] = instruction_byte[3]; 
		r[15] += 4;
		totalMemUsed = totalMemUsed + 4;
	}

	printf("Begin execution: %s\n",argv[1]);
	uint32 encoding, ir, opcode, tick=0,
					 rA, rB, imm, eff;
	uint8 math_flags,branched;
	uint32 *endofmemory = (uint32*)mem+((MEMSIZE/4)-1);

		/* Loops over each instruction, strips out the needed parts
			into individual variables, then proceeds to interpret
			and run code line by line */
	r[15] = 28;
	while( r[15] < MEMSIZE ) {

		tick++;	// increase clock tick by 1
		branched = 0;

		encoding = *(uint32*)(mem+r[15]);
		ir = encoding >> 31;		// get IR (HO bit)
		opcode = encoding << 1;	// strip out ir
		opcode = opcode >> 27;	// push opcode to LO bits
		rA = encoding << 6;		// strip out ir and opcode
		rA = rA >> 28; 			// push rA into LO bits
		if(ir) {						// if ir is set we are using rB, else imm
			rB = encoding << 10;	// strip out ir, opcode and rA
			rB = rB >> 28; 		// push rB into LO bits
			eff = r[rB];			// set effective to value of register rB
		}
		else {
			imm = encoding << 10;// strip out ir, opcode and rA
			imm = imm >> 10;		// push 22-bit immediate into LO bits
			eff = imm;				// set effective to value of immediate

				/* If we are going to do signed Math or sign comparison,
					we need to mask the unsigned effective value as such */
			if (eff >> 21 == 1 && ((opcode >= 0xD && opcode <= 0x11) || opcode == 0x0))
				eff = eff | 0xffc00000; 
					/* mask with 11111111110000000000000000000000 - ten 1s and twenty-two 0s
						signed imm is only 22-bits by default, this fills it to 32 */
		}
				/* Makes sure that we are not trying to branch to r[15],
					because that would cause an infinite loop */
			if ( rA == 15 && ( opcode >= 0x2 && opcode <= 0x5 )) {
				fprintf(stderr, "ERROR: cannot use r[15] as rA in branch - infinite loop\n");
				fprintf(stderr, " -- Skipping instruction %u.\n",r[15]/4); // Show errored instruction
				opcode = 0xBADC0DE;	// 0xBADC0DE will cause the switch statement to default
			}
				/* Makes sure that we are not trying to write to r[15],
					because r[15] is reserved for the program counter. */
			if ( rA == 15 && ((opcode >= 0x6 && opcode <= 0xB) 
								|| (opcode >= 0xD && opcode <= 0x16))) 
			{
				fprintf(stderr, "ERROR: cannot set value of r[15] - reserved for program counter.\n");
				fprintf(stderr, " -- Skipping instruction %u.\n",r[15]/4); // Show errored instruction
				opcode = 0xBADC0DE;	// 0xBADC0DE will cause the switch statement to default
			}
			if ( opcode == 0x17 && eff > 5 ) { // Interrupts must be between 0-5
				fprintf(stderr, "ERROR: effective outside of Interrupt Vector Table.\n");
				fprintf(stderr, " -- Skipping instruction %u.\n",r[15]/4); // Show errored instruction
				opcode = 0xBADC0DE;	// 0xBADC0DE will cause the switch statement to default
			}
			if ( opcode >= 0x2 && opcode <= 0x5 ) {
				if (!ir) {				/* if we are going to be branching, we need to extend
					eff = eff | rB;		the effective value by the 4 bits used in rB */ 
					if ( (eff % 4) != 0 ) {
						fprintf(stderr, "ERROR: may only set PC to a multiple of 4.\n");
						// Show errored instruction
						fprintf(stderr, " -- Skipping instruction %u.\n",r[15]/4); 
						opcode = 0xBADC0DE;	// 0xBADC0DE will cause the switch statement to default
					}
				} else {
					eff = r[rA];
					if ( (eff % 4) != 0 ) {
						fprintf(stderr, "ERROR: may only set PC to a multiple of 4.\n");
						// Show errored instruction
						fprintf(stderr, " -- Skipping instruction %u.\n",r[15]/4); 
						opcode = 0xBADC0DE;	// 0xBADC0DE will cause the switch statement to default
					}
				}
			}
		switch ( opcode ) {

			/* Needs to be bugtested!!!  */

			case 0x0: // SCMP - compare signed
				printf("cntrl; SCMP %u %u = ",r[rA],eff);
				mem[0] = mem[0] & 0x0;	/* zero the entire first byte of memory
													before continuing with logic */
				if ( (signed int)r[rA] == (signed int)eff )
					mem[0] = mem[0] | 0x1; // set bit 0 [0001]
				else
					mem[0] = mem[0] | 0x2; // set bit 1 [0010]
				if ( (signed int)r[rA] > (signed int)eff )
					mem[0] = mem[0] | 0x4; // set bit 2 [0100]
				if ( (signed int)r[rA] < (signed int)eff )
					mem[0] = mem[0] | 0x8; // set bit 3 [1000]
				printBinary(encoding);
				break;
			case 0x1: // UCMP - compare unsigned
				printf("cntrl; UCMP %u %u = ",r[rA],eff);
				mem[0] = mem[0] & 0x0;	/* zero the entire first byte of memory
													before continuing with logic */
				if ( r[rA] == eff )
					mem[0] = mem[0] | 0x1; // set bit 0 [0001]
				else
					mem[0] = mem[0] | 0x2; // set bit 1 [0010]
				if ( r[rA] > eff )
					mem[0] = mem[0] | 0x4; // set bit 2 [0100]
				if ( r[rA] < eff )
					mem[0] = mem[0] | 0x8; // set bit 3 [1000]
				printBinary(encoding);
				break;
			case 0x2: // BRAE - branch if equal
				printf("cntrl; BRAE ");

				math_flags = 0x0;
				math_flags = math_flags | mem[0];
				math_flags = (math_flags<<7)>>7;

				printBinary(encoding);
				if ( math_flags == 0x1 ) {
					r[15] = eff;
					branched = 1;
				} 
				break;
			case 0x3: // BRANE - branch if not equal
				printf("cntrl; BRANE ");

				math_flags = 0x0;
				math_flags = math_flags | mem[0];
				math_flags = (math_flags<<6)>>7;

				printBinary(encoding);
				if ( math_flags == 0x1 ) {
					r[15] = eff;
					branched = 1;
				}
				break;
			case 0x4: // BRAGT - branch if greater than
				printf("cntrl; BRAGT ");

				math_flags = 0x0;
				math_flags = math_flags | mem[0];
				math_flags = (math_flags<<5)>>7;

				printBinary(encoding);
				if ( math_flags == 0x1 ) {
					r[15] = eff;
					branched = 1;
				} 
				break;
			case 0x5: // BRALT - branch if less than
				printf("cntrl; BRALT ");

				math_flags = 0x0;
				math_flags = math_flags | mem[0];
				math_flags = (math_flags<<4)>>7;

				printBinary(encoding);
				if ( math_flags == 0x1 ) {
					r[15] = eff;
					branched = 1;
				}
				break;
			case 0x6: // LSH - left shift
				printf("r[%u]; LSH %u %u = ",rA,r[rA],eff);
				r[rA] = r[rA] << eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0x7: // RSH - right shift
				printf("r[%u]; RSH %u %u = ",rA,r[rA],eff);
				r[rA] = r[rA] >> eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0x8: // AND - bitwise and
				printf("r[%u]; AND %u %u = ",rA,r[rA],eff);
				r[rA] = r[rA] & eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0x9: // OR - bitwise or
				printf("r[%u]; OR %u %u = ",rA,r[rA],eff);
				r[rA] = r[rA] | eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0xA: // NOT - bitwise not
				printf("r[%u]; NOT %u = ",rA,eff);
				r[rA] = ~eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0xB: // STOR - store from register into memory
				if ( eff > MEMSIZE )
					fprintf(stderr, "STOR: mem[eff] out of scope of memory\n");
				else {
					r[3]=24;
					printf("mem[%u]; STOR r[%u]=%u mem[%u]=%u = ",eff,rA,r[rA],eff,mem[eff]);
					mem[eff] = r[rA];
					printf("%u ",mem[eff]);
					printBinary(encoding);
				}
				break;
			case 0xC: // LOAD - load from memory into register
				if ( eff > MEMSIZE )
					fprintf(stderr, "LOAD: mem[eff] out of scope of memory\n");
				else {
					printf("r[%u]; LOAD r[%u]=%u mem[%u]=%u = ",rA,rA,r[rA],eff,mem[eff]);
					r[rA] = mem[eff];
					printf("%u ",r[rA]);
					printBinary(encoding);
				}
				break;
			case 0xD: // SADD - signed addition
				printf("r[%u]; SADD (%d + %d) = ",rA,r[rA],eff);
				r[rA] = (signed int)r[rA] + (signed int)eff;
				printf("%d ",r[rA]);
				printBinary(encoding);
				break;
			case 0xE: // SSUB - signed subtraction
				printf("r[%u]; SSUB %d %d = ",rA,r[rA],eff);
				r[rA] = (signed int)r[rA] - (signed int)eff;
				printf("%d ",r[rA]);
				printBinary(encoding);
				break;
			case 0xF: // SMUL - signed multiplication
				printf("r[%u]; SMUL %d %d = ",rA,r[rA],eff);
				r[rA] = (signed int)r[rA] * (signed int)eff;
				printf("%d ",r[rA]);
				printBinary(encoding);
				break;
			case 0x10: // SDIV - signed division
				printf("r[%u]; SDIV %d %d = ",rA,r[rA],eff);
				r[rA] = (signed int)r[rA] / (signed int)eff;
				printf("%d ",r[rA]);
				printBinary(encoding);
				break;
			case 0x11: // SEXP - sex party
				printf("r[%u]; SEXP %d %d = ",rA,r[rA],eff);
				r[rA] = (int)pow( (double)r[rA], (double)eff );
				printf("%d ",r[rA]);
				printBinary(encoding);
				break;
			case 0x12: // UADD - unsigned addition
				printf("r[%u]; UADD %u %u = ",rA,r[rA],eff);
				r[rA] = r[rA] + eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0x13: // USUB - unsigned subtraction
				printf("r[%u]; USUB %u %u = ",rA,r[rA],eff);
				r[rA] = r[rA] - eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0x14: // UMUL - unsigned multiplication
				printf("r[%u]; UMUL %u %u = ",rA,r[rA],eff);
				r[rA] = r[rA] * eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0x15: // UDIV - unsigned divison
				printf("r[%u]; UDIV %u %u = ",rA,r[rA],eff);
				r[rA] = r[rA] / eff;
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0x16: // UEXP - unsigned exponent (power)
				printf("r[%u]; UEXP %u %u = ",rA,r[rA],eff);
				r[rA] = (uint32)pow( (double)r[rA], (double)eff );
				printf("%u ",r[rA]);
				printBinary(encoding);
				break;
			case 0x17: // INT - interrupt
				*endofmemory = r[15];					// Backup r[15] to end of mem[]
				r[15] = *((uint32*)mem+(eff*4)+4);	// get PC from IVT at beginning
				branched = 1;								// of memory
				break;
			case 0x18: // IRET - interrupt return
//				r[15] = *((uint32*)mem+(MEMSIZE-1));	// restore PC (r[15])
				r[15] = *endofmemory;	// restore PC (r[15])
				break;
			case 0xBADC0DE:	// BAD CODE - error found above
				break;			// Do nothing.
			default:
				printf("Instruction ");
				printBinLength(opcode,5);
				printf(" not recognized.\n -- Skipping instruction %u\n",r[15]/4);
				break;
		}	

		printf("\ttick  = %u\n", tick);
		int count;
		for ( count=0; count <= 15; count++ ) {
			printf("\tr[%2u] = %11u\t", count, r[count]);
			printBinary(r[count]);
		}

		if ( SIG == 0 )	// break if signal is 0, meaning that user-input has
			break;			// attempted to end the program (Ctrl-C)

		if ( branched != 1 )	// we don't want to iterate r[15]
			r[15] += 4;			// when doing a branch
															
	}
	fclose(file);
	return 0;
}

// Function to print binary of an integer
void printBinary(int n) {
	uint32 i;
	i = 1<<(sizeof(n) * 8 - 1);
	printf("[");
	while (i > 0) {
		if (n & i)
			printf("1");
		else
			printf("0");
		i >>= 1;
	}
	printf("]\n");
}

// Prints binary of an integer from LO bit up to bit number 'length'
void printBinLength(int n, int length) {
	uint32 i;
	i = 1<<length-1;
	while (i > 0) {
		if (n & i)
			printf("1");
		else
			printf("0");
		i >>= 1;
	}
}

// sets the signal to tell the program to exit
void setSignal() {
	SIG = 0;
	return;
}

