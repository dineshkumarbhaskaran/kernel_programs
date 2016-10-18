#include <srd_lun.h>

struct srd_lun srd_lun_list[SRD_MAX_LUNS]; 
int srd_lun_host_list[32] = {1, 1, 2, 1, };
int srd_lun_id[SRD_MAX_LUNS] = {1, 2, 1, 8};

int __srd_lun_list_init(struct srd_lun *lun_list)
{
    int i;
    int lun_size = SRD_LUN_SIZE * 1024 * 1024;
    int bsize = 512;
    int capacity = lun_size / bsize;

    trace();
    for (i = 0; i < SRD_MAX_LUNS; i++) {
        lun_list[i].id = srd_lun_id[i];
        lun_list[i].unique_id = i;
        lun_list[i].bsize = bsize;
        lun_list[i].capacity = capacity;
        lun_list[i].lun_size = lun_size;
    }

    trace();
    return 0;
}

int srd_lun_list_init()
{
    int i;
    int ret;

    trace();
    memset(srd_lun_list, 0, sizeof(srd_lun_list));

    ret = __srd_lun_list_init(srd_lun_list);
    if (ret != 0) {
        trace();
        return -ENOMEM;
    }

    for (i = 0; i < SRD_MAX_LUNS; i++) {
        pr_info ("Allocating memory of order %d\n", 
                        get_order(srd_lun_list[i].lun_size));
        srd_lun_list[i].mm_ptr = (void *)__get_free_pages(GFP_KERNEL, 
                                    get_order(srd_lun_list[i].lun_size));
        if (NULL == srd_lun_list[i].mm_ptr) {
            return -ENOMEM;
        }
    }

    trace();
    return 0;
}

void srd_lun_list_free()
{
    int i;

    trace();
    for (i = 0; i < SRD_MAX_LUNS; i++) {
        free_pages((unsigned long)srd_lun_list[i].mm_ptr, 
                    get_order(srd_lun_list[i].lun_size));
    }
    trace();
}

struct srd_lun *srd_is_lun_present (struct list_head *head, u64 lun_id)
{
    struct srd_lun *lun;

    for (; !list_empty(head); head = head->prev) {
        lun = list_entry(head, struct srd_lun, host_list);
        if (lun->id == lun_id) {
            return lun;
        }
    }

    return NULL;

}

int srd_attach_luns(struct list_head *head, int host_id)
{
    int i;

    trace();

    for (i = 0; i < SRD_MAX_LUNS; i++) {
        if (srd_lun_host_list[i] == host_id) {
            pr_info ("%p %p", &srd_lun_list[i].host_list, head);
            list_add_tail(&srd_lun_list[i].host_list, head);
        }
    }

    trace();
    return 0;
}

void srd_dettach_luns(struct list_head *head)
{
    trace();

    while (!list_empty(head)) {
        list_del(head->next);
        head = head->prev;
    }

    trace();
}

