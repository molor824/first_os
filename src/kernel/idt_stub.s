.extern exception_handler
.extern irq_handler

.macro isr_stub_wrapper index, callback
isr_stub_\index:
    pushal
    pushl $\index
    call \callback
    add $4, %esp
    popal
    iret
.endm

isr_stub_wrapper 0, exception_handler
isr_stub_wrapper 1, exception_handler
isr_stub_wrapper 2, exception_handler
isr_stub_wrapper 3, exception_handler
isr_stub_wrapper 4, exception_handler
isr_stub_wrapper 5, exception_handler
isr_stub_wrapper 6, exception_handler
isr_stub_wrapper 7, exception_handler
isr_stub_wrapper 8, exception_handler
isr_stub_wrapper 9, exception_handler
isr_stub_wrapper 10, exception_handler
isr_stub_wrapper 11, exception_handler
isr_stub_wrapper 12, exception_handler
isr_stub_wrapper 13, exception_handler
isr_stub_wrapper 14, exception_handler
isr_stub_wrapper 15, exception_handler
isr_stub_wrapper 16, exception_handler
isr_stub_wrapper 17, exception_handler
isr_stub_wrapper 18, exception_handler
isr_stub_wrapper 19, exception_handler
isr_stub_wrapper 20, exception_handler
isr_stub_wrapper 21, exception_handler
isr_stub_wrapper 22, exception_handler
isr_stub_wrapper 23, exception_handler
isr_stub_wrapper 24, exception_handler
isr_stub_wrapper 25, exception_handler
isr_stub_wrapper 26, exception_handler
isr_stub_wrapper 27, exception_handler
isr_stub_wrapper 28, exception_handler
isr_stub_wrapper 29, exception_handler
isr_stub_wrapper 30, exception_handler
isr_stub_wrapper 31, exception_handler
isr_stub_wrapper 32, irq_handler
isr_stub_wrapper 33, irq_handler
isr_stub_wrapper 34, irq_handler
isr_stub_wrapper 35, irq_handler
isr_stub_wrapper 36, irq_handler
isr_stub_wrapper 37, irq_handler
isr_stub_wrapper 38, irq_handler
isr_stub_wrapper 39, irq_handler
isr_stub_wrapper 40, irq_handler
isr_stub_wrapper 41, irq_handler
isr_stub_wrapper 42, irq_handler
isr_stub_wrapper 43, irq_handler
isr_stub_wrapper 44, irq_handler
isr_stub_wrapper 45, irq_handler
isr_stub_wrapper 46, irq_handler
isr_stub_wrapper 47, irq_handler

.global isr_stub_table
.global isr_stub_table_end
isr_stub_table:
    .long isr_stub_0
    .long isr_stub_1
    .long isr_stub_2
    .long isr_stub_3
    .long isr_stub_4
    .long isr_stub_5
    .long isr_stub_6
    .long isr_stub_7
    .long isr_stub_8
    .long isr_stub_9
    .long isr_stub_10
    .long isr_stub_11
    .long isr_stub_12
    .long isr_stub_13
    .long isr_stub_14
    .long isr_stub_15
    .long isr_stub_16
    .long isr_stub_17
    .long isr_stub_18
    .long isr_stub_19
    .long isr_stub_20
    .long isr_stub_21
    .long isr_stub_22
    .long isr_stub_23
    .long isr_stub_24
    .long isr_stub_25
    .long isr_stub_26
    .long isr_stub_27
    .long isr_stub_28
    .long isr_stub_29
    .long isr_stub_30
    .long isr_stub_31
    .long isr_stub_32
    .long isr_stub_33
    .long isr_stub_34
    .long isr_stub_35
    .long isr_stub_36
    .long isr_stub_37
    .long isr_stub_38
    .long isr_stub_39
    .long isr_stub_40
    .long isr_stub_41
    .long isr_stub_42
    .long isr_stub_43
    .long isr_stub_44
    .long isr_stub_45
    .long isr_stub_46
    .long isr_stub_47

isr_stub_table_end:
