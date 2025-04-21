# S4-Gadgets V4L2 Pipeline Build System

PWD := $(shell pwd)
KDIR := /lib/modules/$(shell uname -r)/build

# Targets for both AD9984A and TC358748 drivers
all:
	$(MAKE) -C $(KDIR) M=$(PWD)/drivers modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD)/drivers clean

install:
	@echo "Install via DKMS is recommended"

help:
	@echo "Usage:"
	@echo "  make         - build all kernel modules"
	@echo "  make clean   - clean build artifacts"
	@echo "  make install - (not used)"