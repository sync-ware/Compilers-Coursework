.globl main
.text

f:
addi $sp,$sp,-4
move $s0, $a0
sw $s0,0($sp)
addi $sp,$sp,-4
move $s1, $a1
sw $s1,0($sp)
addi $sp,$sp,-4
sw $ra,0($sp)
sw $a0, y
sw $a1, a
lw $t0, a
lw $t1, y
add $t0, $t0, $t1
move $v0, $t0
lw $ra,0($sp)
addi $sp,$sp,4
lw $s1,0($sp)
sw $s1, a
addi $sp,$sp,4
lw $s0,0($sp)
sw $s0, y
addi $sp,$sp,4
jr $ra
main:
lw $t2, x
li $t4, 2
move $a0, $t4
li $t5, 3
move $a1, $t5
jal f
sw $v0, x
lw $t6, x
move $v0, $t6
li $v0, 10
syscall
a: .word 0
y: .word 0
x: .word 0
