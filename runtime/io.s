.section .rodata
fmt_int:
    .string "%d\n"

.text
.globl m2c_print
m2c_print:
    pushq %rbp
    movq %rsp, %rbp
    call puts@PLT
    popq %rbp
    ret

.globl m2c_print_int
m2c_print_int:
    pushq %rbp
    movq %rsp, %rbp
    movl %edi, %esi
    leaq fmt_int(%rip), %rdi
    xorl %eax, %eax
    call printf@PLT
    popq %rbp
    ret
