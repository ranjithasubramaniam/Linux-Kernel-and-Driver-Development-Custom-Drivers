ifneq ($(KERNELRELEASE),)
obj-m := serial.o
#CFLAGS_serial.o := -DDEBUG
#$(info    VAR is $(CONFIG_DRIVER))
else
KDIR := $(HOME)/Documents/1_Linux_Kernel/2_Workspace/linux-kernel-labs/src/linux
all:
	$(MAKE) -C $(KDIR) M=$$PWD
endif
