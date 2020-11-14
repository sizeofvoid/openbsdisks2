/*
 * Most of this code is borrowed from /usr/src/sbin/camcontrol/camcontrol.c
 * Original license banner:
 *
 * Copyright (c) 1997-2007 Kenneth D. Merry
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <cam/ata/ata_all.h>
#include <cam/cam.h>
#include <cam/cam_ccb.h>
#include <cam/cam_debug.h>
#include <cam/scsi/scsi_all.h>
#include <cam/scsi/scsi_da.h>
#include <cam/scsi/scsi_message.h>
#include <cam/scsi/scsi_pass.h>
#include <cam/scsi/smp_all.h>
#include <camlib.h>

#include "camcontrolprober.h"

// stolen from <sys/endian.h>
#if __FreeBSD_version < 1200000
/*
 * Host to big endian, host to little endian, big endian to host, and little
 * endian to host byte order functions as detailed in byteorder(9).
 */
#if _BYTE_ORDER == _LITTLE_ENDIAN
#define htobe16(x) bswap16((x))
#define htobe32(x) bswap32((x))
#define htobe64(x) bswap64((x))
#define htole16(x) ((uint16_t)(x))
#define htole32(x) ((uint32_t)(x))
#define htole64(x) ((uint64_t)(x))

#define be16toh(x) bswap16((x))
#define be32toh(x) bswap32((x))
#define be64toh(x) bswap64((x))
#define le16toh(x) ((uint16_t)(x))
#define le32toh(x) ((uint32_t)(x))
#define le64toh(x) ((uint64_t)(x))
#else /* _BYTE_ORDER != _LITTLE_ENDIAN */
#define htobe16(x) ((uint16_t)(x))
#define htobe32(x) ((uint32_t)(x))
#define htobe64(x) ((uint64_t)(x))
#define htole16(x) bswap16((x))
#define htole32(x) bswap32((x))
#define htole64(x) bswap64((x))

#define be16toh(x) ((uint16_t)(x))
#define be32toh(x) ((uint32_t)(x))
#define be64toh(x) ((uint64_t)(x))
#define le16toh(x) bswap16((x))
#define le32toh(x) bswap32((x))
#define le64toh(x) bswap64((x))
#endif /* _BYTE_ORDER == _LITTLE_ENDIAN */
#endif

#if __FreeBSD_version < 1300000
void ata_param_fixup(struct ata_params* ident_buf);
#endif

struct ata_res_pass16 {
    u_int16_t reserved[5];
    u_int8_t flags;
    u_int8_t error;
    u_int8_t sector_count_exp;
    u_int8_t sector_count;
    u_int8_t lba_low_exp;
    u_int8_t lba_low;
    u_int8_t lba_mid_exp;
    u_int8_t lba_mid;
    u_int8_t lba_high_exp;
    u_int8_t lba_high;
    u_int8_t device;
    u_int8_t status;
};

typedef enum {
    CC_DT_NONE,
    CC_DT_SCSI,
    CC_DT_SATL,
    CC_DT_ATA,
    CC_DT_NVME,
    CC_DT_MMCSD,
    CC_DT_UNKNOWN
} camcontrol_devtype;

static enum camtransport map_transport(int transport);
static int
get_device_type(struct cam_device* dev, camcontrol_devtype* devtype, int* is_sata);
static int
scsiinquiry(struct cam_device* device, int task_attr, int retry_count,
    int* result);
static int
ataidentify(struct cam_device* device, int retry_count, int* is_removable, int* transport);
static int
scsi_cam_pass_16_send(struct cam_device* device, union ccb* ccb, int quiet);
int ata_do_identify(struct cam_device* device, int retry_count,
    struct ccb_pathinq* cpi, union ccb* ccb, struct ata_params** ident_bufp);
static int
ata_read_native_max(struct cam_device* device, int retry_count,
    u_int32_t timeout, union ccb* ccb,
    struct ata_params* parm, u_int64_t* hpasize);
