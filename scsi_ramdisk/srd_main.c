#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> 

#include <srd_common.h>
#include <srd_lun.h>
#include <srd_host.h>

static struct device *srd_root_dev = NULL;

static int srd_ll_bus_match(struct device *dev, struct device_driver *driver) 
{
    trace();

    return 1;
}

static struct bus_type srd_ll_bus = {
    .name      = "srd_ll_bus",
    .dev_attrs = NULL, 
    .match     = srd_ll_bus_match,
    .probe     = srd_bus_driver_probe,
    .remove    = srd_bus_driver_remove
};

static struct device_driver srd_bus_device_driver = {
    .name      = "srd_bus_device_driver",
    .bus       = &srd_ll_bus,
};

static int __init srd_init(void)
{
    int ret;

    trace();

    ret = srd_lun_list_init();
    if (ret < 0) {
        pr_err("Lun list creation failed\n");
        return ret;
    }

    pr_err("Lun list created\n");

    srd_root_dev = root_device_register("scsi_ramdisk");
    if (IS_ERR(srd_root_dev)) {
        pr_err("srd root device register failed\n");
        srd_lun_list_free();
        ret = PTR_ERR(srd_root_dev);
        return ret;
    }

    pr_info("srd_root_dev registered\n");

    /* Register a psuedo bus for the pseudo device */
    ret = bus_register(&srd_ll_bus);
    if (ret < 0) {
        pr_err("srd low level bus register failed\n");
        srd_lun_list_free();
        root_device_unregister(srd_root_dev);
        return ret;
    }

    pr_info("srd_ll_bus registered\n");

    /* Now lets register a driver with the bus type */
    ret = driver_register(&srd_bus_device_driver);
    if (ret < 0) {
        pr_err("srd device driver register failed\n");
        srd_lun_list_free();
        bus_unregister(&srd_ll_bus);
        root_device_unregister(srd_root_dev);
        return ret;
    }

    pr_info("srd device driver registered\n");
     
    /* Let us add one adapter now */
    ret = srd_add_host_device(srd_root_dev, &srd_ll_bus);
    if (ret < 0) {
        pr_err("srd_add_host failed\n");
        srd_lun_list_free();
        driver_unregister(&srd_bus_device_driver);
        bus_unregister(&srd_ll_bus);
        root_device_unregister(srd_root_dev);
        return ret;
    }

    pr_info("srd host added\n");

    trace();

    return 0;    
}

static void __exit srd_exit(void)
{
    trace();

    srd_lun_list_free();
    pr_info("srd_lun_list_free completed\n");

    srd_remove_host_device();
    pr_info("srd_remove_host completed\n");

    driver_unregister(&srd_bus_device_driver);
    pr_info("driver unregistered.\n");

    bus_unregister(&srd_ll_bus);
    pr_info("bus unregistered.\n");

    root_device_unregister(srd_root_dev);
    pr_info("root device unregistered.\n");

    trace();
}

module_init(srd_init);
module_exit(srd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DINESH K B");
