obj-m := lock_kmod.o
lock_kmod-objs := syscall.o proc.o main.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
KERNEL = $(shell uname -r)

all: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) modules
clean: unload
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean

load: all
    ifeq ($(shell lsmod | grep lock_kmod),)
		sudo insmod lock_kmod.ko
    else
		@echo "Already loaded."
    endif

unload:
    ifneq ($(shell lsmod | grep lock_kmod),)
		sudo rmmod lock_kmod.ko
    else
		@echo "Already unloaded."
    endif

reload: unload load

pretty:
	@stylize --clang_style=file

