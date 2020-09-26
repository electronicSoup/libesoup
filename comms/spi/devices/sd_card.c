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
#include "libesoup/timers/hw_timers.h"
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

enum sd_cmd {
	sd_reset = 0x00,
};

struct  __attribute__ ((packed)) sd_card_command {
	uint8_t     data[6];
};

static timer_id timer;
static uint8_t  finished = 0;

struct spi_io_channel spi_io;
struct spi_device spi_device;

static void init_command(struct sd_card_command *buffer, enum sd_cmd cmd);
static void send_command(struct sd_card_command *buffer);

void toggle(timer_id timer, union sigval data)
{
	struct period period;
	static uint8_t count = 0;

	count++;
	gpio_toggle_output(SD_CARD_SCK);
	if (count >= 150) {
		hw_timer_stop(timer, &period);
		gpio_set(SD_CARD_SCK, GPIO_MODE_DIGITAL_OUTPUT, 1);
		finished = 1;
	}

}

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
	struct   timer_req request;
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

	request.period.units = uSeconds;
	request.period.duration = 5;
	request.data.sival_int = 0;
	request.type = repeat_expiry;
	request.exp_fn = toggle;

	rc = hw_timer_start(&request);
	if (rc < 0) {
		LOG_E("Failed to create HW Timer\n\r");
		return(rc);
	}
	timer = (timer_id)rc;

	while(!finished);
	LOG_D("Finished preamble\n\r");

	spi_io.sck  = SD_CARD_SCK;              // SCK
	spi_io.mosi = SD_CARD_MOSI,             // MOSI
	spi_io.miso = SD_CARD_MISO;             // MISO
	spi_io.cs   = INVALID_GPIO_PIN;          // CS

	spi_device.io       = spi_io;
	spi_device.bus_mode = bus_mode_1;   // 2x
	spi_device.brg      = 256;

	rc = spi_reserve(&spi_device);
	LOG_D("Reserved SPI Channel %d\n\r", spi_device.channel);

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK;

	init_command(&cmd, sd_reset);
	send_command(&cmd);


	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);
	rc = spi_write_byte(&spi_device, 0x00);

	serial_printf("Response 0x%x\n\r", rc);
//	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;
	return(rc);
}

static void init_command(struct sd_card_command *buffer, enum sd_cmd cmd)
{
	buffer->data[0] = (uint8_t)cmd;
	buffer->data[0] = buffer->data[0] | 0x40;
	buffer->data[5] = 0x95;

	buffer->data[1] = 0x00;
	buffer->data[2] = 0x00;
	buffer->data[3] = 0x00;
	buffer->data[4] = 0x00;
}

static void send_command(struct sd_card_command *cmd)
{
	result_t rc;
	uint8_t  i;

	for (i = 0; i < 6; i++) {
		rc = spi_write_byte(&spi_device, cmd->data[i]);
	}
}
#endif // SYS_SD_CARD
