#include "libesoup_config.h"

#ifdef SYS_FAT_FS

#include "libesoup/fileio/ff14/source/ff.h"
#include "libesoup/fileio/ff14/source/diskio.h"
#ifdef SYS_TEST_BUILD
#include "libesoup/gpio/gpio.h"
#endif

#define DEBUG_FILE
#define TAG "DISK"
#include "libesoup/logger/serial_log.h"

#if defined(SYS_SD_CARD)
#include "libesoup/comms/spi/devices/sd_card.h"
#endif // SYS_SD_CARD

static uint8_t initialised = FALSE;

DSTATUS disk_initialize (BYTE pdrv)
{
	result_t rc;

	LOG_D("%s\n\r", __func__);
	if(!initialised) {
		rc = sd_card_init();
		RC_CHECK_PRINT_CONT("Failed to init SD Card\n\r");
		initialised = TRUE;
	} else {
		LOG_W("Already initialised\n\r");
	}
	return(0);
}

DSTATUS disk_status (BYTE pdrv)
{
	LOG_D("disk_status %s\n\r", __func__);
	return(0);
}

/*
 * http://elm-chan.org/fsw/ff/doc/dread.html
 */
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
	result_t rc;
	LOG_D("%s drive %d sector 0x%lx, count %d\n\r", __func__, pdrv, sector, count);

	if (initialised) {
#ifdef SYS_TEST_BUILD
		rc = gpio_set(RA0, GPIO_MODE_DIGITAL_OUTPUT, 0);
#endif
#ifdef SYS_SD_CARD
		rc = sd_card_read(sector, buff);
#endif
	} else {
		LOG_W("Disk not initialised\n\r");
	}
	return (RES_OK);
}

#endif // SYS_FAT_FS
