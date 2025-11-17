BUILD_DIR = build
SRC_DIR = src

GCC = i686-elf-gcc
QEMU = qemu-system-i386

.PHONY: all build qemu

all: build $(BUILD_DIR)/os.bin

$(BUILD_DIR)/os.bin: $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o $(SRC_DIR)/linker.ld
	$(GCC) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/os.bin -ffreestanding -O2 -nostdlib $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o -lgcc

$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.s
	$(GCC) -c $(SRC_DIR)/boot.s -o $(BUILD_DIR)/boot.o

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.c
	$(GCC) -c $(SRC_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o -std=gnu99 -ffreestanding -O2 -Wall

build:
	mkdir -p $(BUILD_DIR)

qemu:
	$(QEMU) -kernel $(BUILD_DIR)/os.bin
