/**
 *
 * \file libesoup/boards/cinnamonBun/board.c
 *
 * Copyright 2018 electronicSoup Limited
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

#if defined(__dsPIC33EP256MU806__)

#include "libesoup/errno.h"

#ifdef SYS_SPI_BUS
#include "libesoup/comms/spi/spi.h"
#endif

#ifdef SYS_EEPROM
#ifndef SYS_SPI_BUS
#error libesoup_config.h should define SYS_SPI_BUS upon which SYS_EEPROM depends
#endif
#include "libesoup/hardware/eeprom.h"
#endif

#ifdef SYS_ONE_WIRE
#include "libesoup/comms/one_wire/one_wire.h"
#endif


result_t board_init(void)
{
	result_t               rc;
#ifdef SYS_ONE_WIRE
	int16_t                ow_channel;
#endif

#ifdef SYS_SPI_BUS
	uint8_t                spi_channel;	
	struct spi_io_channel  spi_io;
	
	spi_io.miso = BRD_SPI_MISO;
	spi_io.mosi = BRD_SPI_MOSI;
	spi_io.sck  = BRD_SPI_SCK;
		
	rc = spi_channel_init(SPI_ANY_CHANNEL, &spi_io);
	RC_CHECK
	spi_channel = (uint8_t)rc;
#endif

#ifdef SYS_EEPROM
	rc = eprom_init(spi_channel);
	RC_CHECK
#endif

#ifdef SYS_ONE_WIRE
	rc = one_wire_init();
	RC_CHECK
	ow_channel = rc;

	rc = one_wire_reserve(OW_ANY_CHANNEL, ONE_WIRE_PIN);
//	rc = one_wire_reserve(OW_ANY_CHANNEL, RD0);
	RC_CHECK
#endif
	return(0);
}
#endif // dsPIC33EP256MU806
