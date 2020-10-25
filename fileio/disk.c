#include "libesoup_config.h"

#ifdef SYS_FAT_FS

#include "libesoup/fileio/ff14/source/ff.h"
#include "libesoup/fileio/ff14/source/diskio.h"

#define DEBUG_FILE
#define TAG "DISK"
#include "libesoup/logger/serial_log.h"

#if defined(SYS_SD_CARD)
#include "libesoup/comms/spi/devices/sd_card.h"
#endif // SYS_SD_CARD

DSTATUS disk_initialize (BYTE pdrv)
{
	result_t rc;

	LOG_D("%s\n\r", __func__);
	rc = sd_card_init();
	RC_CHECK_PRINT_CONT("Failed to init SD Card\n\r");
	return(0);
}

DSTATUS disk_status (BYTE pdrv)
{
	LOG_D("disk_status %s\n\r", __func__);
	return(0);
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
	LOG_D("%s %d sector 0x%x, count %d\n\r", __func__, pdrv, sector, count);
#ifdef SYS_SD_CARD
//	rc = sd_card_read(0x0000);
#endif
	return (RES_OK);
}

#endif // SYS_FAT_FS
