/**
 * @file libesoup/comms/spi/drivers/dsPIC33EP256GP502.c
 *
 * @author John Whitmore
 *
 * @brief Definitions for the dsPIC33EP256MU806 micro-controller
 *
 * Copyright 2017-2020 electronicSoup Limited
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#if defined(__dsPIC33EP256GP502__)
#include "libesoup_config.h"
#include "libesoup/comms/spi/spi.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/peripheral.h"
#include "libesoup/logger/serial_log.h"

/*
 * Function to initialise the Peripheral pins of spi channel
 */
result_t channel_init(struct spi_chan *chan)
{
	result_t           rc;
	struct spi_device *device;

	device = chan->active_device;

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
	switch(device->chan_id) {
#if defined(SYS_SPI1)
	case SPI_1:
		/*
		 * SPI_1 Has fixed Pins
		 *
	         * Init the SPI Config
	         */
		SPI1CON1bits.MSTEN = 1;   // Master mode
		SPI1CON1bits.PPRE  = 0x00;
		SPI1CON1bits.SPRE  = 0x05;
		SPI1CON1bits.SMP   = 1;

		switch (device->bus_mode) {
		case bus_mode_0:
			serial_printf("SPI Mode 0\n\r");
			SPI1CON1bits.CKP = 0;
			SPI1CON1bits.CKE = 0;
			break;
		case bus_mode_1:
			serial_printf("SPI Mode 1\n\r");
			SPI1CON1bits.CKP = 0;
			SPI1CON1bits.CKE = 1;
			break;
		case bus_mode_2:
			serial_printf("SPI Mode 2\n\r");
			SPI1CON1bits.CKP = 1;
			SPI1CON1bits.CKE = 0;
			break;
		case bus_mode_3:
			serial_printf("SPI Mode 3\n\r");
			SPI1CON1bits.CKP = 1;
			SPI1CON1bits.CKE = 1;
			break;
		default:
			break;
		}

		SPI1CON2 = 0x00;
		SPI1STATbits.SPIEN = 1;   // Enable the SPI
		break;
#endif // SYS_SPI1
#if defined(SYS_SPI2)
	case SPI_2:
		/*
		 * Set up periphearl pins
		 */
		if (device->io.miso != INVALID_GPIO_PIN) {
			rc = set_peripheral_input(device->io.miso);
			RC_CHECK;
			PPS_I_SPI_2_DI = rc;
//			SPI2_ENABLE_MISO;
		} else {
//			SPI2_DISABLE_MISO;
		}
		if (device->io.mosi != INVALID_GPIO_PIN) {
			rc = set_peripheral_output(device->io.mosi, PPS_O_SPI2DO);
			RC_CHECK
			SPI2_ENABLE_MOSI;
		} else {
			SPI2_DISABLE_MOSI;
		}
		if (device->io.cs != INVALID_GPIO_PIN) {
			rc = set_peripheral_output(device->io.cs, PPS_O_SPI2SS);
			RC_CHECK
			SPI2_ENABLE_SS;
		} else {
			SPI2_DISABLE_SS;
		}

		rc = set_peripheral_output(device->io.sck,  PPS_O_SPI2CLK);
		RC_CHECK


		/*
	         * Init the SPI Config
	         */
		SPI2CON1bits.MSTEN = 1;   // Master mode
		// (0,5) 615KHz
		// (0,4) 470KHz
		// (0,3) 363KHz
		// (0,2) 307KHz
		SPI2CON1bits.PPRE  = 0x00;
		SPI2CON1bits.SPRE  = 0x02;
		SPI2CON1bits.SMP   = 1;

		switch (device->bus_mode) {
		case bus_mode_0:
			serial_printf("SPI Mode 0\n\r");
			SPI2CON1bits.CKP = 0;
			SPI2CON1bits.CKE = 0;
			break;
		case bus_mode_1:
			serial_printf("SPI Mode 1\n\r");
			SPI2CON1bits.CKP = 0;
			SPI2CON1bits.CKE = 1;
			break;
		case bus_mode_2:
			serial_printf("SPI Mode 2\n\r");
			SPI2CON1bits.CKP = 1;
			SPI2CON1bits.CKE = 0;
			break;
		case bus_mode_3:
			serial_printf("SPI Mode 3\n\r");
			SPI2CON1bits.CKP = 1;
			SPI2CON1bits.CKE = 1;
			break;
		default:
			break;
		}

		SPI2CON2 = 0x00;
		SPI2STATbits.SPIEN = 1;   // Enable the SPI
		break;
#endif // SYS_SPI2
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	return(device->chan_id);
}

result_t spi_write_byte(struct spi_device *device, uint8_t write)
{
	enum spi_chan_id channel;

	channel = device->chan_id;

	switch(channel) {
#if defined(SYS_SPI1)
	case SPI_1:
		SPI1BUF = write;
		while (!SPI1STATbits.SPIRBF);
		return(SPI1BUF);
		break;
#endif // SYS_SPI1
#if defined(SYS_SPI2)
	case SPI_2:
		SPI2BUF = write;
		while (!SPI2STATbits.SPIRBF);
		return(SPI2BUF);
		break;
#endif // SYS_SPI1
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

result_t spi_read_byte(struct spi_device *device)
{
	uint8_t           clear;
	enum spi_chan_id  channel;

	channel = device->chan_id;

	switch(channel) {
#if defined(SYS_SPI1)
	case SPI_1:
		// Wait for Tx Buffer to clear
		while (SPI1STATbits.SPITBF);
		while(SPI1STATbits.SPIRBF) {
			clear = SPI1BUF;
//			serial_printf("C20x%x\n\r", clear);
		}
		SPI1BUF = 0xff;
		while (!SPI1STATbits.SPIRBF);
		return(SPI1BUF);
		break;
#endif // SYS_SPI1
#if defined(SYS_SPI2)
	case SPI_2:
		// Wait for Tx Buffer to clear
		while (SPI2STATbits.SPITBF) ;
		while(SPI2STATbits.SPIRBF) {
			clear = SPI2BUF;
//			serial_printf("C20x%x\n\r", clear);
		}
		SPI2BUF = 0xff;
		while (!SPI2STATbits.SPIRBF) ;
		return(SPI2BUF);
		break;
#endif // SYS_SPI1
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

#endif // #if defined(__dsPIC33EP256GP502__)