static int
get_cpi(struct cam_device* device, struct ccb_pathinq* cpi);
static int
get_cgd(struct cam_device* device, struct ccb_getdev* cgd);
static int
ata_do_28bit_cmd(struct cam_device* device, union ccb* ccb, int retries,
    u_int32_t flags, u_int8_t protocol, u_int8_t tag_action,
    u_int8_t command, u_int8_t features, u_int32_t lba,
    u_int8_t sector_count, u_int8_t* data_ptr, u_int16_t dxfer_len,
    int timeout, int quiet);
static int
ata_do_cmd(struct cam_device* device, union ccb* ccb, int retries,
    u_int32_t flags, u_int8_t protocol, u_int8_t ata_flags,
    u_int8_t tag_action, u_int8_t command, u_int8_t features,
    u_int64_t lba, u_int8_t sector_count, u_int8_t* data_ptr,
    u_int16_t dxfer_len, int timeout, int force48bit);
static int
atahpa_proc_resp(struct cam_device* device, union ccb* ccb,
    int is48bit, u_int64_t* hpasize);
static int
ata_try_pass_16(struct cam_device* device);
static int
ata_do_pass_16(struct cam_device* device, union ccb* ccb, int retries,
    u_int32_t flags, u_int8_t protocol, u_int8_t ata_flags,
    u_int8_t tag_action, u_int8_t command, u_int8_t features,
    u_int64_t lba, u_int8_t sector_count, u_int8_t* data_ptr,
    u_int16_t dxfer_len, int timeout, int quiet);
static int
ata_cam_send(struct cam_device* device, union ccb* ccb, int quiet);
int dev_has_vpd_page(struct cam_device* dev, uint8_t page_id, int retry_count,
    int timeout, int verbosemode);

static void warnx(const char* arg) { printf("%s\n", arg); }
static void errx(int a, const char* b, const char* c) { printf("%s: %s\n", b, c); }

int camcontrol(const char* device_path, struct camcontrolreturn* retval)
{
    char name[30];
    char* device = name;
    int task_attr = MSG_SIMPLE_Q_TAG;
    struct cam_device* cam_dev = NULL;
    int error = 1;
    int is_removable = 0;
    int retry_count = 1;
    int unit = 0;
    int transport;

    if (cam_get_device(device_path, name, sizeof name, &unit) == -1) {
        errx(1, "%s", cam_errbuf);
        return (-1);
    }

    if ((cam_dev = cam_open_spec_device(device, unit, O_RDWR, NULL))
        == NULL) {
        errx(1, "%s", cam_errbuf);
        return (-1);
    }

    camcontrol_devtype devtype;
    int is_sata; // only set if devtype == CC_DT_ATA
    get_device_type(cam_dev, &devtype, &is_sata);

    if (devtype == CC_DT_SCSI) {
        struct ccb_pathinq cpi;
        error = scsiinquiry(cam_dev, task_attr, retry_count, &is_removable);
        if (get_cpi(cam_dev, &cpi) != 0)
            warnx("couldn't get CPI");
        retval->is_removable = is_removable;
        retval->transport = map_transport(cpi.transport);
    }
    else if (devtype == CC_DT_ATA) {
        error = ataidentify(cam_dev, retry_count, &is_removable, &transport);
        retval->is_removable = is_removable;
        retval->transport = map_transport(transport);
        retval->is_sata = is_sata;
    }
    else if (devtype == CC_DT_SATL) {
        // don't do ataidentify here, see get_device_type() comment why
        error = 0;
        retval->is_removable = 0;
        retval->transport = CAM_TR_UNKNOWN;
        retval->is_sata = is_sata;
    }
    else
        warnx("camprober: unhandled devtype");

    cam_close_device(cam_dev);

    if (error)
        return (-1);

    return 0;
}

static enum camtransport map_transport(int transport)
{
    switch (transport) {
    case XPORT_SPI:
        return CAM_TR_SCSI;
    case XPORT_USB:
        return CAM_TR_USB;
    case XPORT_ATA:
        return CAM_TR_ATA;
    default:
        return CAM_TR_UNKNOWN;
    }
}

