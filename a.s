    .globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $28, %rsp
    movl $2, %edx
    jmp .L1852871876
.L1852871888:
    movl $3, -4(%rbp)
    movl -8(%rbp), %r10d
    movl %r10d, -12(%rbp)
    movl -8(%rbp), %r10d
    movl %r10d, -8(%rbp)
    addl $1, -8(%rbp)
    movl -4(%rbp), %r10d
    cmpl %r10d, -8(%rbp)
    movl $0, -16(%rbp)
    setg -16(%rbp)
    cmpl $0, -16(%rbp)
    je .L0
    jmp .L1852871957
    movl -8(%rbp), %eax
    movq %rbp, %rsp
    popq %rbp
    ret
    movl -4(%rbp), %r10d
    movl %r10d, -20(%rbp)
    movl -4(%rbp), %r10d
    movl %r10d, -4(%rbp)
    addl $1, -4(%rbp)
    movl -8(%rbp), %r10d
    movl %r10d, -8(%rbp)
    addl $1, -8(%rbp)
    movl -8(%rbp), %r10d
    movl %r10d, -24(%rbp)
.L0:
    movl -8(%rbp), %eax
    movq %rbp, %rsp
    popq %rbp
    ret

.section .note.GNU-stack,"",@progbits
