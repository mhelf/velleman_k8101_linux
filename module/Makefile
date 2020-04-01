obj-m := k8101_driver.o
KERNEL_NAME = $(shell uname -r)
KERNEL_DIR = /lib/modules/$(KERNEL_NAME)/build
PWD=$(shell pwd)
all:
	@echo "creating module k8101"
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
clean:
	rm -rf *.o *.ko *.mod *.order *.symvers *~ *.mod.c
