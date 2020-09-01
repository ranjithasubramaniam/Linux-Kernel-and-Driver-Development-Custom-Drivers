ifneq ($(KERNELRELEASE),)
obj-m := drvbroken.o
else
KDIR := /home/suresh/Documents/1_Linux_Kernel/2_Workspace/linux-kernel-labs/src/linux
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
