.globl main
.text

main:
lw $t0, x
li $t1, 1
li $t2, 1
li $t3, 2
div $t2, $t3
mflo $t2
add $t1, $t1, $t2
sw $t1, x
lw $t4, x
li $t5, 1
beq $t4, $t5, L1
li $t7, 0
j L2
L1:
li $t6, 1
L2:

li $v0, 10
syscall
x: .word 0
