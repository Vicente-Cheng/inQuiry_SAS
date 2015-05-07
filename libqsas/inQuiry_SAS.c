/*
 * Copyright (c) 2004-2012 Douglas Gilbert.
 * Copyright (c) 2013 - Wei-Chung, Vicente, Cheng.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#include "inQuiry_SAS.h"
#include "sg_lib.h"
#include "sg_cmds_extra.h"
#include "sg_cmds_basic.h"

/*
 * This program issues SCSI SEND DIAGNOSTIC and RECEIVE DIAGNOSTIC RESULTS
 * commands tailored for SES (enclosure) devices.
 */

/* convert to WWN (String) */
#define MPTSAS_WWN_STRLEN       (16 + 2)

#define MX_ALLOC_LEN ((64 * 1024) - 1)
#define MX_ELEM_HDR 1024
#define MX_DATA_IN 2048
#define MX_JOIN_ROWS 260
#define NUM_ACTIVE_ET_AESP_ARR 32
#define MX_SERIAL_NUM_LEN 512
#define MaxLogMessage 512

/* 8 bits represents -19 C to +235 C */
/* value of 0 (would simply -20 C) reserved */
#define TEMPERAT_OFF 20

/* Send Diagnostic and Receive Diagnostic Results page codes */
#define DPC_SUPPORTED 0x0
#define DPC_CONFIGURATION 0x1
#define DPC_ENC_CONTROL 0x2
#define DPC_ENC_STATUS 0x2
#define DPC_HELP_TEXT 0x3
#define DPC_STRING 0x4
#define DPC_THRESHOLD 0x5
#define DPC_ARRAY_CONTROL 0x6   /* obsolete */
#define DPC_ARRAY_STATUS 0x6    /* obsolete */
#define DPC_ELEM_DESC 0x7
#define DPC_SHORT_ENC_STATUS 0x8
#define DPC_ENC_BUSY 0x9
#define DPC_ADD_ELEM_STATUS 0xa
#define DPC_SUBENC_HELP_TEXT 0xb
#define DPC_SUBENC_STRING 0xc
#define DPC_SUPPORTED_SES 0xd
#define DPC_DOWNLOAD_MICROCODE 0xe
#define DPC_SUBENC_NICKNAME 0xf
/* Quanta Jbod7 */
#define QUANTA_SERIAL_NUM 0x1c

/* Element Type codes */
#define UNSPECIFIED_ETC 0x0
#define DEVICE_ETC 0x1
#define POWER_SUPPLY_ETC 0x2
#define COOLING_ETC 0x3
#define TEMPERATURE_ETC 0x4
#define DOOR_LOCK_ETC 0x5
#define AUD_ALARM_ETC 0x6
#define ESC_ELECTRONICS_ETC 0x7
#define SCC_CELECTR_ETC 0x8
#define NV_CACHE_ETC 0x9
#define INV_OP_REASON_ETC 0xa
#define UI_POWER_SUPPLY_ETC 0xb
#define DISPLAY_ETC 0xc
#define KEY_PAD_ETC 0xd
#define ENCLOSURE_ETC 0xe
#define SCSI_PORT_TRAN_ETC 0xf
#define LANGUAGE_ETC 0x10
#define COMM_PORT_ETC 0x11
#define VOLT_SENSOR_ETC 0x12
#define CURR_SENSOR_ETC 0x13
#define SCSI_TPORT_ETC 0x14
#define SCSI_IPORT_ETC 0x15
#define SIMPLE_SUBENC_ETC 0x16
#define ARRAY_DEV_ETC 0x17
#define SAS_EXPANDER_ETC 0x18
#define SAS_CONNECTOR_ETC 0x19

char            logMessage[MaxLogMessage];
struct diag_page_code {
    int page_code;
    const char * desc;
};

