.globl main
.text

main:
lw $t0, x
li $t1, 2
li $t2, 5
add $t1, $t1, $t2
sw $t1, x
li $t3, 0

li $v0, 10
syscall
x: .word 0
