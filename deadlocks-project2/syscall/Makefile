obj-m := syscall.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
KERNEL = $(shell uname -r)

all: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) modules
clean: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean

load: all
	sudo insmod syscall.ko
unload:
	sudo rmmod syscall.ko

pretty:
	@stylize --clang_style=file
