/**
 *
 * \file es_lib/utils/spi.c
 *
 * SPI Interface functions for the electronicSoup Cinnamon Bun
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "system.h"
#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "SPI"

void spi_init(void)
{
	uint8_t loop;

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "spi_init()\n\r");
#endif
#else   // if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif  // if defined(SYS_LOG_LEVEL)

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
