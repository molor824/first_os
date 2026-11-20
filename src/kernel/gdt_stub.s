.section .bss
.align 8
gdtr:
    .skip 10

.section .text
.global set_gdt

set_gdt:
    movw %di, gdtr(%rip)
    movq %rsi, gdtr+2(%rip)
    lgdt gdtr(%rip)
    ret

.global reload_segments
reload_segments:
    pushq $0x08
    leaq .reload_CS(%rip), %rax
    pushq %rax
    lretq
.reload_CS:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ret
