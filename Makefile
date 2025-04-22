# S4-Gadgets V4L2 Pipeline Build System

obj-m += s4_ad9984a.o s4_tc358748.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean