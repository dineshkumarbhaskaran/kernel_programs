#include <srd_cmnd.h>

#define CHECK_CONDITION_RESULT \
    ((DRIVER_SENSE << 24) | SAM_STAT_CHECK_CONDITION);

#define srd_make_sense_invalid_opcode(__buf) \
    srd_make_sense_buffer(__buf, ILLEGAL_REQUEST, INVALID_OPCODE, 0)

void srd_make_sense_buffer(unsigned char *buf, int key, int asc, int ascq)
{
    trace();

    if (WARN_ON(!buf)) {
        return;
    }

    memset(buf, 0, SCSI_SENSE_BUFFERSIZE);
    scsi_build_sense_buffer(0, buf, key, asc, ascq);

    pr_err("%s : sense 0x%x, 0x%x, 0x%x\n", "my_dev", key, asc, ascq);
}

static int srd_complete_cmd(struct scsi_cmnd *scmd, struct srd_lun *lun)
{

}

static int srd_respond_inquiry(struct scsi_cmnd *scmd, struct srd_lun *lun)
{
    unsigned char *cmd = scmd->cmnd;
    unsigned char arr[SRD_MAX_INQ_SIZE];
    unsigned int alloc_len = cmd[3] << 8 + cmd[4];
    unsigned int page_code = cmd[2];
    unsigned int is_device = 1;
    unsigned int ret;

    if (NULL == lun) {
        is_device = 0;
    }

    arr[0] = (is_device == 0) ? SRD_NO_DEVICE : 0x0;
    if (0x1 == cmd[1] & 0x1) {
        /* EVPD bit is set, that we dont support */
        srd_make_sense_invalid_opcode(lun->sense);
        ret = CHECK_CONDITION_RESULT;
    } else {
        arr[1] = 0; // Not a removable media.
        arr[2] = 0x6; // Complying to SPC-4 
    }

    return ret;
}

int srd_respond(struct scsi_cmnd *scmd, struct srd_lun *lun)
{
    int ret = 0;
    int scsi_result = 0;
    unsigned char *cmd = scmd->cmnd;

    trace();

    if ((NULL == lun) && (cmd[0] != INQUIRY)) {
        trace();
        scsi_result = DID_NO_CONNECT << 16;
        scmd->result = DID_NO_CONNECT << 16;
        scmd->scsi_done(scmd);
        return 0;
    }

    switch (cmd[0]) {
        case INQUIRY:
            srd_respond_inquiry(scmd, lun);
            break;

        default:
            srd_make_sense_invalid_opcode(lun->sense);
            scmd->result = CHECK_CONDITION_RESULT;
            scmd->scsi_done(scmd);
            break;
    }

    trace();
    return 0;
}

