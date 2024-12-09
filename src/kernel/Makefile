.PHONY: all kernel clean

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin:
	nasm main.asm -f bin -o $(BUILD_DIR)/kernel.bin
