/**
 *
 * @file libesoup/comms/spi/spi.h
 *
 * @author John Whitmore
 *
 * SPI Interface functions for the electronicSoup Cinnamon Bun
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
#ifndef _SPI_H
#define _SPI_H

#include "libesoup_config.h"

#if defined(SYS_SPI1) || defined(SYS_SPI2) || defined(SYS_SPI3)

/**
 * @def   SPI_ANY_CHANNEL
 * @brief Value passed to API function to indicate no preference in SPI Channel
 */
#define SPI_ANY_CHANNEL  0xFF

/**
 * @def spi_io_channel
 * @brief Structure to define the GPIO Pins used in an SPI Bus
 */
struct spi_io_channel {
	enum gpio_pin sck;
	enum gpio_pin mosi;
	enum gpio_pin miso;
	enum gpio_pin cs;
};

enum bus_mode {
	bus_mode_0,
	bus_mode_1,
	bus_mode_2,
	bus_mode_3
};

enum clock_polarity {
        IDLE_HIGHT,
        IDLE_LOW
};

enum spi_mode {
        MASTER,
        SLAVE
};

struct spi_device {
	enum spi_channel       channel;
	struct spi_io_channel  io;
        enum clock_polarity    clock_polarity;
        enum spi_mode          spi_mode;
	enum bus_mode          bus_mode;
	uint16_t               brg;
};

extern result_t spi_reserve(struct spi_device *device);
extern result_t spi_release(struct spi_device *device);

/**
 * @brief  Function to write a byte to an SPI device
 *
 * The device should have already locked the SPI channel before attempting
 * transmission of data.
 *
 * @param  device_id The device to transmit the byte to
 * @param  byte the data byte to be transmitted
 * @return result byte read from the device or negative on error
 */
extern result_t spi_write_byte(struct spi_device *device, uint8_t write);

#endif // #ifdef SYS_SPI_BUS

#endif // _SPI_H
