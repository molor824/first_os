org 0x0
bits 16

%define CRLF 0x0D, 0x0A

start:
    jmp main

; prints a string to screen
; params:
; - ds:si 
puts:
    ; save registers we will modify
    push si
    push ax

.loop:
    lodsb                  ; load next character in al
    or al, al              ; test for zero
    jz .done

    mov ah, 0x0E           ; call bios interrupt
    int 0x10

    jmp .loop

.done:
    pop ax
    pop si
    ret

main:
    ; for now we use same stack as the bootloader

    ; print message
    mov si, msg_hello
    call puts

.halt:
    cli
    hlt

msg_hello: db 'Hello World! from kernel', CRLF, 0x0
