/**
 *
 * \file libesoup/utils/spi.h
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
#ifndef _SPI_H
#define _SPI_H

#ifdef SYS_SPI_BUS

#define SPI_ANY_CHANNEL  0xFF


struct spi_io_channel {
	enum pin_t sck;
	enum pin_t mosi;
	enum pin_t miso;
};

extern void          spi_init(void);
extern int16_t       spi_channel_init(uint8_t ch, struct spi_io_channel *);
extern int16_t       spi_write_byte(uint8_t device_id, uint8_t write);
extern int16_t       spi_device_init(uint8_t ch);
extern result_t      spi_lock(uint8_t device_id);
extern result_t      spi_unlock(uint8_t device_id);

#endif // #ifdef SYS_SPI_BUS

#endif // _SPI_H
