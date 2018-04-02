/**
 *
 * @file libesoup/comms/spi/spi.h
 *
 * @author John Whitmore
 *
 * SPI Interface functions for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017-2018 electronicSoup Limited
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

#ifdef SYS_SPI_BUS

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
	enum pin_t sck;
	enum pin_t mosi;
	enum pin_t miso;
};

/**
 * @brief Function to initialse the SPI Data structures
 * @return result, negative on error.
 */
extern result_t      spi_init(void);

/**
 * @brief  Function to reserve an SPI channel and specify PINS used
 * @param  ch channel to be used or SPI_ANY_CHANNEL if no preference
 * @param  spi_io_channel * pointer to sturcture specifing pins used.
 * @return result (negative on error) 
 */
extern result_t      spi_channel_init(uint8_t ch, struct spi_io_channel *);

/**
 * @brief  Function to specify a device on an SPI Channel
 * @param  ch SPI Channel 
 * @return result_t 
 */
extern result_t      spi_device_init(uint8_t ch);

/**
 * @brief function to lock SPI channel for a device
 * @param device_id device locking the SPI Channel
 * @return result_t negative on error.
 */ 
extern result_t      spi_lock(uint8_t device_id);

/**
 * @brief function to unlock SPI channel for a device
 * @param device_id device unlocking the SPI Channel
 * @return result_t negative on error.
 */ 
extern result_t      spi_unlock(uint8_t device_id);

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
extern result_t      spi_write_byte(uint8_t device_id, uint8_t byte);

#endif // #ifdef SYS_SPI_BUS

#endif // _SPI_H