/*
 * devtype is filled in with the type of device.
 * Returns 0 for success, non-zero for failure.
 */
static int
get_device_type(struct cam_device* dev, camcontrol_devtype* devtype, int* is_sata)
{
    struct ccb_getdev cgd;
    int retval;

    retval = get_cgd(dev, &cgd);
    if (retval != 0)
        goto bailout;

    *is_sata = 0;

    switch (cgd.protocol) {
    case PROTO_SCSI:
        break;
    case PROTO_SATAPM:
        *is_sata = 1;
    case PROTO_ATA:
    case PROTO_ATAPI:
        *devtype = CC_DT_ATA;
        goto bailout;
        break; /*NOTREACHED*/
    case PROTO_NVME:
        *devtype = CC_DT_NVME;
        goto bailout;
        break; /*NOTREACHED*/
#if __FreeBSD_version >= 1200000
    case PROTO_MMCSD:
        *devtype = CC_DT_MMCSD;
        goto bailout;
        break; /*NOTREACHED*/
#endif
    default:
        *devtype = CC_DT_UNKNOWN;
        goto bailout;
        break; /*NOTREACHED*/
    }

    // if (retry_count == -1) {
    /*
        * For a retry count of -1, use only the cached data to avoid
        * I/O to the drive. Sending the identify command to the drive
        * can cause issues for SATL attachaed drives since identify is
        * not an NCQ command.
        */
    if (cgd.ident_data.config != 0)
        *devtype = CC_DT_SATL;
    else
        *devtype = CC_DT_SCSI;
    retval = 0;

bailout:
    return (retval);
}

static int
ataidentify(struct cam_device* device, int retry_count, int* is_removable, int* transport)
{
    union ccb* ccb;
    struct ata_params* ident_buf;
    struct ccb_pathinq cpi;
    u_int64_t hpasize;

    if ((ccb = cam_getccb(device)) == NULL) {
        warnx("couldn't allocate CCB");
        return (1);
    }

    if (get_cpi(device, &cpi) != 0) {
        warnx("couldn't get CPI");
        return (-1);
    }

    if (ata_do_identify(device, retry_count, &cpi, ccb, &ident_buf) != 0) {
        cam_freeccb(ccb);
        return (1);
    }

    if (ident_buf->support.command1 & ATA_SUPPORT_PROTECTED) {
        if (ata_read_native_max(device, retry_count, 0, ccb,
                ident_buf, &hpasize)
            != 0) {
            cam_freeccb(ccb);
            return (1);
        }
    }
    else {
        hpasize = 0;
    }

    *is_removable = ident_buf->enabled.extension & ATA_SUPPORT_REMOVABLE;
    *transport = cpi.transport;

    free(ident_buf);
    cam_freeccb(ccb);

    return (0);
}

int ata_do_identify(struct cam_device* device, int retry_count,
    struct ccb_pathinq* cpi, union ccb* ccb, struct ata_params** ident_bufp)
{
    struct ata_params* ident_buf;
    struct ccb_getdev cgd;
    u_int i, error;
    uint16_t* ptr;
    u_int8_t command, retry_command;

    /* Neither PROTO_ATAPI or PROTO_SATAPM are used in cpi.protocol */
    if (cpi->protocol == PROTO_ATA) {
        if (get_cgd(device, &cgd) != 0) {
            warnx("couldn't get CGD");
            return (-1);
        }

        command = (cgd.protocol == PROTO_ATA) ? ATA_ATA_IDENTIFY : ATA_ATAPI_IDENTIFY;
        retry_command = 0;
    }
    else {
        /* We don't know which for sure so try both */
        command = ATA_ATA_IDENTIFY;
        retry_command = ATA_ATAPI_IDENTIFY;
    }

    ptr = (uint16_t*)calloc(1, sizeof(struct ata_params));
    if (ptr == NULL) {
        warnx("can't calloc memory for identify\n");
        return (1);
    }

