.section .init
    // GCC will put the rest

    popl %ebp
    ret

.section .fini
    // GCC will put the rest

    popl %ebp
    ret
