MODULE_NAME := kfetch
STUDENT_ID := 313560005
obj-m += kfetch_mod_313560005.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	sudo insmod kfetch_mod_313560005.ko

unload:
	sudo rmmod kfetch_mod_313560005
