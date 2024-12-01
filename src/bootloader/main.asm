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

main:
    ; setup data segments
    mov ax, 0             ; you cant write to ds/es directly
    mov ds, ax
    mov es, ax
    
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00        ; stack grows downwards

    ; set drive number
    mov [ebr_drive_number], dl

    ; print message
    mov si, msg_loading
    call puts

    ; read drive parameters
    push es
    mov ah, 0x8
    int 0x13
    jc floppy_error
    pop es

    and cl, 0x3F    ; remove top 2 bits
    xor ch, ch
    mov [bdb_sectors_per_track], cx  ; sector count

    inc dh
    mov [bdb_heads], dh              ; head count

    ; read FAT root directory
    mov ax, [bdb_sectors_per_fat]    ; lba = sectors_per_fat * fat_count + reserved_sectors
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx                           ; ax0 = sectors_per_fat * fat_count
    add ax, [bdb_reserved_sectors]   ; lba = ax1 = ax0 + reserved_sectors
    push ax                          ; store lba

    ; compute size of root directory = ax = (32 * number_of_entries) / bytes_per_sector
    mov ax, [bdb_sectors_per_fat]    ; ax = sectors_per_fat
    shl ax, 5                        ; ax *= 32
    xor dx, dx                       ; dx = 0
    div word [bdb_bytes_per_sector]  ; dx = ax % bytes_per_sector; ax /= bytes_per_sector;

    test dx, dx                      ; if dx != 0
    jz .skip_inc
    inc ax                           ; ax++

.skip_inc:
    ; read root directory
    ; sectors to read = size of root directory
    mov cl, al                  ; cl = ax = size of root directory
    pop ax                      ; retrieve lba
    mov dl, [ebr_drive_number]  ; dl = drive number
    mov bx, buffer              ; es:bx = buffer
    call disk_read

    ; search for kernel.bin
    xor bx, bx                  ; how many entries checked
    mov di, buffer              ; start searching from buffer, name field has offset of 0

.search_kernel:
    mov si, file_kernel_bin     ; kernel file name string pointer
    mov cx, 11                  ; length is always 11
    push di                     ; save buffer on memory
    repe cmpsb
    pop di
    je .found_kernel

    add di, 32                  ; jump to the next entry
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_kernel

    ; kernel not found
    jmp kernel_not_found_error

.found_kernel:
    ; di has address to the entry
    mov ax, [di + 26]          ; first logical cluster (offset 26)
    mov [kernel_cluster], ax   ; store it globally

    ; load FAT disk onto memory
    mov ax, [bdb_reserved_sectors]
    mov bx, buffer
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call disk_read

    ; read kernel and process FAT chain
    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx
    mov bx, KERNEL_LOAD_OFFSET

.load_kernel_loop:
    ; read next cluster
    mov ax, [kernel_cluster]
    add ax, 31                   ; first cluster = (cluster_number - 2) * sectors_per_cluster + kernel_cluster
                                 ; TODO: HACKY SOLUTION, FIX IN THE FUTURE!!!

    mov cl, 1                    ; read 1 sector
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, [bdb_bytes_per_sector]

    ; compute location of next cluster
    mov ax, [kernel_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx                       ; ax = index of entry in FAT, dx = cluster mod 2

    mov si, buffer
    add si, ax
    mov ax, [ds:si]              ; read entry from FAT table at index ax

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster_after

.even:
    and ax, 0xFFF

.next_cluster_after:
    cmp ax, 0xFF8                ; check if ax is below FF8 or not
    jae .read_finish

    mov [kernel_cluster], ax     ; load next cluster
    jmp .load_kernel_loop

.read_finish:
    ; jump to kernel
    mov dl, [ebr_drive_number]
    
    mov ax, KERNEL_LOAD_SEGMENT  ; kernel segment
    mov ds, ax
    mov es, ax

    jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET ; jmp far

    jmp wait_key_and_reboot                    ; should never happen

    ; print message from where we copied from hard drive
    mov si, 0x7EDE
    call puts

    cli                   ; disable interrupt and halt
    hlt

; error handles
floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

kernel_not_found_error:
    mov si, msg_kernel_not_found
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 0x16              ; waits for keypress
    jmp 0xFFFF:0          ; should reboot

    cli                   ; disable interrupts and halt
    hlt

msg_loading: db 'Loading...', CRLF, 0x0
msg_read_failed: db 'Failed to read from disk!', CRLF, 0x0
msg_kernel_not_found: db 'Kernel not found!', CRLF, 0x0
file_kernel_bin: db 'KERNEL  BIN'
kernel_cluster: dw 0

KERNEL_LOAD_SEGMENT equ 0x2000
KERNEL_LOAD_OFFSET  equ 0x0

times 510-($-$$) db 0
dw 0xAA55

buffer: