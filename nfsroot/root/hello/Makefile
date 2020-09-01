ifneq ($(KERNELRELEASE),)
obj-m := hello_version.o
else
KDIR := $(HOME)/Documents/1_Linux_Kernel/2_Workspace/linux-kernel-labs/src/linux
INSTALL_MOD_PATH_PREFIX := $(HOME)/Documents/1_Linux_Kernel/2_Workspace/linux-kernel-labs/modules/nfsroot
all:
	$(MAKE) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH_PREFIX) -C $(KDIR) M=$$PWD modules_install
endif
