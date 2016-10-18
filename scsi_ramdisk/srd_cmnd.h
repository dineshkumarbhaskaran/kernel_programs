#ifndef __SRD_CMND_H__
#define __SRD_CMND_H__

#include <srd_common.h>
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <srd_lun.h>

#define INVALID_OPCODE 0x20

#define SRD_MAX_INQ_SIZE        0x60
#define SRD_NO_DEVICE           0x7F

struct srd_host_s;

int srd_respond(struct scsi_cmnd *cmnd, struct srd_lun *lun,
                  int scsi_result);
#endif //__SRD_CMND_H__