/* Diagnostic page names, control and/or status (in and/or out) */
static struct diag_page_code dpc_arr[] = {
    {DPC_SUPPORTED, "Supported Diagnostic Pages"},  /* 0 */
    {DPC_CONFIGURATION, "Configuration (SES)"},
    {DPC_ENC_STATUS, "Enclosure Status/Control (SES)"},
    {DPC_HELP_TEXT, "Help Text (SES)"},
    {DPC_STRING, "String In/Out (SES)"},
    {DPC_THRESHOLD, "Threshold In/Out (SES)"},
    {DPC_ARRAY_STATUS, "Array Status/Control (SES, obsolete)"},
    {DPC_ELEM_DESC, "Element Descriptor (SES)"},
    {DPC_SHORT_ENC_STATUS, "Short Enclosure Status (SES)"},  /* 8 */
    {DPC_ENC_BUSY, "Enclosure Busy (SES-2)"},
    {DPC_ADD_ELEM_STATUS, "Additional Element Status (SES-2)"},
    {DPC_SUBENC_HELP_TEXT, "Subenclosure Help Text (SES-2)"},
    {DPC_SUBENC_STRING, "Subenclosure String In/Out (SES-2)"},
    {DPC_SUPPORTED_SES, "Supported SES Diagnostic Pages (SES-2)"},
    {DPC_DOWNLOAD_MICROCODE, "Download Microcode (SES-2)"},
    {DPC_SUBENC_NICKNAME, "Subenclosure Nickname (SES-2)"},
    /* Quanta specify */
    {QUANTA_SERIAL_NUM, "Quanta Jbod7 Serial Number"},
    /* ------------- */
    {0x3f, "Protocol Specific (SAS transport)"},
    {0x40, "Translate Address (SBC)"},
    {0x41, "Device Status (SBC)"},
    {-1, NULL},
};

/* Diagnostic page names, for status (or in) pages */
static struct diag_page_code in_dpc_arr[] = {
    {DPC_SUPPORTED, "Supported Diagnostic Pages"},  /* 0 */
    {DPC_CONFIGURATION, "Configuration (SES)"},
    {DPC_ENC_STATUS, "Enclosure Status (SES)"},
    {DPC_HELP_TEXT, "Help Text (SES)"},
    {DPC_STRING, "String In (SES)"},
    {DPC_THRESHOLD, "Threshold In (SES)"},
    {DPC_ARRAY_STATUS, "Array Status (SES, obsolete)"},
    {DPC_ELEM_DESC, "Element Descriptor (SES)"},
    {DPC_SHORT_ENC_STATUS, "Short Enclosure Status (SES)"},  /* 8 */
    {DPC_ENC_BUSY, "Enclosure Busy (SES-2)"},
    {DPC_ADD_ELEM_STATUS, "Additional Element Status (SES-2)"},
    {DPC_SUBENC_HELP_TEXT, "Subenclosure Help Text (SES-2)"},
    {DPC_SUBENC_STRING, "Subenclosure String In (SES-2)"},
    {DPC_SUPPORTED_SES, "Supported SES Diagnostic Pages (SES-2)"},
    {DPC_DOWNLOAD_MICROCODE, "Download Microcode (SES-2)"},
    {DPC_SUBENC_NICKNAME, "Subenclosure Nickname (SES-2)"},
    /* Quanta specify */
    {QUANTA_SERIAL_NUM, "Quanta Jbod7 Serial Number"},
    /* ------------- */
    {0x3f, "Protocol Specific (SAS transport)"},
    {0x40, "Translate Address (SBC)"},
    {0x41, "Device Status (SBC)"},
    {-1, NULL},
};

/* Diagnostic page names, for control (or out) pages */
static struct diag_page_code out_dpc_arr[] = {
    {DPC_SUPPORTED, "?? [Supported Diagnostic Pages]"},  /* 0 */
    {DPC_CONFIGURATION, "?? [Configuration (SES)]"},
    {DPC_ENC_CONTROL, "Enclosure Control (SES)"},
    {DPC_HELP_TEXT, "Help Text (SES)"},
    {DPC_STRING, "String Out (SES)"},
    {DPC_THRESHOLD, "Threshold Out (SES)"},
    {DPC_ARRAY_CONTROL, "Array Control (SES, obsolete)"},
    {DPC_ELEM_DESC, "?? [Element Descriptor (SES)]"},
    {DPC_SHORT_ENC_STATUS, "?? [Short Enclosure Status (SES)]"},  /* 8 */
    {DPC_ENC_BUSY, "?? [Enclosure Busy (SES-2)]"},
    {DPC_ADD_ELEM_STATUS, "?? [Additional Element Status (SES-2)]"},
    {DPC_SUBENC_HELP_TEXT, "?? [Subenclosure Help Text (SES-2)]"},
    {DPC_SUBENC_STRING, "Subenclosure String Out (SES-2)"},
    {DPC_SUPPORTED_SES, "?? [Supported SES Diagnostic Pages (SES-2)]"},
    {DPC_DOWNLOAD_MICROCODE, "Download Microcode (SES-2)"},
    {DPC_SUBENC_NICKNAME, "Subenclosure Nickname (SES-2)"},
    /* Quanta specify */
    {QUANTA_SERIAL_NUM, "Quanta Jbod7 Serial Number"},
    /* ------------- */
    {0x3f, "Protocol Specific (SAS transport)"},
    {0x40, "Translate Address (SBC)"},
    {0x41, "Device Status (SBC)"},
    {-1, NULL},
};

