.PHONY: all stage1 clean

all: stage1

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin:
	nasm main.asm -f bin -o $(BUILD_DIR)/stage1.bin