    error = ata_do_28bit_cmd(device,
        ccb,
        /*retries*/ retry_count,
        /*flags*/ CAM_DIR_IN,
        /*protocol*/ AP_PROTO_PIO_IN,
        /*tag_action*/ MSG_SIMPLE_Q_TAG,
        /*command*/ command,
        /*features*/ 0,
        /*lba*/ 0,
        /*sector_count*/ 0,
        /*data_ptr*/ (u_int8_t*)ptr,
        /*dxfer_len*/ sizeof(struct ata_params),
        /*timeout*/ 30 * 1000,
        /*quiet*/ 1);

    if (error != 0) {
        if (retry_command == 0) {
            free(ptr);
            return (1);
        }
        error = ata_do_28bit_cmd(device,
            ccb,
            /*retries*/ retry_count,
            /*flags*/ CAM_DIR_IN,
            /*protocol*/ AP_PROTO_PIO_IN,
            /*tag_action*/ MSG_SIMPLE_Q_TAG,
            /*command*/ retry_command,
            /*features*/ 0,
            /*lba*/ 0,
            /*sector_count*/ 0,
            /*data_ptr*/ (u_int8_t*)ptr,
            /*dxfer_len*/ sizeof(struct ata_params),
            /*timeout*/ 30 * 1000,
            /*quiet*/ 0);

        if (error != 0) {
            free(ptr);
            return (1);
        }
    }

    ident_buf = (struct ata_params*)ptr;
    ata_param_fixup(ident_buf);

    error = 1;
    for (i = 0; i < sizeof(struct ata_params) / 2; i++) {
        if (ptr[i] != 0)
            error = 0;
    }

    /* check for invalid (all zero) response */
    if (error != 0) {
        warnx("Invalid identify response detected");
        free(ptr);
        return (error);
    }

    *ident_bufp = ident_buf;

    return (0);
}

static int
ata_read_native_max(struct cam_device* device, int retry_count,
    u_int32_t timeout, union ccb* ccb,
    struct ata_params* parm, u_int64_t* hpasize)
{
    int error;
    u_int cmd, is48bit;
    u_int8_t protocol;

    is48bit = parm->support.command2 & ATA_SUPPORT_ADDRESS48;
    protocol = AP_PROTO_NON_DATA;

    if (is48bit) {
        cmd = ATA_READ_NATIVE_MAX_ADDRESS48;
        protocol |= AP_EXTEND;
    }
    else {
        cmd = ATA_READ_NATIVE_MAX_ADDRESS;
    }

    error = ata_do_cmd(device,
        ccb,
        retry_count,
        /*flags*/ CAM_DIR_NONE,
        /*protocol*/ protocol,
        /*ata_flags*/ AP_FLAG_CHK_COND,
        /*tag_action*/ MSG_SIMPLE_Q_TAG,
        /*command*/ cmd,
        /*features*/ 0,
        /*lba*/ 0,
        /*sector_count*/ 0,
        /*data_ptr*/ NULL,
        /*dxfer_len*/ 0,
        timeout ? timeout : 5000,
        is48bit);

    if (error)
        return (error);

    return atahpa_proc_resp(device, ccb, is48bit, hpasize);
}

static int
ata_do_28bit_cmd(struct cam_device* device, union ccb* ccb, int retries,
    u_int32_t flags, u_int8_t protocol, u_int8_t tag_action,
    u_int8_t command, u_int8_t features, u_int32_t lba,
    u_int8_t sector_count, u_int8_t* data_ptr, u_int16_t dxfer_len,
    int timeout, int quiet)
{
    switch (ata_try_pass_16(device)) {
    case -1:
        return (1);
    case 1:
        /* Try using SCSI Passthrough */
        return ata_do_pass_16(device, ccb, retries, flags, protocol,
            0, tag_action, command, features, lba,
            sector_count, data_ptr, dxfer_len,
            timeout, quiet);
    }

    CCB_CLEAR_ALL_EXCEPT_HDR(&ccb->ataio);
    cam_fill_ataio(&ccb->ataio,
        retries,
        NULL,
        flags,
        tag_action,
        data_ptr,
        dxfer_len,
        timeout);

    ata_28bit_cmd(&ccb->ataio, command, features, lba, sector_count);
    return ata_cam_send(device, ccb, quiet);
}

