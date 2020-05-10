SOURCES = $(wildcard */*.c */*/*.c */*/*/*.c)
HEADERS = $(wildcard */*.h */*/*.h */*/*/*.h)
OBJS = ${SOURCES:.c=.o}
TARGET_PLATFORM ?= elf_i386

CC_PARAMS = -m32 -fno-pie -ffreestanding -w -c -g
CC = gcc


BOOT_LOAD_ADDRESS = 0xf000

BYTE_COUNT = $(shell wc -c < kernel.bin)
BC_PART = $(shell expr ${BYTE_COUNT} + 512 - 1)
BLOCK_COUNT = $(shell expr ${BC_PART} / 512)

MPCMD = 'lsblk -o NAME,MOUNTPOINT -i | grep "\-${DRIVE_NAME}" | cut -d " " -f 2'
MP = $(${MPCMD})

all: compile qemu
	@echo "Qemu run: done !"

find:
	@grep -rl "${VALUE}" .

debug_qemu: compile qemu_debug
	@echo "Qemu Debugging: done !"

compile: debug linkage boot_compilation concatenation clean
	@echo "Compilation: done !"

windows: linkage concatenation test_windows
	@echo "done !"

install_build_tools:
	@apt install fasm nasm gedit gcc qemu-system-x86

edit:
	@gedit ${SOURCES} &
	@gedit -s ${HEADERS} &


debug:
	@printf "Sources: "
	@echo "${OBJS}" | awk -F" " '{print NF}'

boot_compilation:
	@echo "Kernel size: ${BYTE_COUNT}"
	@echo "Blocks: ${BLOCK_COUNT}"
	@cp boot/bootloader.asm boot/bootloader_mod.asm
	@sed -i -e 's/sectors_to_read:/sectors_to_read:db ${BLOCK_COUNT};/g' -e 's/LOAD_ADDRESS equ/LOAD_ADDRESS equ ${BOOT_LOAD_ADDRESS};/g' boot/bootloader_mod.asm
	@fasm boot/bootloader_mod.asm boot/bootloader.bin

linkage: boot/kernel_entry.o kernel/interrupts/interrupt_packer.o ${OBJS}
	@echo "LD: kernel.bin"
	@ld -m ${TARGET_PLATFORM} -o kernel.bin -Ttext ${BOOT_LOAD_ADDRESS} $^ --oformat binary

%.o: %.c ${HEADERS}
	@echo "CC: $@"
	@${CC} ${CC_PARAMS} $< -o $@

%.o: %.asm
	@echo "ASM: $@"
	@nasm $< -f elf -o $@

concatenation:
	@cat boot/bootloader.bin kernel.bin > KaptainOS.bin

upload:
	@echo "Selected : ${DRIVE_NAME} (MP: ${MP})"
	@dd if=KaptainOS.bin of=/dev/${DRIVE_NAME} bs=512 && sync

qemu:
	@qemu-system-i386 -hda KaptainOS.bin

qemu_sata:
	@qemu-system-i386 -hda KaptainOS.bin -drive file=ramdisk,if=none,id=img0 -device ich9-ahci,id=ahci -device ide-drive,drive=img0,bus=ahci.0

upload_win:
	@cp KaptainOS.bin '/run/user/1000/gvfs/smb-share:server=alfred-pc,share=netshare/kos/KaptainOS.bin'

qemu_debug:
	@qemu-system-i386 -hda KaptainOS.bin -s -S &
	@gdb -ex="target remote localhost:1234" -ex="symbol-file kernel/kernel.o"

test_windows:
	@C:\Program Files (x86)\qemu\qemu-system-i386w.exe -hda KaptainOS.bin

dump:
	@dd if=/dev/${DRIVE_NAME} of=dump.bin bs=512 count=16

clean:
	@rm ${OBJS} kernel.bin boot/bootloader.bin

backup: SUFFIX = $(shell bash -c 'read -p "File name suffix: " cmt; echo $$cmt')
backup:
	@$(shell bash -c 'read -p "Comment: " cmt; echo $$cmt > backup_note.txt')
	zip -x "project_backup/*" -r $(shell date +"project_backup/%F_%Hh%M-${SUFFIX}.zip") .
	@rm backup_note.txt
	@echo "Project backed up !"
