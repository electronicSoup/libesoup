/**
 *
 * \file libesoup/comms/spi/spi.c
 *
 * SPI Interface functions for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017-2018 electronicSoup Limited
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
 */
#include "libesoup_config.h"

#ifdef SYS_SPI_BUS

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "SPI";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/peripheral.h"
#include "libesoup/comms/spi/spi.h"

/*
 * Check the system configuraiton
 */
#ifndef SYS_SPI_NUM_CHANNELS
#error libesoup config file should define SYS_SPI_NUM_CHANNELS
#endif

#ifndef SYS_SPI_NUM_DEVICES
#error libesoup config file should define SYS_SPI_NUM_DEVICES
#endif

struct spi_chan {
	boolean                active;
	uint8_t                locking_device;
	struct spi_io_channel  io;
};

struct spi_chan channel[SYS_SPI_NUM_CHANNELS];

struct spi_device {
	uint8_t              channel;
};

struct spi_device device[SYS_SPI_NUM_DEVICES];

static int16_t	channel_init(uint16_t channel);

void spi_init(void)
{
	uint8_t loop;

	for(loop = 0; loop < SYS_SPI_NUM_CHANNELS; loop++) {
		channel[loop].active = FALSE;
		channel[loop].locking_device = 0xFF;
	}

	for(loop = 0; loop < SYS_SPI_NUM_DEVICES; loop++) {
		device[loop].channel = 0xFF;
	}
}

/*
 * Returns the channel number
 */
int16_t spi_channel_init(uint8_t ch, struct spi_io_channel *io)
{
	uint16_t loop;
	
	if(ch != SPI_ANY_CHANNEL) {
		if(channel[ch].active) return(-ERR_BUSY);
	} else {
		for(loop = 0; loop < SYS_SPI_NUM_CHANNELS; loop++) {
			if(channel[loop].active) continue;
			ch = loop;
			break;
		}
	}
	
	if(ch < SYS_SPI_NUM_CHANNELS) {
		channel[ch].active  = TRUE;
		channel[ch].io.miso = io->miso;
		channel[ch].io.mosi = io->mosi;
		channel[ch].io.sck  = io->sck;
		
		/*
		 * Setup pins
		 */
		gpio_set(channel[ch].io.miso, INPUT_PIN, 0);
		gpio_set(channel[ch].io.mosi, OUTPUT_PIN, 0);
		gpio_set(channel[ch].io.sck,  OUTPUT_PIN, 0);
			
		return(channel_init(ch));
	}
	return(-ERR_NO_RESOURCES);
}

int16_t spi_device_init(uint8_t spi_ch)
{
	int16_t   loop;
	
	for(loop = 0; loop < SYS_SPI_NUM_DEVICES; loop++) {
		if(device[loop].channel == 0xFF) {
			device[loop].channel = spi_ch;
			return(loop);
		}
	}
	return(-ERR_NO_RESOURCES);
}

/*
 * Local static function to initialise the Peripheral pins of spi channel
 */
static int16_t	channel_init(uint16_t ch)
{
	int16_t rc;
	switch(ch) {
	case 0:
		rc = set_peripheral_input(channel[ch].io.miso);
		RC_CHECK;
		PPS_I_SPI_1_DI = rc;
		rc = set_peripheral_output(channel[ch].io.mosi, PPS_O_SPI1DO);
		RC_CHECK
		rc = set_peripheral_output(channel[ch].io.sck,  PPS_O_SPI1CLK);
		RC_CHECK

		/*
	         * Init the SPI Config
	         */
		SPI1CON1bits.SSEN = 0;    // Don't use SPIs chip select it's extrnal
		SPI1CON1bits.MSTEN = 1;   // Master mode
		SPI1CON1bits.PPRE = 0x02;
		SPI1CON1bits.SPRE = 0x07;

		SPI1CON1bits.CKE = 0;
		SPI1CON1bits.CKP = 1;

		SPI1CON2 = 0x00;
		SPI1STATbits.SPIEN = 1;   // Enable the SPI
		break;
		
	case 1:
		rc = set_peripheral_input(channel[ch].io.miso);
		RC_CHECK;
		PPS_I_SPI3DI = rc;
		rc = set_peripheral_output(channel[ch].io.mosi, PPS_O_SPI3DO);
		RC_CHECK
		rc = set_peripheral_output(channel[ch].io.sck,  PPS_O_SPI3CLK);
		RC_CHECK

		/*
	         * Init the SPI Config
	         */
		SPI3CON1bits.SSEN = 0;    // Don't use SPIs chip select it's extrnal
		SPI3CON1bits.MSTEN = 1;   // Master mode
		SPI3CON1bits.PPRE = 0x02;
		SPI3CON1bits.SPRE = 0x07;

		SPI3CON1bits.CKE = 0;
		SPI3CON1bits.CKP = 1;

		SPI3CON2 = 0x00;
		SPI3STATbits.SPIEN = 1;   // Enable the SPI
		break;
		
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	
	return(ch);
}

result_t spi_lock(uint8_t device_id)
{
	if(channel[device[device_id].channel].locking_device == 0xff) {
		channel[device[device_id].channel].locking_device = device_id;
		return(0);
	}
	return(-ERR_BUSY);
}

result_t spi_unlock(uint8_t device_id)
{
	if(channel[device[device_id].channel].locking_device == device_id) {
		channel[device[device_id].channel].locking_device = 0xff;
		return(0);
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

int16_t spi_write_byte(uint8_t device_id, uint8_t write)
{
	if(channel[device[device_id].channel].locking_device == device_id) {
		switch(device[device_id].channel) {
		case 0:
			SPI1BUF = write;
			while (!SPI1STATbits.SPIRBF);
			return(SPI1BUF);
		}
	}
	return(-ERR_GENERAL_ERROR);
}

#endif // #ifdef SYS_SPI_BUS
