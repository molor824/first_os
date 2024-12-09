.PHONY: all stage2 clean

all: stage2

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin:
	nasm main.asm -f bin -o $(BUILD_DIR)/stage2.bin
