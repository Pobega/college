.data
initial_prompt:	.asciiz	"Enter size of array: "
array_prompt:		.asciiz	"Enter an integer: "
output1:				.asciiz	"\nArray["
output2:				.asciiz	"]: "

.text

main:
	# prompt user for array size
	li $v0, 4
	la $a0, initial_prompt
	syscall

	li $v0, 5
	syscall
	move $s0, $v0

	li $t0, 4
	mult $s0, $t0
	mflo $s1

	li $v0, 9
	move $a0, $s1
	syscall
	move $s7, $v0
	move $s6, $s7

	li $t0, 1

get_array:
	li $v0, 4
	la $a0, array_prompt
	syscall

	li $v0, 5
	syscall
	sw $v0, ($s6)

	li $t5, 1
	beq $t0, $s0, bubble_init
	addi $t0, $t0, 1

	addi $s6, $s6, 4

	j get_array

bubble_init:

	li $t7, 0
	li $t0, 1
	blt $t5, $t0, print_array
	li $t5, 0

	move $s6, $s7

bubble_sort:

	beq $t0, $s0, bubble_init
	addi $t0, 1

	move $t2, $s6
	addi $t2, $t2, 4
	lw $t3, ($s6)
	lw $t4, ($t2)
	bge $t3, $t4, switch

	addi $s6, $s6, 4
	j bubble_sort

switch:
	sw $t3, ($t2)
	sw $t4, ($s6)
	li $t5, 1
	j bubble_sort

print_array:
	li $v0, 4
	la $a0, output1
	syscall

	li $v0, 1
	move $a0, $t7
	syscall

	li $v0, 4
	la $a0, output2
	syscall

	li $v0, 1
	lw $t4, ($s7)
	move $a0, $t4
	syscall

	addi $t7, $t7, 1
	beq $t7, $s0, exit
	addi $s7, $s7, 4
	j print_array

exit:
	li $v0, 10
	syscall
