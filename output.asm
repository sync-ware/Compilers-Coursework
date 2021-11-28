.globl main
.text

f:
lw $t0, z
lw $t1, x
lw $t2, y
add $t1, $t1, $t2
sw $t1, z
lw $t3, z

li $v0, 10
syscall
z: .word 0
x: .word 0
y: .word 0
