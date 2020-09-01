ifneq ($(KERNELRELEASE),)
obj-m := nunchuk.o
else
KDIR := $(HOME)/Documents/1_Linux_Kernel/2_Workspace/linux-kernel-labs/src/linux
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
