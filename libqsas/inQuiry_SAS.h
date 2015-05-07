
#include <stdlib.h>
#include <stdio.h>

/* SES define */
/* 
    SUPPORT product:
        Quanta JBOD7
	Quanta S2LR
*/

#define	MAX_NUM_DISK_JB7		24

/* General define */
#define	TRUE	0
#define	FALSE	1

typedef struct disk_simple_info {
        int			slot_num;
	/* Installed: 1, NOT Installed: 0 */
	int			disk_status;
	uint64_t		disk_sas_Address;
	char			*sys_name;
} diskSimpleInfo_t;

typedef struct ses_info {
	char			*hw_module;
	char			*sysdev_nam;
	int 			num_of_disk;
	diskSimpleInfo_t	disk_simpleInfo[MAX_NUM_DISK_JB7];
} sesInfo_t;
