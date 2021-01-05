KERNELDIR := /home/lsy/tools/kernel_src/zdyz_4_NXP_Original_kernel/linux-imx-rel_imx_4.1.15_2.1.0_ga
CURRENT_PATH := $(shell pwd)

obj-m := timer_led.o
#OBJ := $(patsubst %.c,%.o,$(wildcard *.c))
#obj-m := $(OBJ)

build: kernel_modules
kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

.PHONY:
print:
	@echo "OBJ = $(OBJ)"
	@echo "obj-m = $(obj-m)"
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
