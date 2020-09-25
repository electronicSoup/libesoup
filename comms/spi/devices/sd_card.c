/**
 * @file libesoup/core.c
 *
 * @author John Whitmore
 *
 * @brief File containing SD Card API
 *
 * SD Card SPI Protocol http://alumni.cs.ucr.edu/~amitra/sdcard/Additional/sdcard_appnote_foust.pdf
 *
 * Note to self must look into http://elm-chan.org/fsw/ff/00index_e.html
 *
 * Copyright 2020 electronicSoup Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU Lesser General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************
 *
 */
#include "libesoup_config.h"

#if defined(SYS_SD_CARD)

#define TEST

#define DEBUG_FILE
#define TAG "SD_CARD"
#include "libesoup/logger/serial_log.h"

#include "libesoup/gpio/gpio.h"
#ifdef SYS_CHANGE_NOTIFICATION
#include "libesoup/gpio/change_notification.h"
#endif

#include "libesoup/comms/spi/spi.h"

/*
 * SD Card
 * Clk RB11
 * WriteP  RB15
 * CMD RB13
 * DAT3/CD (SS) RB14
 * DAT0 RA0
 * DAT1 RA1

 */
#define SD_CARD_DETECT  RB5
#define SD_CARD_WRITE_P RB15

#define SD_CARD_SCK     RB11   // GREY    Idle High Works
#define SD_CARD_MOSI    RB13   // RED     Idle Low no activity
#define SD_CARD_SS      RB14   // ORANGE  Idle low no activity
#define SD_CARD_MISO    RA0    // YELLOW  Idle Hight no Activity

#define SD_CMD_RESET    0x00;

struct  __attribute__ ((packed)) sd_card_command {
	union cmd {
		struct cmd_byte {
			uint8_t     msbit   : 1;
			uint8_t     one     : 1;
			uint8_t     command : 6;
		};
		uint8_t ms_byte;
	};
	uint8_t     data[4];
	union crc {
		struct crc_byte {
			uint8_t     crc     : 7;
			uint8_t     lsbit   : 1;
		};
		uint8_t ls_byte;
	};
};


struct spi_io_channel spi_io;
struct spi_device spi_device;

static void init_command(struct sd_card_command *cmd);

#ifdef SYS_CHANGE_NOTIFICATION
void sd_card_detect(enum gpio_pin pin)
{
	LOG_D("SD Card Detect\n\r");
}
#endif

result_t sd_card_init(void)
{
	uint8_t  i;
	result_t rc;
	struct   sd_card_command  cmd;

	LOG_D("sd_card_init()\n\r");

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = gpio_set(SD_CARD_SCK, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = gpio_set(SD_CARD_MOSI, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = gpio_set(SD_CARD_DETECT, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK;

	rc = gpio_set(SD_CARD_WRITE_P, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK;

#ifdef SYS_CHANGE_NOTIFICATION
	rc = change_notifier_register(SD_CARD_DETECT, sd_card_detect);
#endif

	for (i = 0; i < 200; i++) {
		gpio_toggle_output(SD_CARD_SCK);
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
	}

	spi_io.sck  = SD_CARD_SCK;              // SCK
	spi_io.mosi = SD_CARD_MOSI,             // MOSI
	spi_io.miso = SD_CARD_MISO;             // MISO
	spi_io.cs   = INVALID_GPIO_PIN;          // CS

	spi_device.io      = spi_io;
	spi_device.bus_mod = bus_mode_2;

	rc = spi_reserve(&spi_device);
	LOG_D("Reserved SPI Channel %d\n\r", spi_device.channel);

	Nop();
	Nop();
	Nop();
	Nop();

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK;

	init_command(&cmd);
//	cmd command = SD_CMD_RESET;
	send_command(&cmd);

	rc = spi_write_byte(&spi_device, 0x40);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x95);

	Nop();
	Nop();
	Nop();
	Nop();

	rc = spi_write_byte(&spi_device, 0x00);

	serial_printf("Response 0x%x\n\r", rc);
	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;
	return(rc);
}

static void init_command(struct sd_card_command *cmd)
{
}

static 	send_command(struct sd_card_command *cmd);
{
	rc = spi_write_byte(&spi_device, 0x40);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x95);
}
#endif // SYS_SD_CARD
