org 0x7C00
bits 16

; first 3 byte must be jump to start and nop
jmp short start
nop

bdb_oem:                   db 'MSWIN4.1' 
bdb_bytes_per_sector:      dw 512
bdb_sectors_per_cluster:   db 2
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

; Converts LBA (Logical Block Address) to CHS address (Cylinder Head Sector)
; Params
;  ax: LBA
; Returns
;  cx [bits 0-5]: sector number
;  cx [bits 6-15]: cylinder
;  dh: head
lba_to_chs:
    push ax
    push dx

    xor dx, dx                            ; dx = 0
    div word [bdb_sectors_per_track]  ; ax = LBA / SectorsPerTrack
                                      ; dx = LBA % SectorsPerTrack
    inc dx                            ; dx = (LBA % SectorsPerTrack) + 1 = sector
    mov cx, dx                        ; cx = sector

    xor dx, dx                        ; dx = 0
    div word [bdb_heads]              ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                      ; dx = (LBA / SectorsPerTrack) % Heads = head
    mov dh, dl                        ; dh = head
    mov ch, al                        ; ch = cylinder (lower 8 bits)
    shl ah, 6                         ; ah = cylinder (higher 2 bits)
    or cl, ah                         ; cl = cylinder (higher 2 bits)

    pop ax       ; restore dx
    mov dl, al   ; restore only dl
    pop ax       ; restore ax
    ret

; Reads sectors from a disk
; Params:
;  ax: LBA address
;  cl: number of sectors to read (up to 128)
;  dl: drive number
;  es:bx: memory address to store the data
disk_read:
    push ax
    push bx
    push cx
    push dx
    push di

    push cx              ; temporarily save cx (number of sectors to read)
    call lba_to_chs      ; call absorbs cx
    pop ax               ; restore sectors to ax

    mov ah, 0x2

    mov di, 3            ; retry count (since HDD has tendency to fail)

.loop:
    pusha                ; save all the registers
    stc                  ; set carry flag, some BIOS dont set it
    int 0x13             ; carry flag clear = success
    jnc .done

    ; read failed
    popa
    call disk_reset

    dec di               ; di--
    test di, di          ; applies OR without writing to the register, instead setting a flags
    jnz .loop

.fail:
    ; all the attempts failed
    jmp floppy_error

.done:
    popa

    int 0x13

    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; Resets disk controller
; Params
;  dl: drive number
disk_reset:
    pusha
    mov ah, 0
    stc
    int 0x13
    jc floppy_error
    popa
    ret

floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 0x16              ; waits for keypress
    jmp 0xFFFF:0          ; should reboot

    cli                   ; disable interrupts and halt
    hlt

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

    ; BIOS should set DL to drive number
    mov [ebr_drive_number], dl

    mov ax, 0             ; LBA=0 first sector from disk
    mov cl, 1             ; 1 sector to read
    mov bx, 0x7E00        ; data should be after bootloader
    call disk_read

    ; print message from where we copied from hard drive
    mov si, 0x7EDE
    call puts

    cli                   ; disable interrupt and halt
    hlt

msg_hello_disk: db 'Disk says '
msg_hello: db 'Hello World!', CRLF, 0x0
msg_read_failed: db 'Failed to read from disk!', CRLF, 0x0

times 510-($-$$) db 0
dw 0xAA55