static int
ata_do_cmd(struct cam_device* device, union ccb* ccb, int retries,
    u_int32_t flags, u_int8_t protocol, u_int8_t ata_flags,
    u_int8_t tag_action, u_int8_t command, u_int8_t features,
    u_int64_t lba, u_int8_t sector_count, u_int8_t* data_ptr,
    u_int16_t dxfer_len, int timeout, int force48bit)
{
    int retval;

    retval = ata_try_pass_16(device);
    if (retval == -1)
        return (1);

    if (retval == 1) {
        int error;

        /* Try using SCSI Passthrough */
        error = ata_do_pass_16(device, ccb, retries, flags, protocol,
            ata_flags, tag_action, command, features,
            lba, sector_count, data_ptr, dxfer_len,
            timeout, 0);

        if (ata_flags & AP_FLAG_CHK_COND) {
            /* Decode ata_res from sense data */
            struct ata_res_pass16* res_pass16;
            struct ata_res* res;
            u_int i;
            u_int16_t* ptr;

            /* sense_data is 4 byte aligned */
            ptr = (uint16_t*)(uintptr_t)&ccb->csio.sense_data;
            for (i = 0; i < sizeof(*res_pass16) / 2; i++)
                ptr[i] = le16toh(ptr[i]);

            /* sense_data is 4 byte aligned */
            res_pass16 = (struct ata_res_pass16*)(uintptr_t)&ccb->csio.sense_data;
            res = &ccb->ataio.res;
            res->flags = res_pass16->flags;
            res->status = res_pass16->status;
            res->error = res_pass16->error;
            res->lba_low = res_pass16->lba_low;
            res->lba_mid = res_pass16->lba_mid;
            res->lba_high = res_pass16->lba_high;
            res->device = res_pass16->device;
            res->lba_low_exp = res_pass16->lba_low_exp;
            res->lba_mid_exp = res_pass16->lba_mid_exp;
            res->lba_high_exp = res_pass16->lba_high_exp;
            res->sector_count = res_pass16->sector_count;
            res->sector_count_exp = res_pass16->sector_count_exp;
        }

        return (error);
    }

    CCB_CLEAR_ALL_EXCEPT_HDR(&ccb->ataio);
    cam_fill_ataio(&ccb->ataio,
        retries,
        NULL,
        flags,
        tag_action,
        data_ptr,
        dxfer_len,
        timeout);

    if (force48bit || lba > ATA_MAX_28BIT_LBA)
        ata_48bit_cmd(&ccb->ataio, command, features, lba, sector_count);
    else
        ata_28bit_cmd(&ccb->ataio, command, features, lba, sector_count);

    if (ata_flags & AP_FLAG_CHK_COND)
        ccb->ataio.cmd.flags |= CAM_ATAIO_NEEDRESULT;

    return ata_cam_send(device, ccb, 0);
}

static int
atahpa_proc_resp(struct cam_device* device, union ccb* ccb,
    int is48bit, u_int64_t* hpasize)
{
    struct ata_res* res;

    res = &ccb->ataio.res;
    if (res->status & ATA_STATUS_ERROR) {
        if (res->error & ATA_ERROR_ID_NOT_FOUND) {
            warnx("Max address has already been set since "
                  "last power-on or hardware reset");
        }

        return (1);
    }

    if (hpasize != NULL) {
        if (is48bit) {
            *hpasize = (((u_int64_t)((res->lba_high_exp << 16) | (res->lba_mid_exp << 8) | res->lba_low_exp) << 24) | ((res->lba_high << 16) | (res->lba_mid << 8) | res->lba_low)) + 1;
        }
        else {
            *hpasize = (((res->device & 0x0f) << 24) | (res->lba_high << 16) | (res->lba_mid << 8) | res->lba_low) + 1;
        }
    }

    return (0);
}

