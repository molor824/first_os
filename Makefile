CFLAGS=-ffreestanding -nostdlib -lgcc -std=gnu99 -Wall
BUILD_DIR=build
SRC_DIR=src
KERNEL_BUILD_DIR=build/kernel
KERNEL_SRC_DIR=src/kernel

GCC=i686-elf-gcc
QEMU=qemu-system-i386

# Kernel sources/objects

KERNEL_SOURCES=$(wildcard $(KERNEL_SRC_DIR)/*.c)
KERNEL_OBJECTS=$(patsubst $(KERNEL_SRC_DIR)/%.c, $(KERNEL_BUILD_DIR)/%.o, $(KERNEL_SOURCES))

# Boot sources/objects

BOOT_SRC=$(SRC_DIR)/boot.s
BOOT_OBJ=$(BUILD_DIR)/boot.o

CRTI_SRC=$(SRC_DIR)/crti.s
CRTN_SRC=$(SRC_DIR)/crtn.s

CRTI_OBJ=$(BUILD_DIR)/crti.o
CRTBEGIN_OBJ=$(shell $(GCC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ=$(shell $(GCC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=$(BUILD_DIR)/crtn.o

LINKER_SRC=$(SRC_DIR)/linker.ld
OS_BIN=$(BUILD_DIR)/first_os.bin

.PHONY: all build qemu clean bootable

all: build $(OS_BIN)

# NOTE: The orders must not change!!!
LINK_OBJS=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(BOOT_OBJ) $(KERNEL_OBJECTS) $(CRTEND_OBJ) $(CRTN_OBJ)

$(OS_BIN): $(LINK_OBJS) $(LINKER_SRC) Makefile
	$(GCC) -T $(LINKER_SRC) -o $(OS_BIN) -nostdlib -lgcc $(LINK_OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s Makefile
	$(GCC) -c -o $@ $(CFLAGS) $<

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c Makefile
	$(GCC) -c -o $@ $(CFLAGS) $<

build:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(KERNEL_BUILD_DIR)

qemu:
	$(QEMU) -kernel $(OS_BIN)

qemu-debug:
	$(QEMU) -s -S -kernel $(OS_BIN)

clean:
	rm -rf $(BUILD_DIR)/*

OS_IMG=$(BUILD_DIR)/first_os.img
GRUB_CFG=$(SRC_DIR)/grub.cfg
ISO_DIR=$(BUILD_DIR)/isodir
ISO_OS=$(ISO_DIR)/boot/first_os.bin
ISO_GRUB=$(ISO_DIR)/boot/grub/grub.cfg

bootable: $(OS_IMG)

$(OS_IMG): $(GRUB_CFG) $(OS_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(OS_BIN) $(ISO_OS)
	cp $(GRUB_CFG) $(ISO_GRUB)
	grub-mkrescue -o $(OS_IMG) $(ISO_DIR)
