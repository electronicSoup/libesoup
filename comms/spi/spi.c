/**
 *
 * \file libesoup/utils/spi.c
 *
 * SPI Interface functions for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017 electronicSoup Limited
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
#define DEBUG_FILE TRUE
#include "libesoup/logger/serial_log.h"
static const char *TAG = "SPI";
#endif // SYS_SERIAL_LOGGING

/*
 * Check required libesoup_config.h defines are found
 */
#ifdef SYS_SERIAL_LOGGING
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

void spi_init(void)
{
	uint8_t loop;

#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("spi_init()\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
	/*
	 * short delay before init SPI
	 */
	for (loop = 0; loop < 0xff; loop++) Nop();

	/*
	 * Initialise the EEPROM Chip Select Pin
	 */
	// Todo - Check that this is defined and issue compiler error.
	EEPROM_CS_PIN_DIRECTION = OUTPUT_PIN;
	EEPROM_DeSelect

	SPI_SCK_DIRECTION = OUTPUT_PIN;
	SPI_MISO_DIRECTION = INPUT_PIN;
	SPI_MOSI_DIRECTION = OUTPUT_PIN;

	SPI_1_DATA_INPUT = SPI_MISO_PIN;

	SPI_MOSI_PIN = SPI_1_DATA_OUTPUT;
	SPI_SCK_PIN  = SPI_1_CLOCK_OUTPUT;

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
}

unsigned char spi_write_byte(unsigned char write)
{
	SPI1BUF = write;
	while (!SPI_RW_FINISHED);
	return(SPI1BUF);
}

#endif // #ifdef SYS_SPI_BUS
