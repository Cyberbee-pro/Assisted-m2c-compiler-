.text
.globl m2c_add
m2c_add:
    movl %edi, %eax
    addl %esi, %eax
    ret

.globl m2c_sub
m2c_sub:
    movl %edi, %eax
    subl %esi, %eax
    ret

.globl m2c_mul
m2c_mul:
    movl %edi, %eax
    imull %esi, %eax
    ret

.globl m2c_div
m2c_div:
    testl %esi, %esi
    je .Ldiv_zero
    movl %edi, %eax
    cltd
    idivl %esi
    ret

.Ldiv_zero:
    xorl %eax, %eax
    ret
