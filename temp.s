.section .rodata
str0:
    .string "SYSTEM"
str1:
    .string "SOS"
str2:
    .string "HELLO"

.text
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    leaq str0(%rip), %rdi
    call m2c_print
    movl $15000, %edi
    movl $35000, %esi
    call m2c_add
    movl %eax, -4(%rbp)
    movl -4(%rbp), %edi
    call m2c_print_int
    movl -4(%rbp), %edi
    movl $10, %esi
    call m2c_mul
    movl %eax, -8(%rbp)
    movl -8(%rbp), %edi
    call m2c_print_int
    movl -8(%rbp), %edi
    movl $50000, %esi
    call m2c_sub
    movl %eax, -12(%rbp)
    movl -12(%rbp), %edi
    call m2c_print_int
    movl -12(%rbp), %edi
    movl $5, %esi
    call m2c_div
    movl %eax, -16(%rbp)
    movl -16(%rbp), %edi
    call m2c_print_int
    leaq str1(%rip), %rdi
    call m2c_print
    movl -16(%rbp), %edi
    movl $1, %esi
    call m2c_add
    movl %eax, -20(%rbp)
    movl -20(%rbp), %edi
    call m2c_print_int
    leaq str2(%rip), %rdi
    call m2c_print
    movl $0, %eax
    leave
    ret
