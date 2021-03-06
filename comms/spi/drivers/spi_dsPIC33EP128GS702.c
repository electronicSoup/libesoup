/**
 * @file libesoup/comms/spi/drivers/dsPIC33EP128GS702.c
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
#if defined(__dsPIC33EP128GS702__)
#include "libesoup_config.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/peripheral.h"
#include "libesoup/comms/spi/spi.h"
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
		if (device->io.miso != INVALID_GPIO_PIN) {
			rc = set_peripheral_input(device->io.miso);
			RC_CHECK;
			PPS_I_SPI_1_DI = rc;
			SPI1_ENABLE_MISO;
		} else {
			SPI1_DISABLE_MISO;
		}
		if (device->io.mosi != INVALID_GPIO_PIN) {
			rc = set_peripheral_output(device->io.mosi, PPS_O_SPI1DO);
			RC_CHECK
			SPI1_ENABLE_MOSI;
		} else {
			SPI1_DISABLE_MOSI;
		}
		if (device->io.cs != INVALID_GPIO_PIN) {
			rc = set_peripheral_output(device->io.cs, PPS_O_SPI1SS);
			RC_CHECK
			SPI_ENABLE_CS;
		} else {
			SPI_DISABLE_CS;
		}

		rc = set_peripheral_output(device->io.sck,  PPS_O_SPI1CLK);
		RC_CHECK

		/*
	         * Init the SPI Config
	         */
		SPI1CON1Lbits.MSTEN  = 1;   // Master mode
		SPI1CON1Lbits.SMP    = 0;
//		SPI1CON1Lbits.ENHBUF = 1;   // Enable Enhanced buffer mode

		switch (device->bus_mode) {
		case bus_mode_0:
			serial_printf("SPI Mode 0\n\r");
			SPI1CON1Lbits.CKP = 0;
			SPI1CON1Lbits.CKE = 0;
			break;
		case bus_mode_1:
			serial_printf("SPI Mode 1\n\r");
			SPI1CON1Lbits.CKP = 0;
			SPI1CON1Lbits.CKE = 1;
			break;
		case bus_mode_2:
			serial_printf("SPI Mode 2\n\r");
			SPI1CON1Lbits.CKP = 1;
			SPI1CON1Lbits.CKE = 0;
			break;
		case bus_mode_3:
			serial_printf("SPI Mode 3\n\r");
			SPI1CON1Lbits.CKP = 1;
			SPI1CON1Lbits.CKE = 1;
			break;
		default:
			break;
		}

		SPI1BRGL = device->brg;

		SPI1CON2 = 0x00;
#if defined(SPI_ISR)
		SPI1IMSKL = 0xffff;   // Enable all ISRs for the moment
		SPI1IMSKH = 0xffff;   // Enable all ISRs for the moment

//		SPI1STATLbits.SPIROV = 0;  // Clear overflow
		SPI1STATL = 0;             // Clear the whole register

		IFS0bits.SPI1TXIF = 0;
		IFS0bits.SPI1RXIF = 0;

		IEC0bits.SPI1TXIE = 1;
		IEC0bits.SPI1RXIE = 1;
#endif // SPI_ISR
		SPI1STATL = 0;             // Clear the whole register
		SPI1CON1Lbits.SPIEN = 1;   // Enable the SPI
		break;
#endif // SYS_SPI1
#if defined(SYS_SPI2)
	case SPI_2:
		break;
#endif // SYS_SPI2
#if defined(SYS_SPI3)
	case SPI_3:
		break;
#endif // SYS_SPI3
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	return(device->chan_id);
}

result_t spi_write_byte(struct spi_device *device, uint8_t write)
{
	uint8_t clear;

	switch(device->chan_id) {
#if defined(SYS_SPI1)
	case SPI_1:
		while (SPI1STATLbits.SPITBF);
		SPI1BUFL = write;
		while(!SPI1STATLbits.SPIRBF);
		clear = SPI1BUFL;
		if (clear != 0xff) {
			serial_printf("C10x%x\n\r", clear);
		}
		return(clear);
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
	uint8_t          clear;

	switch(device->chan_id) {
#if defined(SYS_SPI1)
	case SPI_1:
		while (SPI1STATLbits.SPITBF);
		while(SPI1STATLbits.SPIRBF) {
			clear = SPI1BUFL;
			serial_printf("C20x%x\n\r", clear);
		}
		SPI1BUFL = 0xff;
		while(!SPI1STATLbits.SPIRBF);
		return(SPI1BUFL);
		break;
#endif // SYS_SPI1
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

#endif // #if defined(__dsPIC33EP128GS702__)
