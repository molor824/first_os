CFLAGS=-ffreestanding -nostdlib -lgcc -std=gnu11 -Wall -m32
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

# NOTE TO SELF: Put -lgcc after all the input objects to avoid "undefined reference to `__aeabi_idiv`" errors
# Generally it seems to be a good idea to put inputs before libraries
$(OS_BIN): $(LINK_OBJS) $(LINKER_SRC) Makefile
	$(GCC) $(LINK_OBJS) -T $(LINKER_SRC) $(CFLAGS) -o $(OS_BIN)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s Makefile
	$(GCC) $< $(CFLAGS) -g -c -o $@

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SRC_DIR)/%.c Makefile
	$(GCC) $< $(CFLAGS) -g -c -o $@

build:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(KERNEL_BUILD_DIR)

qemu:
	$(QEMU) -kernel $(OS_BIN)

qemu-debug:
	$(QEMU) -s -S -kernel $(OS_BIN)

clean:
	rm -rf $(BUILD_DIR)/*

OS_ISO=$(BUILD_DIR)/first_os.iso
GRUB_CFG=$(SRC_DIR)/grub.cfg
ISO_DIR=$(BUILD_DIR)/isodir
ISO_OS_BIN=$(ISO_DIR)/boot/first_os.bin
ISO_GRUB_CFG=$(ISO_DIR)/boot/grub/grub.cfg

bootable: $(OS_ISO)

$(OS_ISO): $(GRUB_CFG) $(OS_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(OS_BIN) $(ISO_OS_BIN)
	cp $(GRUB_CFG) $(ISO_GRUB_CFG)
	grub-mkrescue -o $(OS_ISO) $(ISO_DIR)
