/**
 *
 * @file libesoup/comms/spi/spi.c
 *
 * @author John Whitmore
 *
 * @brief SPI Interface functionality
 *
 * Copyright 2017-2020 electronicSoup Limited
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

#if defined(SYS_SPI1) || defined(SYS_SPI2) || defined(SYS_SPI3)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
__attribute__ ((unused)) static const char *TAG = "SPI";
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

struct spi_chan {
	boolean                active;
	struct spi_device     *active_device;
} spi_chan;

struct spi_chan channels[NUM_SPI_CHANNELS];

static result_t	channel_init(enum spi_channel ch);

result_t spi_init(void)
{
	enum spi_channel channel;

	for(channel = 0; channel < NUM_SPI_CHANNELS; channel++) {
		channels[channel].active        = FALSE;
		channels[channel].active_device = NULL;
	}
	return(SUCCESS);
}

/*
 * Returns the channel number
 */
result_t spi_reserve(struct spi_device *device)
{
	enum spi_channel channel;

	for (channel = 0; channel < NUM_SPI_CHANNELS; channel++) {
		if(channels[channel].active) {
			continue;
		} else {
			break;
		}
	}

	if (channel < NUM_SPI_CHANNELS) {
		channels[channel].active        = TRUE;
		channels[channel].active_device = device;
		device->channel                 = channel;

		return(channel_init(channel));
	}
	return(-ERR_NO_RESOURCES);
}

result_t spi_release(struct spi_device *device)
{
	enum spi_channel channel;

	channel = device->channel;
	if(channels[channel].active_device == device) {
		channels[channel].active        = FALSE;
		channels[channel].active_device = NULL;
		device->channel                 = NUM_SPI_CHANNELS;
		return (SUCCESS);
	} else {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
}

#if 0
int16_t spi_device_init(enum spi_channel spi_ch)
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
#endif // 0
/*
 * Local static function to initialise the Peripheral pins of spi channel
 */
static result_t	channel_init(enum spi_channel ch)
{
	result_t           rc;
	struct spi_device *device;

	device = channels[ch].active_device;

	/*
	 * Setup GPIO pins
	 */
	if (device->io.miso != INVALID_GPIO_PIN) {
		gpio_set(device->io.miso, GPIO_MODE_DIGITAL_INPUT, 0);
	}

	if (device->io.mosi != INVALID_GPIO_PIN) {
		gpio_set(device->io.mosi, GPIO_MODE_DIGITAL_OUTPUT, 0);
	}

	/*
	 * Have to have a clock pin.
	 */
	gpio_set(device->io.sck,  GPIO_MODE_DIGITAL_OUTPUT, 0);

	if (device->io.cs != INVALID_GPIO_PIN) {
		gpio_set(device->io.cs,  GPIO_MODE_DIGITAL_OUTPUT, 0);
	}

	/*
	 * Set up the Peripheral pins for the SPI channel
	 */
	switch(ch) {
#if defined(SYS_SPI1)
	case SPI1:
		if (device->io.miso != INVALID_GPIO_PIN) {
			rc = set_peripheral_input(device->io.miso);
			RC_CHECK;
			PPS_I_SPI_1_DI = rc;
		}
		if (device->io.mosi != INVALID_GPIO_PIN) {
			rc = set_peripheral_output(device->io.mosi, PPS_O_SPI1DO);
			RC_CHECK
		}
		if (device->io.cs != INVALID_GPIO_PIN) {
			rc = set_peripheral_output(device->io.mosi, PPS_O_SPI1SS);
			RC_CHECK
			SPI1CON1bits.SSEN  = 1;    // Use SPIs chip select!
		} else {
			SPI1CON1bits.SSEN  = 0;    // Don't use SPIs chip select it's external
		}

		rc = set_peripheral_output(device->io.sck,  PPS_O_SPI1CLK);
		RC_CHECK

		/*
	         * Init the SPI Config
	         */
		SPI1CON1bits.MSTEN = 1;   // Master mode
		SPI1CON1bits.PPRE = 0x02;
		SPI1CON1bits.SPRE = 0x07;

		SPI1CON1bits.CKE = 0;
		SPI1CON1bits.CKP = 1;

		SPI1CON2 = 0x00;
		SPI1STATbits.SPIEN = 1;   // Enable the SPI
		break;
#endif // SYS_SPI1
#if defined(SYS_SPI2)
	case SPI2:
		break;
#endif // SYS_SPI2
#if defined(SYS_SPI3)
	case SPI3:
		break;
#endif // SYS_SPI3
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	return(ch);
}

result_t spi_write_byte(struct spi_device *device, uint8_t write)
{
	enum spi_channel channel;

	channel = device->channel;

	if (channels[channel].active_device == device) {
		switch(channel) {
		case SPI1:
			SPI1BUF = write;
			while (!SPI1STATbits.SPIRBF);
			return(SPI1BUF);
			break;
		default:
			return(-ERR_BAD_INPUT_PARAMETER);
			break;
		}
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

#endif // SYS_SPI1 || SYS_SPI2 || SYS_SPI3
