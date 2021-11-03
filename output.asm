.globl main
.text
main:
li $t0, 1
li $t1, 2
add $t0, $t0, $t1
li $t2, 3
add $t0, $t0, $t2
move $v0, $t0
syscall