static int
ata_try_pass_16(struct cam_device* device)
{
    struct ccb_pathinq cpi;

    if (get_cpi(device, &cpi) != 0) {
        warnx("couldn't get CPI");
        return (-1);
    }

    if (cpi.protocol == PROTO_SCSI) {
        /* possibly compatible with pass_16 */
        return (1);
    }

    /* likely not compatible with pass_16 */
    return (0);
}

static int
ata_do_pass_16(struct cam_device* device, union ccb* ccb, int retries,
    u_int32_t flags, u_int8_t protocol, u_int8_t ata_flags,
    u_int8_t tag_action, u_int8_t command, u_int8_t features,
    u_int64_t lba, u_int8_t sector_count, u_int8_t* data_ptr,
    u_int16_t dxfer_len, int timeout, int quiet)
{
    if (data_ptr != NULL) {
        ata_flags |= AP_FLAG_BYT_BLOK_BYTES | AP_FLAG_TLEN_SECT_CNT;
        if (flags & CAM_DIR_OUT)
            ata_flags |= AP_FLAG_TDIR_TO_DEV;
        else
            ata_flags |= AP_FLAG_TDIR_FROM_DEV;
    }
    else {
        ata_flags |= AP_FLAG_TLEN_NO_DATA;
    }

    CCB_CLEAR_ALL_EXCEPT_HDR(&ccb->csio);

    scsi_ata_pass_16(&ccb->csio,
        retries,
        NULL,
        flags,
        tag_action,
        protocol,
        ata_flags,
        features,
        sector_count,
        lba,
        command,
        /*control*/ 0,
        data_ptr,
        dxfer_len,
        /*sense_len*/ SSD_FULL_SIZE,
        timeout);

    return scsi_cam_pass_16_send(device, ccb, quiet);
}

