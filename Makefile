obj-m := sysmon.o
sysmon-objs := proc.o monitor.o log.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
KERNEL = $(shell uname -r)
all: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) modules
clean: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean

load: all
	sudo insmod sysmon.ko
unload:
	sudo rmmod sysmon.ko
reload: unload load

pretty:
	@stylize --clang_style=file
