ifeq ($(KERNELRELEASE),)
APP_NAME0 := app_imx291
APP_NAME1 := app_camera
KERNELDIR := /home/root/CQA83TLinux_Qt5.8.0_bv3/linux-3.4/
#KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
export ARCH=arm
#export CROSS_COMPILE=/home/work/toolchains/arm-buildroot-linux-gnueabihf/bin/arm-buildroot-linux-gnueabihf-
export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-

modules:
	#$(MAKE) ARCH=arm CROSS_COMPILE=arm-buildroot-linux-gnueabihf- -C $(KERNELDIR) M=$(PWD) modules
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	$(CROSS_COMPILE)gcc $(APP_NAME0).c -o $(APP_NAME0)
	$(CROSS_COMPILE)gcc $(APP_NAME1).c -o $(APP_NAME1) -I /home/root/CQA83TLinux_Qt5.8.0_bv3/linux-3.4/include/

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf *.o $(APP_NAME0) $(APP_NAME1)

else
    obj-m := driver_imx291_v3.o
#   obj-m += driver_imx291_demo.o
endif