/* Names of element types used by the Enclosure Control/Status diagnostic
 * page. */

static char abbrev_arr[8];

/* Boolean array of element types of interest to the Additional Element
 * Status page. Indexed by element type (0 <= et <= 32). */
static int active_et_aesp_arr[NUM_ACTIVE_ET_AESP_ARR] = {
    0, 1 /* dev */, 0, 0,  0, 0, 0, 1 /* esce */,
    0, 0, 0, 0,  0, 0, 0, 0,
    0, 0, 0, 0,  1 /* starg */, 1 /* sinit */, 0, 1 /* arr */,
    1 /* sas exp */, 0, 0, 0,  0, 0, 0, 0,
};

/* Fetch diagnostic page name (status or in). Returns NULL if not found. */
static const char *
find_in_diag_page_desc(int page_num)
{
    const struct diag_page_code * pcdp;

    for (pcdp = in_dpc_arr; pcdp->desc; ++pcdp) {
        if (page_num == pcdp->page_code)
            return pcdp->desc;
        else if (page_num < pcdp->page_code)
            return NULL;
    }
    return NULL;
}

sesInfo_t               ses_Info;
diskSimpleInfo_t        diskSimpleInfo[MAX_NUM_DISK_JB7];

/*
 * INQUIRY page order: 0x01 -> 0x0a
 */

