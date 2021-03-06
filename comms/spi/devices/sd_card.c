/**
 * @file libesoup/core.c
 *
 * @author John Whitmore
 *
 * @brief File containing SD Card API
 *
 * SD Card SPI Protocol https://openlabpro.com/guide/interfacing-microcontrollers-with-sd-card/
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
#include "libesoup/timers/delay.h"


enum sd_cmd {
	sd_reset      = 0x00,
	sd_init       = 0x01,
	sd_cmd8       = 0x08,
	sd_block_size = 0x10,
	sd_read       = 0x11,
	sd_cmd41      = 0x29,
	sd_cmd55      = 0x37,
};

struct  __attribute__ ((packed)) sd_card_command {
	uint8_t     data[6];
};

struct spi_io_channel spi_io;
struct spi_device spi_device;

static void     init_command(struct sd_card_command *buffer, enum sd_cmd cmd);
static void     send_command(struct sd_card_command *buffer);
static result_t set_block_size(uint16_t size);

#if 0
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
#endif

#ifdef SYS_CHANGE_NOTIFICATION
void sd_card_detect(enum gpio_pin pin)
{
	LOG_D("SD Card Detect\n\r");
}
#endif

void flush(void)
{
	result_t rc;
	uint8_t flush_byte;

	// Flush
	do {
		rc = spi_write_byte(&spi_device, 0xff);
		flush_byte = (uint8_t)rc;
	} while (flush_byte != 0xff);
}
result_t sd_card_init(void)
{
	result_t rc;
	uint8_t  rx_byte;
	uint8_t  flush_byte;
	uint8_t  loop;
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
	spi_io.sck  = SD_CARD_SCK;              // SCK
	spi_io.mosi = SD_CARD_MOSI,             // MOSI
	spi_io.miso = SD_CARD_MISO;             // MISO
	spi_io.cs   = INVALID_GPIO_PIN;          // CS

	spi_device.io       = spi_io;
	spi_device.bus_mode = bus_mode_1; //0;   // 2x
	spi_device.brg      = 256;

	rc = spi_reserve(&spi_device);
	RC_CHECK_PRINT_CONT("Failed to reserve\n\r");
	LOG_D("Reserved SPI Channel %d\n\r", spi_device.chan_id);

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	for (loop = 0; loop < 20; loop++) {
		rc = spi_write_byte(&spi_device, 0xff);
	}

	delay_uS(200);

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK;

	init_command(&cmd, sd_reset);
	send_command(&cmd);

	rx_byte = 0xff;
	while (rx_byte == 0xff) {
		delay_uS(200);
		rc = spi_read_byte(&spi_device);
		RC_CHECK;
		rx_byte = (uint8_t)rc;
		if (rx_byte != 0xff) {
//			serial_printf("reset rx 0x%x\n\r", rx_byte);
		}
	}
	if (rx_byte != 0x01) {
		LOG_E("Invalid Response\n\r");
		return(-ERR_INVALID_RESPONSE);
	}
	flush();

//	serial_printf("Reset complete\n\r");
	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;
	delay_mS(1);
	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK;
	// Pg 105 R7 Response 133
	init_command(&cmd, sd_cmd8);
	cmd.data[3] = 0x01;
	cmd.data[4] = 0xAA;
	cmd.data[5] = 0x87;
	send_command(&cmd);

	rx_byte = 0xff;

	rc = spi_read_byte(&spi_device);
	RC_CHECK;
	rx_byte = (uint8_t)rc;

	if (rx_byte != 0x01) {
		rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
		LOG_E("Invalid CMD8 Response\n\r");
		return(-ERR_INVALID_RESPONSE);
	}

	flush();

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);

//	serial_printf("Initialised\n\r");

	delay_mS(1);
//	serial_printf("Response 0x%x\n\r", rc);

#if SD_INIT
	rx_byte = (uint8_t)rc;

	while (rx_byte != 0x00) {
		delay_mS(100);
		rc = spi_read_byte(&spi_device);
		RC_CHECK;
		rx_byte = (uint8_t)rc;
	}
	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
#else // try cmd55 ACMD41
	// http://rjhcoding.com/avrc-sd-interface-3.php
	rx_byte = 0xff;

	while (rx_byte != 0x00) {
		init_command(&cmd, sd_cmd55);

		rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 0);
		send_command(&cmd);
		delay_uS(50);    // was 10
		flush();

		delay_uS(50);

		init_command(&cmd, sd_cmd41);
		send_command(&cmd);
		delay_uS(50);

		do {
			rc = spi_write_byte(&spi_device, 0xff);
			flush_byte = (uint8_t)rc;
			if (flush_byte == 0x00) rx_byte = 0x00;
		} while (flush_byte != 0xff);
		rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);

		if(rx_byte != 0x00) delay_mS(100);
	}
#endif

	/*
	 * Set the block size to 512
	 */
	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 0);
	set_block_size(512);
	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;
	return(rc);
}

result_t sd_card_read(uint32_t sector, uint8_t *buffer)
{
	uint8_t  retry = 0;
	result_t rc;
	uint16_t i;
//	uint8_t  new_line;
	uint8_t *ptr;
	uint8_t  rx_byte;
	uint32_t address;
	struct   sd_card_command  cmd;

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 0);

//	serial_printf("Attempt read sector 0x%lx\n\r", sector);
	ptr = buffer;
	address = sector * 512; // Block size
	init_command(&cmd, sd_read);
	cmd.data[1] = (address >> 24) & 0xff;
	cmd.data[2] = (address >> 16) & 0xff;
	cmd.data[3] = (address>> 8) & 0xff;
	cmd.data[1] = address & 0xff;

	send_command(&cmd);

	rx_byte = 0xff;
	while ((rx_byte != 0x00) && (retry < 10)) {
		retry++;
		delay_uS(50);   // was 20
		rc = spi_read_byte(&spi_device);
		RC_CHECK;
		rx_byte = (uint8_t)rc;
//		serial_printf("rx 0x%x\n\r", rx_byte);
	}
	if (retry > 9) {
		rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
		return(-ERR_GENERAL_ERROR);
	}

	do {
		rc = spi_read_byte(&spi_device);
		RC_CHECK;
		rx_byte = (uint8_t)rc;
	} while (rx_byte != 0xFE);

	for(i = 0; i < 512; i++) {
		rc = spi_read_byte(&spi_device);
		RC_CHECK;
		*buffer++ = (uint8_t)rc;
	}
	LOG_D("Read First Byte 0x%lx:0x%x Last Byte 0x%lx:0x%x\n\r", address, *ptr, address+512, ptr[511]);
	flush();
	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;
#if 0
	new_line = 0;
	ptr = buffer;

	for(i = 0; i < 32; i++) {
		serial_printf("0x%x,", *ptr++);
		new_line++;
		if(++new_line == 16) {
			serial_printf("\n\r");
			new_line = 0;
		}
	}
	serial_printf("\n\rDone\n\r");
	delay_mS(100);
#endif

	return(SUCCESS);
}

static result_t set_block_size(uint16_t size)
{
	result_t rc;
	uint8_t  rx_byte;
	struct   sd_card_command  cmd;

	init_command(&cmd, sd_block_size);
	cmd.data[1] = 0;
	cmd.data[2] = 0;
	cmd.data[3] = (uint8_t)((size >> 8) & 0xff);
	cmd.data[4] = (uint8_t)(size & 0xff);

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK;
	send_command(&cmd);

	rx_byte = 0xff;
	flush();

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;
	return(SUCCESS);
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
	rc = spi_write_byte(&spi_device, 0xff);
}
#endif // SYS_SD_CARD
