# S4-Gadgets V4L2 Pipeline Build System

PWD := $(shell pwd)
KDIR := /lib/modules/$(KERNELRELEASE)/build

obj-m += s4_ad9984a.o
obj-m += s4_tc358748.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean