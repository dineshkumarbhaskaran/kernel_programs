#ifndef __SRD_HOST_H__
#define __SRD_HOST_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>

#include <scsi/scsi_host.h>
#include <scsi/scsi_cmnd.h>

#include <srd_common.h>
#include <srd_lun.h>
#include <srd_cmnd.h>

struct srd_host_s {
    struct Scsi_Host *shost;
    struct device dev;
    struct list_head device_list;
    struct list_head host_list;
}; 

#define to_srd_host(d) container_of(d, struct srd_host_s, dev)

int srd_bus_driver_probe(struct device *dev);
int srd_bus_driver_remove(struct device *dev);
int srd_add_host_device(struct device *srd_root_dev, struct bus_type *srd_ll_bus);
void srd_remove_host_device(void);

#endif //__SRD_HOST_H__
