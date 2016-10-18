#ifndef __SRD_LUN_H__
#define __SRD_LUN_H__

#include <srd_common.h>
#include <linux/slab.h>

#define SRD_MAX_LUNS 4
#define SRD_LUN_SIZE 4

struct srd_lun {
    u64 id;         /* SCSI Lun Id we want */
    int unique_id;  /* To be identified with this driver */
    int bsize;      /* Block size/sector size */
    int capacity;   /* Capacity in logical blocks */
    int lun_size;   /* lun_size = bsize * capacity */ 
    void *mm_ptr;   /* Memory pointer */
    unsigned char* sense[18]; /* The sense data */
    struct list_head host_list;
};

void srd_lun_list_free(void);
int srd_lun_list_init(void);

int srd_attach_luns(struct list_head *head, int host_id);
void srd_dettach_luns(struct list_head *head);
struct srd_lun *srd_is_lun_present (struct list_head *head, u64 lun_id);

#endif // __SRD_LUN_H__
