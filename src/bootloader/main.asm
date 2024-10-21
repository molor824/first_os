org 0x7C00
bits 16

; first 3 byte must be jump to start and nop
jmp start
nop

bdb_oem:                   db 'MSWIN4.1' 
bdb_bytes_per_sector:      dw 512
bdb_sectors_per_cluster:   db 1
bdb_reserved_sectors:      dw 1
bdb_fat_count:             db 2
bdb_dir_entries_count:     dw 0xE0
bdb_total_sectors:         dw 2880
bdb_media_descriptor_type: db 0xF0
bdb_sectors_per_fat:       dw 9
bdb_sectors_per_track:     dw 18
bdb_heads:                 dw 2
bdb_hidden_sectors:        dd 0
bdb_large_sector_count:    dd 0

; extended boot record
ebr_drive_number:          db 0
                           db 0 ; reserved
ebr_signature:             db 0x29
ebr_volume_id:             db 0x12, 0x34, 0x56, 0x78 ; serial number, can be ignored
ebr_volume_label:          db 'FIRST OS   ' ; 11 byte
ebr_system_id:             db 'FAT12   '    ; 8 byte

%define ENDL 0x0D, 0x0A

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
    ; setup data segments
    mov ax, 0             ; you cant write to ds/es directly
    mov ds, ax
    mov es, ax
    
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00        ; stack grows downwards

    ; print message
    mov si, msg_hello
    call puts

    cli
    hlt

.halt:
    jmp .halt

msg_hello: db 'Hello World!', ENDL, 0x0

times 510-($-$$) db 0
dw 0xAA55