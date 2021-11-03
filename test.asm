# A demonstration of some simple MIPS instructions
# used to test QtSPIM

	# Declare main as a global function
	.globl main 

	# All program code is placed after the
	# .text assembler directive
	.text 		

# The label 'main' represents the starting point
main:
	li $t0, 1
    li $t1, 2
    add $t0, $t0, $t1
    li $t2, 3
    add $t0, $t0, $t2
    li $t3, 4
    add $t0, $t0, $t3
    li $v0, 10
	syscall # Exit