.PHONY: all os_image kernel bootloader clean always tools_fat

all: os_image

os_image: build/os.img

build/os.img: bootloader kernel
	dd if=/dev/zero of=build/os.img bs=512 count=2880
	mkfs.fat -F 12 -n "NBOS" build/os.img
	dd if=build/bootloader.bin of=build/os.img conv=notrunc
	mcopy -i build/os.img build/kernel.bin "::kernel.bin"
	mcopy -i build/os.img test.txt "::test.txt"

bootloader: build/bootloader.bin
build/bootloader.bin: always
	nasm src/bootloader/main.asm -f bin -o build/bootloader.bin

kernel: build/kernel.bin
build/kernel.bin: always
	nasm src/kernel/main.asm -f bin -o build/kernel.bin

always:
	mkdir -p build

clean:
	rm -rf build/*