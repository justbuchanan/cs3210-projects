obj-m := sysmon.o
sysmon-objs := proc.o monitor.o
KERNEL = $(shell uname -r)
all: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) modules
clean: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean

load: all
	sudo insmod sysmon.ko
unload:
	sudo rmmod sysmon.ko
