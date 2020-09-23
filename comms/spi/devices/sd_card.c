/**
 * @file libesoup/core.c
 *
 * @author John Whitmore
 *
 * @brief File containing SD Card API
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

#if defined(TEST)
#include "libesoup/timers/sw_timers.h"
#endif // TEST

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

struct spi_io_channel spi_io = {
	SD_CARD_SCK,              // SCK
	SD_CARD_MOSI,             // MOSI
	SD_CARD_MISO,             // MISO
	SD_CARD_SS                // CS
};

struct spi_device spi_device;

#ifdef SYS_CHANGE_NOTIFICATION
void sd_card_detect(enum gpio_pin pin)
{
	LOG_D("SD Card Detect\n\r");
}
#endif

#ifdef TEST
void test_expiry(timer_id timer, union sigval data)
{
	result_t rc;

	LOG_D("test_expiry\n\r");
	rc = spi_write_byte(&spi_device, 0x55);
	RC_CHECK_PRINT_CONT("spi write failed\n\r");

	LOG_D("Read 0x%x\n\r", rc);
}
#endif // TEST

result_t sd_card_init(void)
{
	result_t rc;
	struct timer_req timer_request;

	LOG_D("sd_card_init()\n\r");

#if 0
	rc = gpio_set(SD_CARD_SCK, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK;

	rc = gpio_set(SD_CARD_MOSI, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = gpio_set(SD_CARD_SS, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = gpio_set(SD_CARD_MISO, GPIO_MODE_DIGITAL_INPUT,0);
	RC_CHECK;
#endif
	rc = gpio_set(SD_CARD_DETECT, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK;

	rc = gpio_set(SD_CARD_WRITE_P, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK;

#ifdef SYS_CHANGE_NOTIFICATION
	rc = change_notifier_register(SD_CARD_DETECT, sd_card_detect);
#endif

	spi_device.io      = spi_io;

	rc = spi_reserve(&spi_device);
	LOG_D("Reserved SPI Channel %d\n\r", spi_device.channel);

#ifdef TEST
	timer_request.type            = single_shot_expiry;
	timer_request.exp_fn          = test_expiry;
	timer_request.period.units    = Seconds;
	timer_request.period.duration = 10;
	timer_request.data.sival_int  = 0;

	rc = sw_timer_start(&timer_request);
#endif

	return(rc);
}

#endif // SYS_SD_CARD
