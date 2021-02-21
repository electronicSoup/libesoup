/*
 * libesoup_config.h libesoup/fileio/test/libesoup_config_fat_fs.h
 *
 * file:///home/john/MPLABXProjects/I2C.X/src/libesoup/fileio/ff14/documents/doc/appnote.html
 * http://elm-chan.org/fsw/ff/00index_e.html
 */
#include "libesoup_config.h"

#ifdef FAT_FS_TEST_MAIN

#include "libesoup/fileio/ff14/source/ff.h"
#include "libesoup/fileio/ff14/source/diskio.h"
#include "libesoup/gpio/gpio.h"

#define DEBUG_FILE
#define TAG "MAIN"
#include "libesoup/logger/serial_log.h"
//#include "libesoup/comms/spi/devices/sd_card.h"
#include "libesoup/timers/sw_timers.h"

DWORD AccSize;			/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;

char Line[256];			/* Console input buffer */

FATFS FatFs;			/* File system object */
FIL File[2];			/* File objects */
BYTE Buff[4096];		/* Working buffer */

//static uint8_t  bank_a[32][128 *4];
static uint8_t  llrr_buffer[128 * 4];
static uint8_t  lrrl_buffer[128 * 4];

/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */

static void fat_error(FRESULT result)
{
	switch (result) {
	case FR_OK:
		serial_printf("FR_OK\n\r");
		break;
	case FR_DISK_ERR:
		serial_printf("FR_DISK_ERR\n\r");
		break;
	case FR_INT_ERR:
		serial_printf("FR_INT_ERR\n\r");
		break;
	case FR_NOT_READY:
		serial_printf("FR_NOT_READY\n\r");
		break;
	case FR_NO_FILE:
		serial_printf("FR_NO_FILE\n\r");
		break;
	case FR_NO_PATH:
		serial_printf("FR_NO_PATH\n\r");
		break;
	case FR_INVALID_NAME:
		serial_printf("FR_INVALID_NAME\n\r");
		break;
	case FR_DENIED:
		serial_printf("FR_DENIED\n\r");
		break;
	case FR_EXIST:
		serial_printf("FR_EXIST\n\r");
		break;
	case FR_INVALID_OBJECT:
		serial_printf("FR_INVALID_OBJECT\n\r");
		break;
	case FR_WRITE_PROTECTED:
		serial_printf("FR_WRITE_PROTECTED\n\r");
		break;
	case FR_INVALID_DRIVE:
		serial_printf("FR_INVALID_DRIVE\n\r");
		break;
	case FR_NOT_ENABLED:
		serial_printf("FR_NOT_ENABLED\n\r");
		break;
	case FR_NO_FILESYSTEM:
		serial_printf("FR_NO_FILESYSTEM\n\r");
		break;
	case FR_MKFS_ABORTED:
		serial_printf("FR_MKFS_ABORTED\n\r");
		break;
	case FR_TIMEOUT:
		serial_printf("FR_TIMEOUT\n\r");
		break;
	case FR_LOCKED:
		serial_printf("FR_LOCKED\n\r");
		break;
	case FR_NOT_ENOUGH_CORE:
		serial_printf("FR_NOT_ENOUGH_CORE\n\r");
		break;
	case FR_TOO_MANY_OPEN_FILES:
		serial_printf("FR_TOO_MANY_OPEN_FILES\n\r");
		break;
	case FR_INVALID_PARAMETER:
		serial_printf("FR_INVALID_PARAMETER\n\r");
		break;
	}
}

int main (void)
{
	result_t rc;
//	char *ptr, *ptr2;
//	long p1, p2, p3;
//	BYTE b, drv = 0;
//	const BYTE ft[] = {0,12,16,32};
//	UINT s1, s2, cnt;
//	DWORD ofs = 0, sect = 0;
	FRESULT res;
	FATFS fs;				/* Pointer to file system object */
//	DIR dir;				/* Directory object */
	FIL file;
//	DSTATUS status;
	uint16_t rcon;
	uint16_t bytes_read;

	rcon = RCON;

	libesoup_init();

	serial_printf("RXON 0x%x\n\r", rcon);
	libesoup_tasks();

	/*
	 * http://elm-chan.org/fsw/ff/doc/mount.html
	 */
//	serial_printf("Mount Drive\n\r");
//	status = disk_initialize(0);
//	serial_printf("Status %d\n\r", status);
//	while(1);
	res = f_mount(&fs, "", 1);
	if (res != FR_OK) {
		serial_printf("Result ");
		fat_error(res);
		while(1) {
			libesoup_tasks();
		}
	}

//	if (res != FR_OK) {
//		while (1);
//	}

	/*
	 * http://elm-chan.org/fsw/ff/doc/open.html
	 */
	serial_printf("Open file llrr.bin\n\r");
	res = f_open (&file, "llrr.bin", FF_FS_READONLY);
	if (res != FR_OK) {
		serial_printf("Result ");
		fat_error(res);
		while(1) {
			libesoup_tasks();
		}
	}

	/*
	 * http://elm-chan.org/fsw/ff/doc/read.html
	 */
	serial_printf("Read llrr file\n\r");
	res = f_read(&file, llrr_buffer, 128*4, &bytes_read);
	if (res != FR_OK) {
		serial_printf("Result ");
		fat_error(res);
		while(1) {
			libesoup_tasks();
		}
	}
	serial_printf("Read %d bytes\n\r", bytes_read);

	/*
	 * http://elm-chan.org/fsw/ff/doc/open.html
	 */
	serial_printf("Open file lrrl.bin\n\r");
	res = f_open (&file, "lrrl.bin", FF_FS_READONLY);
	if (res != FR_OK) {
		serial_printf("Result ");
		fat_error(res);
		while(1) {
			libesoup_tasks();
		}
	}

	/*
	 * http://elm-chan.org/fsw/ff/doc/read.html
	 */
	serial_printf("Read lrrl file\n\r");
	res = f_read(&file, lrrl_buffer, 128*4, &bytes_read);
	if (res != FR_OK) {
		serial_printf("Result ");
		fat_error(res);
		while(1) {
			libesoup_tasks();
		}
	}
	serial_printf("Read %d bytes\n\r", bytes_read);

	LOG_D("Entering main loop\n\r");
	while(1) {
		libesoup_tasks();
	}
}


#endif // FAT_FS_TEST_MAIN
