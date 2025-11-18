BUILD_DIR := build
SRC_DIR := src
INCL_DIR := include

GCC := i686-elf-gcc
QEMU := qemu-system-i386

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

.PHONY: all build qemu clean

all: build $(BUILD_DIR)/os.bin

$(BUILD_DIR)/os.bin: $(BUILD_DIR)/boot.o $(OBJECTS) $(SRC_DIR)/linker.ld
	$(GCC) -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/os.bin -ffreestanding -g -nostdlib -lgcc $(BUILD_DIR)/boot.o $(OBJECTS)

$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.s
	$(GCC) -c $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(GCC) -c -o $@ -std=gnu99 -ffreestanding -g -Wall -I $(INCL_DIR) $^

build:
	mkdir -p $(BUILD_DIR)

qemu:
	$(QEMU) -kernel $(BUILD_DIR)/os.bin

qemu-debug:
	$(QEMU) -s -S -kernel $(BUILD_DIR)/os.bin

clean:
	rm -rf build