static int
ata_cam_send(struct cam_device* device, union ccb* ccb, int quiet)
{
    /* Disable freezing the device queue */
    ccb->ccb_h.flags |= CAM_DEV_QFRZDIS;

    //     if (arglist & CAM_ARG_ERR_RECOVER)
    //         ccb->ccb_h.flags |= CAM_PASS_ERR_RECOVER;

    if (cam_send_ccb(device, ccb) < 0) {
        return (1);
    }

    if ((ccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP) {
        return (1);
    }

    return (0);
}

static int
scsiinquiry(struct cam_device* device, int task_attr,
    int retry_count, int* result)
{
    union ccb* ccb;
    struct scsi_inquiry_data* inq_buf;
    int error = 0;

    ccb = cam_getccb(device);

    if (ccb == NULL) {
        warnx("couldn't allocate CCB");
        return (1);
    }

    /* cam_getccb cleans up the header, caller has to zero the payload */
    CCB_CLEAR_ALL_EXCEPT_HDR(&ccb->csio);

    inq_buf = (struct scsi_inquiry_data*)malloc(
        sizeof(struct scsi_inquiry_data));

    if (inq_buf == NULL) {
        cam_freeccb(ccb);
        warnx("can't malloc memory for inquiry\n");
        return (1);
    }
    bzero(inq_buf, sizeof(*inq_buf));

    /*
    * Note that although the size of the inquiry buffer is the full
    * 256 bytes specified in the SCSI spec, we only tell the device
    * that we have allocated SHORT_INQUIRY_LENGTH bytes.  There are
    * two reasons for this:
    *
    *  - The SCSI spec says that when a length field is only 1 byte,
    *    a value of 0 will be interpreted as 256.  Therefore
    *    scsi_inquiry() will convert an inq_len (which is passed in as
    *    a u_int32_t, but the field in the CDB is only 1 byte) of 256
    *    to 0.  Evidently, very few devices meet the spec in that
    *    regard.  Some devices, like many Seagate disks, take the 0 as
    *    0, and don't return any data.  One Pioneer DVD-R drive
    *    returns more data than the command asked for.
    *
    *    So, since there are numerous devices that just don't work
    *    right with the full inquiry size, we don't send the full size.
    *
    *  - The second reason not to use the full inquiry data length is
    *    that we don't need it here.  The only reason we issue a
    *    standard inquiry is to get the vendor name, device name,
    *    and revision so scsi_print_inquiry() can print them.
    *
    * If, at some point in the future, more inquiry data is needed for
    * some reason, this code should use a procedure similar to the
    * probe code.  i.e., issue a short inquiry, and determine from
    * the additional length passed back from the device how much
    * inquiry data the device supports.  Once the amount the device
    * supports is determined, issue an inquiry for that amount and no
    * more.
    *
    * KDM, 2/18/2000
    */
    scsi_inquiry(&ccb->csio,
        /* retries */ retry_count,
        /* cbfcnp */ NULL,
        /* tag_action */ task_attr,
        /* inq_buf */ (u_int8_t*)inq_buf,
        /* inq_len */ SHORT_INQUIRY_LENGTH,
        /* evpd */ 0,
        /* page_code */ 0,
        /* sense_len */ SSD_FULL_SIZE,
        /* timeout */ 5000);

    /* Disable freezing the device queue */
    ccb->ccb_h.flags |= CAM_DEV_QFRZDIS;

    //     if (arglist & CAM_ARG_ERR_RECOVER)
    //         ccb->ccb_h.flags |= CAM_PASS_ERR_RECOVER;

    if (cam_send_ccb(device, ccb) < 0) {
        perror("error sending SCSI inquiry");

        cam_freeccb(ccb);
        return (1);
    }

    if ((ccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP) {
        error = 1;
    }

    cam_freeccb(ccb);

    if (error != 0) {
        free(inq_buf);
        return (error);
    }

    *result = SID_IS_REMOVABLE(inq_buf);

    free(inq_buf);

    return (0);
}

static int
scsi_cam_pass_16_send(struct cam_device* device, union ccb* ccb, int quiet)
{
    struct ata_pass_16* ata_pass_16;
    struct ata_cmd ata_cmd;

    ata_pass_16 = (struct ata_pass_16*)ccb->csio.cdb_io.cdb_bytes;
    ata_cmd.command = ata_pass_16->command;
    ata_cmd.control = ata_pass_16->control;
    ata_cmd.features = ata_pass_16->features;

    /* Disable freezing the device queue */
    ccb->ccb_h.flags |= CAM_DEV_QFRZDIS;

    //     if (arglist & CAM_ARG_ERR_RECOVER)
    //         ccb->ccb_h.flags |= CAM_PASS_ERR_RECOVER;

    if (cam_send_ccb(device, ccb) < 0) {
        return (1);
    }

    if (!(ata_pass_16->flags & AP_FLAG_CHK_COND) && (ccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP) {
        return (1);
    }

    return (0);
}

/*
 * Get a path inquiry CCB for the specified device.
 */
static int
get_cpi(struct cam_device* device, struct ccb_pathinq* cpi)
{
    union ccb* ccb;
    int retval = 0;

    ccb = cam_getccb(device);
    if (ccb == NULL) {
        warnx("get_cpi: couldn't allocate CCB");
        return (1);
    }
    CCB_CLEAR_ALL_EXCEPT_HDR(&ccb->cpi);
    ccb->ccb_h.func_code = XPT_PATH_INQ;
    if (cam_send_ccb(device, ccb) < 0) {
        retval = 1;
        goto get_cpi_bailout;
    }
    if ((ccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP) {
        retval = 1;
        goto get_cpi_bailout;
    }
    bcopy(&ccb->cpi, cpi, sizeof(struct ccb_pathinq));

get_cpi_bailout:
    cam_freeccb(ccb);
    return (retval);
}

/*
 * Get a get device CCB for the specified device.
 */
static int
get_cgd(struct cam_device* device, struct ccb_getdev* cgd)
{
    union ccb* ccb;
    int retval = 0;

    ccb = cam_getccb(device);
    if (ccb == NULL) {
        warnx("get_cgd: couldn't allocate CCB");
        return (1);
    }
    CCB_CLEAR_ALL_EXCEPT_HDR(&ccb->cgd);
    ccb->ccb_h.func_code = XPT_GDEV_TYPE;
    if (cam_send_ccb(device, ccb) < 0) {
        retval = 1;
        goto get_cgd_bailout;
    }
    if ((ccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP) {
        retval = 1;
        goto get_cgd_bailout;
    }
    bcopy(&ccb->cgd, cgd, sizeof(struct ccb_getdev));

get_cgd_bailout:
    cam_freeccb(ccb);
    return (retval);
}

/*
 * Returns 1 if the device has the VPD page, 0 if it does not, and -1 on an
 * error.
 */
int dev_has_vpd_page(struct cam_device* dev, uint8_t page_id, int retry_count,
    int timeout, int verbosemode)
{
    union ccb* ccb = NULL;
    struct scsi_vpd_supported_page_list sup_pages;
    int i;
    int retval = 0;

    ccb = cam_getccb(dev);
    if (ccb == NULL) {
        warnx("Unable to allocate CCB");
        retval = -1;
        goto bailout;
    }

    /* cam_getccb cleans up the header, caller has to zero the payload */
    CCB_CLEAR_ALL_EXCEPT_HDR(&ccb->csio);

    bzero(&sup_pages, sizeof(sup_pages));

    scsi_inquiry(&ccb->csio,
        /*retries*/ retry_count,
        /*cbfcnp*/ NULL,
        /* tag_action */ MSG_SIMPLE_Q_TAG,
        /* inq_buf */ (u_int8_t*)&sup_pages,
        /* inq_len */ sizeof(sup_pages),
        /* evpd */ 1,
        /* page_code */ SVPD_SUPPORTED_PAGE_LIST,
        /* sense_len */ SSD_FULL_SIZE,
        /* timeout */ timeout ? timeout : 5000);

    /* Disable freezing the device queue */
    ccb->ccb_h.flags |= CAM_DEV_QFRZDIS;

    if (retry_count != 0)
        ccb->ccb_h.flags |= CAM_PASS_ERR_RECOVER;

    if (cam_send_ccb(dev, ccb) < 0) {
        cam_freeccb(ccb);
        ccb = NULL;
        retval = -1;
        goto bailout;
    }

    if ((ccb->ccb_h.status & CAM_STATUS_MASK) != CAM_REQ_CMP) {
        if (verbosemode != 0)
            cam_error_print(dev, ccb, CAM_ESF_ALL,
                CAM_EPF_ALL, stderr);
        retval = -1;
        goto bailout;
    }

    for (i = 0; i < sup_pages.length; i++) {
        if (sup_pages.list[i] == page_id) {
            retval = 1;
            goto bailout;
        }
    }
bailout:
    if (ccb != NULL)
        cam_freeccb(ccb);

    return (retval);
}

#if __FreeBSD_version < 1300000
void ata_param_fixup(struct ata_params* ident_buf)
{
    int16_t* ptr;

    for (ptr = (int16_t*)ident_buf;
         ptr < (int16_t*)ident_buf + sizeof(struct ata_params) / 2; ptr++) {
        *ptr = le16toh(*ptr);
    }
    if (strncmp(ident_buf->model, "FX", 2) && strncmp(ident_buf->model, "NEC", 3) && strncmp(ident_buf->model, "Pioneer", 7) && strncmp(ident_buf->model, "SHARP", 5)) {
        ata_bswap(ident_buf->model, sizeof(ident_buf->model));
        ata_bswap(ident_buf->revision, sizeof(ident_buf->revision));
        ata_bswap(ident_buf->serial, sizeof(ident_buf->serial));
    }
    ata_btrim(ident_buf->model, sizeof(ident_buf->model));
    ata_bpack(ident_buf->model, ident_buf->model, sizeof(ident_buf->model));
    ata_btrim(ident_buf->revision, sizeof(ident_buf->revision));
    ata_bpack(ident_buf->revision, ident_buf->revision, sizeof(ident_buf->revision));
    ata_btrim(ident_buf->serial, sizeof(ident_buf->serial));
    ata_bpack(ident_buf->serial, ident_buf->serial, sizeof(ident_buf->serial));
}
#endif
