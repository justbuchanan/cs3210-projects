obj-m := lock_kmod.o
lock_kmod-objs := syscall.o main.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
KERNEL = $(shell uname -r)

all: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) modules
clean: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean

load: all
	sudo insmod lock_kmod.ko
unload:
	sudo rmmod lock_kmod.ko

pretty:
	@stylize --clang_style=file
