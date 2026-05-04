.section .rodata
fmt_int:
    .string "%d\n"
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
    call puts@PLT
    movl $15000, %eax
    addl $35000, %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %esi
    leaq fmt_int(%rip), %rdi
    xorl %eax, %eax
    call printf@PLT
    movl -4(%rbp), %eax
    movl $10, %ecx
    imull %ecx, %eax
    movl %eax, -8(%rbp)
    movl -8(%rbp), %esi
    leaq fmt_int(%rip), %rdi
    xorl %eax, %eax
    call printf@PLT
    movl -8(%rbp), %eax
    subl $50000, %eax
    movl %eax, -12(%rbp)
    movl -12(%rbp), %esi
    leaq fmt_int(%rip), %rdi
    xorl %eax, %eax
    call printf@PLT
    movl -12(%rbp), %eax
    cltd
    movl $5, %ecx
    idivl %ecx
    movl %eax, -16(%rbp)
    movl -16(%rbp), %esi
    leaq fmt_int(%rip), %rdi
    xorl %eax, %eax
    call printf@PLT
    leaq str1(%rip), %rdi
    call puts@PLT
    movl -16(%rbp), %eax
    addl $1, %eax
    movl %eax, -20(%rbp)
    movl -20(%rbp), %esi
    leaq fmt_int(%rip), %rdi
    xorl %eax, %eax
    call printf@PLT
    leaq str2(%rip), %rdi
    call puts@PLT
    movl $0, %eax
    leave
    ret
