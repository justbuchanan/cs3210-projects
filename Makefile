obj-m = proc.o 
KERNEL = $(shell uname -r)
all: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) modules
clean: 
	make -C /lib/modules/$(KERNEL)/build M=$(PWD) clean

load: all
	sudo insmod proc.ko
unload:
	sudo rmmod proc.ko
