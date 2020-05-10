org 0x7c00
jmp start
times 3-($-$$) db 0x90

bpbOEM:                 DB "DIY.OS  "
bpbBytesPerSector:      DW 512
bpbSectorsPerCluster:   DB 1
bpbReservedSectors:     DW 1
bpbNumberOfFATs:        DB 2
bpbRootEntries:         DW 224
bpbTotalSectors:        DW 2880
bpbMedia:               DB 0xF0
bpbSectorsPerFAT:       DW 9
bpbSectorsPerTrack:     DW 18
bpbHeadsPerCylinder:    DW 2
bpbHiddenSectors:       DD 0
bpbTotalSectorsBig:     DD 0
bsDriveNumber:          DB 0
bsUnused:               DB 0
bsExtBootSignature:     DB 0x29
bsSerialNumber:         DD "srno"
bsVolumeLabel:          DB "NO NAME    "
bsFileSystem:           DB "FAT12   "

LOAD_ADDRESS equ 0xf000; ; replaced at compilation
STACK_ADDRESS equ 0xf000
START_SECTOR equ 2
sectors_to_read:db 98;db 0 ; replaced at compilation

print:
	pusha
	mov ah, 0x0e
print_loop:
	mov al, [bx]
	cmp al, 0
	je print_end
	int 0x10
	add bx, 1
	jmp print_loop
print_end:
	mov al, 13
	int 0x10
	mov al, 10
	int 0x10
	popa
	ret

err_msg:db "Disk read error !",0
err_disp:
	mov bx, err_msg
	call print
	jmp $

gdt_start:
gdt_null_descriptor:
	dd 0, 0
gdt_code_descriptor:
	dw 0xffff, 0
	db 0, 10011010b, 11001111b, 0
gdt_data_descriptor:
	dw 0xffff, 0
	db 0, 10010010b, 11001111b, 0
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

CODE_SEGMENT equ gdt_code_descriptor - gdt_start
DATA_SEGMENT equ gdt_data_descriptor - gdt_start

welcome:db "To 32 bit PM mode !",0
start:
	; setup segments and stack
	cli
	xor bx, bx
	mov ss, bx
	mov sp, STACK_ADDRESS
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	sti

	xor bx, bx
	mov ah, 0x02
	mov al, [sectors_to_read]
	mov ch, bl ; cylinder 0
	mov cl, START_SECTOR
	mov dh, bl ; head 0
	mov es, bx
	mov bx, LOAD_ADDRESS ; es:bx = 0:LOAD_ADDRESS
	int 0x13
	jc err_disp

	cmp al, [sectors_to_read] ; check how many sectors were read, must be SECTORS_TO_READ
	jne err_disp

	mov bx, welcome
	call print

	; TO 32-bit protected mode
	cli
	lgdt [gdt_descriptor]
	mov eax, cr0
	or eax, 0x1
	mov cr0, eax
	jmp CODE_SEGMENT:init_protected_mode ; "far" jump

use32
init_protected_mode:
	mov ax, DATA_SEGMENT
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call LOAD_ADDRESS
	jmp $

times 446-($-$$) db 0   ; Pad with 0s up until first partition entry
db 0x80                 ; 0x80 = Active boot partition, 0x00=inactive
db 0x00, 0x01, 0x00     ; CHS of first absolute sector (MBR) of hard drive
                        ;     Head=0, Sector=1, Cylinder=0
db 0x0c                 ; Partition type (has to be non-zero)
                        ;     0x0c = Win 95 FAT32 (LBA)
db 0x00, 0x0f, 0x00     ; CHS of last absolute sector (MBR) of hard drive
                        ;     Head=0, Sector=1, Cylinder=0
                        ;     We are effectively saying Size of partition is 1 sector
dd 0x0                  ; LBA of first absolute sector (0=MBR)
dd 0xf                  ; Number of sectors in partition. We set it to 1 but if you
                        ;     wish you could set it to the number of sectors on the disk
times 510-($-$$) db 0
dw 0xaa55 ; magic bytes
