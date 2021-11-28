.globl main
.text

main:
lw $t0, x
li $t1, 1
sw $t1, x
lw $t2, y
li $t3, 1
sw $t3, y
lw $t4, x
lw $t5, y
bne $t4, $t5, L1
li $t7, 1
j L2
L1:
li $t6, 0
L2:

li $v0, 10
syscall
x: .word 0
y: .word 0
