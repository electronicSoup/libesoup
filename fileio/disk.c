#include "libesoup_config.h"

#include "source/ff.h"
#include "source/diskio.h"

#define DEBUG_FILE
#define TAG "FILE"
#include "libesoup/logger/serial_log.h"

DSTATUS disk_initialize (BYTE pdrv)
{
	LOG_D("%s\n\r", __func__);
	return(0);
}

DSTATUS disk_status (BYTE pdrv)
{
	LOG_D("%s\n\r", __func__);
	return(0);
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
	LOG_D("%s\n\r", __func__);
	return (RES_OK);
}
