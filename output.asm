.globl main
.text

RuûUmain:
lw $t0, x
li $t1, 1
add $t1, $t1, $t1
sw $t1, x
lw $t3, y
li $t4, 1
add $t4, $t4, $t4
sw $t4, y
lw $t6, x

li $v0, 10
syscall
x: .word 0
y: .word 0
