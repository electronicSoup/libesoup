/**
 * @file libesoup/boards/cinnamonBun/pic24FJ/board.c
 *
 * @author John Whitmore
 *
 * Board initialistaion code for the PIC24 based cinnamonBun device.
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

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)

#include "libesoup/errno.h"
#ifdef SYS_SPI_BUS
#include "libesoup/comms/spi/spi.h"
#endif
#ifdef SYS_EEPROM
#include "libesoup/hardware/eeprom.h"
#endif

result_t board_init(void)
{
#ifdef SYS_SPI_BUS
	result_t               rc;
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
	return(0);
}
#endif //  defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
