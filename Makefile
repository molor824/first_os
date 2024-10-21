build/floppy.img: build build/bootloader.bin
	cp build/bootloader.bin build/floppy.img
	truncate -s 1440k build/floppy.img

build/bootloader.bin: build src/bootloader/main.asm
	nasm src/bootloader/main.asm -f bin -o build/bootloader.bin

build:
	mkdir -p build