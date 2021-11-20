.globl main
.text

main:
lw $t0, x
li $t1, 5
sw $t1, x
lw $t2, y
li $t3, 2
sw $t3, y
lw $t4, x
lw $t5, y
add $t4, $t4, $t5

li $v0, 10
syscall
x: .word 0
y: .word 0
