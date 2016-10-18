ROOT=/home/dinesh/kernel_programs/scsi_ramdisk

EXTRA_CFLAGS:=-I$(ROOT) -DDEBUG 
obj-m += srd.o 

srd-objs := srd_main.o srd_host.o srd_lun.o srd_cmnd.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

