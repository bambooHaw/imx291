ifeq ($(KERNELRELEASE),)
APP_NAME := app_iic
KERNELDIR := /opt/hxs/projBoxV3/qt5_8/linux-3.4/
#KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
export ARCH=arm
export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(CROSS_COMPILE)gcc $(APP_NAME).c -o $(APP_NAME)

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf *.o $(APP_NAME)

else
    obj-m := driver_iic_boxV3.o
endif
