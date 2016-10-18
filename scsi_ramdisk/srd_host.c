#include <srd_host.h>

#define SRD_CAN_QUEUE  1
#define SRD_MAX_HOSTS  16

int srd_host_id = 1;
static LIST_HEAD(srd_host_list);

static int srd_queuecommmand(struct Scsi_Host *scsi_host, 
                             struct scsi_cmnd *scmd)
{
    int i;
#ifdef DEBUG
    char buf[96];
    int  n;
#endif 

    trace();

#ifdef DEBUG
    n = snprintf (buf, 24, "Lun %04lld %s", scmd->device->lun, "received cdb: ");
    for (i = 0; i < 16 && i < scmd->cmd_len; i++) {
        n += snprintf (buf + n, 4, "%02d ", scmd->cmnd[i]);
    }
    pr_info("%s\n", buf);
#endif 

    struct srd_lun *lun = srd_is_lun_present((struct srd_host_s*)scsi_host->hostdata,
                                             scmd->device->lun);
    if ((NULL == lun) &&
        (scmd->cmd[0] != INQUIRY) && 
        (scmd->device->lun != SCSI_W_LUN_REPORT_LUNS)) {
        scmd->result = DID_NO_CONNECT << 16;
        scmd->scsi_done(scmd);
    }

    return srd_respond(scmd, lun, 0);
}

static int srd_abort(struct scsi_cmnd *cmd)
{
    trace();
    return 0;
}

static int srd_device_reset(struct scsi_cmnd *cmd)
{
    trace();
    return 0;
}

static int srd_target_reset(struct scsi_cmnd *cmd)
{
    trace();
    return 0;
}

static int srd_bus_reset(struct scsi_cmnd *cmd)
{
    trace();
    return 0;
}

static int srd_host_reset(struct scsi_cmnd *cmd)
{
    trace();
    return 0;
}

static struct scsi_host_template srd_driver_template = {
    .proc_name               = "srd",
    .name                    = "SRD driver v0.1",
    .queuecommand            = srd_queuecommmand,
    .eh_abort_handler        = srd_abort,
    .eh_device_reset_handler = srd_device_reset,
    .eh_target_reset_handler = srd_target_reset,
    .eh_bus_reset_handler    = srd_bus_reset,
    .eh_host_reset_handler   = srd_host_reset,
    .can_queue               = SRD_CAN_QUEUE,
    .this_id                 = 7,
    .sg_tablesize            = 2048,
    .cmd_per_lun             = 255,
    .max_sectors             = -1U, 
    .use_clustering          = DISABLE_CLUSTERING,
    .module                  = THIS_MODULE,
    .track_queue_depth       = 1
};

int srd_bus_driver_probe(struct device *dev)
{
	int error = 0;
	struct srd_host_s *srd_host = to_srd_host(dev);
	struct Scsi_Host *scsi_host;

	trace();
	scsi_host = scsi_host_alloc(&srd_driver_template, sizeof(struct srd_host_s));
	if (NULL == scsi_host) {
		pr_err("%s:%d scsi_host_alloc failed\n", __FILE__, __LINE__);
		error = -ENODEV;
		return error;
	}

    *((struct srd_host_s **)scsi_host->hostdata) = srd_host;
	srd_host->shost = scsi_host;
	error = scsi_add_host(scsi_host, &srd_host->dev);
	if (error != 0) {
        pr_err("scsi_add_host failed\n");
        scsi_host_put(scsi_host);
    }

    scsi_scan_host(srd_host->shost);
    trace();
    return error;
}

int srd_bus_driver_remove(struct device *dev)
{
	struct srd_host_s *srd_host = to_srd_host(dev);
	
	trace();

    scsi_remove_host(srd_host->shost);
    scsi_host_put(srd_host->shost);

	trace();
	return 0;
}

/* Host device release method registered with device_register */
static void srd_release_host(struct device *dev)
{
    struct srd_host_s *srd_host = to_srd_host(dev);
    trace();
    kfree(srd_host);
}

int srd_add_host_device(struct device *srd_root_dev, struct bus_type *srd_ll_bus)
{
    int ret;
    struct srd_host_s *srd_host = NULL;

    trace();
    
    srd_host = kzalloc(sizeof(struct srd_host_s), GFP_KERNEL);
    if (NULL == srd_host) {
        pr_err("%s:%d kzalloc failed for srd_host\n", __FILE__, __LINE__);
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&srd_host->device_list);

    srd_host->dev.bus = srd_ll_bus;
    srd_host->dev.parent = srd_root_dev;
    srd_host->dev.release = srd_release_host;
    dev_set_name(&srd_host->dev, "srd_host%02d\n", srd_host_id);

    ret = srd_attach_luns(&srd_host->device_list, srd_host_id);
    if (ret < 0) {
        kfree(srd_host);
        return ret;
    }

    /* 
     * Moment we do this probe of the earlier registered driver will be 
     * called. 
     */
    ret = device_register(&srd_host->dev);
    if (ret < 0) {
        kfree(srd_host);
        return ret;
    }

    pr_info("Added host number srd_host%02d", srd_host_id);
    srd_host_id++;

    list_add_tail(&srd_host->host_list, &srd_host_list);

    trace();
    return ret;
}

void srd_remove_host_device(void)
{
    struct srd_host_s *host = NULL;

    trace();

    if (!list_empty(&srd_host_list)) {
        host = list_entry(srd_host_list.next, struct srd_host_s, host_list);
        srd_dettach_luns(&host->device_list);
        list_del(&host->host_list);
    }

    if (NULL != host) 
        device_unregister(&host->dev);

    trace();

    return;
}

MODULE_LICENSE("GPL");
