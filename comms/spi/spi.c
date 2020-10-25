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

/*
 * Function provided by uC driver code
 */
extern result_t channel_init(struct spi_chan *);

struct spi_chan channels[NUM_SPI_CHANNELS];

//#if defined(__dsPIC33EP128GS702__)
#ifdef SYS_SPI1
void __attribute__((__interrupt__, __no_auto_psv__)) _SPI1TXInterrupt(void)
{
	IFS0bits.SPI1TXIF = 0;
	serial_printf("*SPI1_TX* H0x%x:L0x%x\n\r", SPI1STATH, SPI1STATL);
}
#endif

#ifdef SYS_SPI1
void __attribute__((__interrupt__, __no_auto_psv__)) _SPI1RXInterrupt(void)
{
	IFS0bits.SPI1RXIF = 0;
	serial_printf("*SPI1_RX* H0x%x:L0x%x\n\r", SPI1STATH, SPI1STATL);
}
#endif
//#endif // uC Selection


result_t spi_init(void)
{
	enum spi_chan_id channel;

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
	enum spi_chan_id channel;

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
		device->chan_id                 = channel;
		device->pChan                   = &channels[channel];

		return(channel_init(&channels[channel]));
	}
	return(-ERR_NO_RESOURCES);
}

result_t spi_release(struct spi_device *device)
{
	enum spi_chan_id channel;

	channel = device->chan_id;
	if(channels[channel].active_device == device) {
		channels[channel].active        = FALSE;
		channels[channel].active_device = NULL;
		device->chan_id                 = NUM_SPI_CHANNELS;
		return (SUCCESS);
	} else {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
}

#endif // SYS_SPI1 || SYS_SPI2 || SYS_SPI3
