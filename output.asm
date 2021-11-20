.globl main
.text

main:
lw $t0, x
li $t1, 5
sw $t1, x
lw $t2, x
li $t3, 2
sw $t3, x
lw $t4, x

li $v0, 10
syscall
x: .word 0