int ses_inq_all(sesInfo_t *ses_Info)
{   
    const char* c_page_code; /* check page code valid */
    int res, fd, m, s2lr_tg;
    unsigned char* ses_recv_0x1_buff;
    int ses_recv_0x1_buff_len;
    unsigned char* ses_recv_0x2_buff;
    int ses_recv_0x2_buff_len;
    unsigned char* ses_recv_0xa_buff;
    int ses_recv_0xa_buff_len;
    unsigned char* ses_recv_0x5_buff;
    int ses_recv_0x5_buff_len;
    unsigned char* ses_recv_0x1c_buff;
    int ses_recv_0x1c_buff_len;
    int ses_processor_num, element_id;
    char encl_vendor[9];
    char encl_product[17];
    char encl_revision[5];
    char disk_wwnstr[MPTSAS_WWN_STRLEN];
    char ses_wwnstr[MPTSAS_WWN_STRLEN];
    uint32_t tmp_sas_address_upper, tmp_sas_address_lower;
    uint64_t sas_address, attach_sas_address;
    uint64_t ses_sas_address, expander_sas_address;
    int pgc_0x01_element_cnt, pgc_0x01_element_max_cnt;

    ses_recv_0x1_buff = (unsigned char *)calloc(MX_ALLOC_LEN, 1);
    ses_recv_0x2_buff = (unsigned char *)calloc(MX_ALLOC_LEN, 1);
    ses_recv_0xa_buff = (unsigned char *)calloc(MX_ALLOC_LEN, 1);
    ses_recv_0x5_buff = (unsigned char *)calloc(MX_ALLOC_LEN, 1);
    ses_recv_0x1c_buff = (unsigned char *)calloc(MX_SERIAL_NUM_LEN, 1);

    fd = sg_cmds_open_device(ses_Info->sysdev_nam, 0 /* rw */, 0 /* verbose */);

    if (fd < 0) {
        printf("open error: %s\n", ses_Info->sysdev_nam);
        return FALSE;
    }

    /* page 0x01 Configuration (SES) */
    c_page_code = find_in_diag_page_desc(0x01);
    if(c_page_code != NULL) {
        //printf("Page code: %s\n", c_page_code);
        if(sg_ll_receive_diag(fd, 1 /* pcv */, 1 /* page code*/ ,
                              ses_recv_0x1_buff, MX_ALLOC_LEN, 1, 0)) {
            printf("%s doesn't respond in command Configuration "
                   "(SES)\n", ses_Info->sysdev_nam);
        } else {
            ses_recv_0x1_buff_len = (ses_recv_0x1_buff[2]<<8) +
                                     ses_recv_0x1_buff[3] + 4;
	    switch(ses_recv_0x1_buff_len) {
		/* S2LR side */
		case 0x50:
		    ses_Info->hw_module = "S2LR";
		    pgc_0x01_element_cnt = 48;
		    pgc_0x01_element_max_cnt = 84;
		    s2lr_tg = 1;
		    break;
		/* JBOD7 side */
		case 0xc7:
		    ses_Info->hw_module = "JBOD7";
		    pgc_0x01_element_cnt = 68;
		    pgc_0x01_element_max_cnt = 104;
		    s2lr_tg = 0;
		    break;
	    }
            for(element_id = pgc_0x01_element_cnt;
                element_id < pgc_0x01_element_max_cnt;
                element_id += 4) {
                switch(ses_recv_0x1_buff[element_id]) {
                    case ARRAY_DEV_ETC:
                        ses_Info->num_of_disk =
                            ses_recv_0x1_buff[element_id+1];
                        break;
                    case POWER_SUPPLY_ETC:
                        break;
                    case COOLING_ETC:
                        break;
                    case TEMPERATURE_ETC:
                        break;
                    case ESC_ELECTRONICS_ETC:
                        break;
                    case ENCLOSURE_ETC:
                        break;
                    case VOLT_SENSOR_ETC:
                        break;
                    case SAS_CONNECTOR_ETC:
                        break;
                    case SAS_EXPANDER_ETC:
                        break;
                }
            }
        }
    } else {
        printf("Page code 0x01 NOT fount!\n");
    }
    free(ses_recv_0x1_buff);

    /* page 0x0a Additional Element Status (SES) */
    c_page_code = find_in_diag_page_desc(0x0a);
    if(c_page_code != NULL) {
        //printf("Page code: %s\n", c_page_code);
        if(sg_ll_receive_diag(fd, 1 /* pcv */, 10 /* page code*/ ,
                              ses_recv_0xa_buff, MX_ALLOC_LEN, 1, 0)) {
            printf("%s doesn't respond in command Additional Element Status "
                   "(SES)\n", ses_Info->sysdev_nam);
        } else {
            ses_recv_0xa_buff_len = (ses_recv_0xa_buff[2] << 8) +
                                    ses_recv_0xa_buff[3] + 4;
            /* disk information */
            for (element_id = 0;
                 element_id < ses_Info->num_of_disk;
                 element_id++) {
		if (s2lr_tg == 0)
                	ses_Info->disk_simpleInfo[element_id].slot_num =
			    ses_recv_0xa_buff[15 + element_id * 36];
		else if (s2lr_tg == 1)
			ses_Info->disk_simpleInfo[element_id].slot_num =
			    ses_recv_0xa_buff[15 + element_id * 36] + 1;
		else
			ses_Info->disk_simpleInfo[element_id].slot_num =
			    ses_recv_0xa_buff[15 + element_id * 36];
                ses_Info->disk_simpleInfo[element_id].disk_status =
                    (ses_recv_0xa_buff[16 + element_id * 36] & 0x10) >> 4;
                if (ses_Info->disk_simpleInfo[element_id].disk_status == 1){
                    tmp_sas_address_upper =
                        (ses_recv_0xa_buff[28 + (element_id * 36)] << 24) +
                        (ses_recv_0xa_buff[29 + (element_id * 36)] << 16) +
                        (ses_recv_0xa_buff[30 + (element_id * 36)] << 8) +
                        ses_recv_0xa_buff[31 + (element_id * 36)];
                    tmp_sas_address_lower =
                        (ses_recv_0xa_buff[32 + (element_id * 36)] << 24) +
                        (ses_recv_0xa_buff[33 + (element_id * 36)] << 16) +
                        (ses_recv_0xa_buff[34 + (element_id * 36)] << 8) +
                        ses_recv_0xa_buff[35 + (element_id * 36)];
                    ses_Info->disk_simpleInfo[element_id].disk_sas_Address =
                        (((uint64_t)tmp_sas_address_upper) << 32) +
                        tmp_sas_address_lower;
                }
            }
        }
    } else {
        printf("Page code 0x0a NOT found!\n");
    }
    free(ses_recv_0xa_buff);

    if (sg_cmds_close_device(fd) < 0) {
        printf("close fd failed!\n");
    }
    return TRUE;
} 

