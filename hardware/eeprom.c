/**
 *
 * \file libesoup/utils/eeprom.c
 *
 * eeprom functions of the electronicSoup Cinnamon Bun
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
 *******************************************************************************
 *
 * The Cinnamon Bun uses an SPI EEPROM chip with 128 Bytes of memory. The max
 * address is defined in core.h as EEPROM_MAX_ADDRESS any address passed to
 * the eeprom functions exceedign this maximum address will cause a returned
 * error code of ERR_ADDRESS_RANGE. The error code type is also defined in
 * core.h of libesoup.
 *
 * The eeprom address map is split into two sections with the first 32 Bytes
 * being reserved for a Bootloader. This size is specified in core.h by the
 * constant SYS_EEPROM_BOOT_PAGE_SIZE. If your project is not using a Bootloader,
 * i.e. is being programmed directly into the PIC Processor using a PicKit3, or
 * similar flash programmer, then defining the switch SYS_EEPROM_USE_BOOT_PAGE in
 * your libesoup_config.h file will ignore this restriction and allow access to the full
 * eeprom address space.
 *
 * If you do rely on a bootloader which uses this flash page then comment out
 * or remove the definition of SYS_EEPROM_USE_BOOT_PAGE. With this commented out
 * the primitive read and write functions in this file will add
 * SYS_EEPROM_BOOT_PAGE_SIZE to any given address.
 */

#include "libesoup_config.h"

#if defined(SYS_EEPROM)

#ifndef SYS_SPI_BUS
#error libesoup_config.h file should define SYS_SPI_BUS upon which EEPROM depends (see libesoup/examples)
#endif

/*
 * The EEPROM chip is connected to the first SPI bus of the PIC24FJ256GB106
 * so spi code is required.
 */
#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/spi/spi.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "EEPROM";
#include "libesoup/logger/serial_log.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

//#ifndef EEPROM_CS_PIN_DIRECTION
//#error Board file should define EEPROM_CS_PIN_DIRECTION (see libesoup/examples/libesoup_config.h)
//#endif

static uint8_t device_id = 0xff;

/*
 * EEPROM SPI Commands.
 */
#define SPI_EEPROM_STATUS_WRITE   0x01
#define SPI_EEPROM_WRITE          0x02
#define SPI_EEPROM_READ           0x03
#define SPI_EEPROM_WRITE_DISABLE  0x04
#define SPI_EEPROM_STATUS_READ    0x05
#define SPI_EEPROM_WRITE_ENABLE   0x06

#define EEPROM_STATUS_WIP         0x01

/*
 * SPI Channel shoud already be locked
 */
static result_t clear_write_in_progress(void)
{
	result_t rc;
	uint8_t  status;
	
	do {
		BRD_EEPROM_Select
		Nop();
		rc = spi_write_byte(device_id, SPI_EEPROM_STATUS_READ);
		if(rc < 0) {
			BRD_EEPROM_DeSelect
			return(rc);
		}
		rc = spi_write_byte(device_id, 0x00);
		BRD_EEPROM_DeSelect
		RC_CHECK
			
		status = (uint8_t)rc;
	} while (status & EEPROM_STATUS_WIP);

	return(0);
}

/*
 */
result_t eprom_init(uint8_t spi_chan)
{
	result_t rc;
	
	/*
	 * Initialise the EEPROM Chip Select Pin
	 */
	rc = spi_device_init(spi_chan);
	LOG_D("SPI added device %d\n\r", rc);
	RC_CHECK;
	device_id = (uint8_t)rc;
	
	rc = gpio_set(BRD_EEPROM_CS_PIN, GPIO_MODE_DIGITAL_OUTPUT, 1);
	BRD_EEPROM_DeSelect
		
	return(0);
}

/*
 * result_t eeprom_read(UINT16 address, BYTE *data)
 *
 * Function to simply read the data BYTE from an input EEPROM Address
 *
 * Input  : UINT16 address - EEPROM address to read from
 *
 * Output : BYTE *data - the read data value at given address.
 *
 * Return : ERR_ADDRESS_RANGE if the input address exceeds EEPROM_MAX_ADDRESS
 *          otherwise SUCCESS
 *
 */
result_t eeprom_read(uint16_t address)
{
	result_t rc;
	uint8_t  byte;
	
	if(address <= BRD_EEPROM_MAX_ADDRESS) {
		rc = spi_lock(device_id);
		RC_CHECK
		
		rc = clear_write_in_progress();
		RC_CHECK
			
		BRD_EEPROM_Select
		rc = spi_write_byte(device_id, SPI_EEPROM_READ);
		rc = spi_write_byte(device_id, address);
		rc = spi_write_byte(device_id, 0x00);
		BRD_EEPROM_DeSelect
		RC_CHECK
			
		byte = (uint8_t)rc;

		rc = spi_unlock(device_id);
		RC_CHECK
		return(byte);
	}
	return (-ERR_ADDRESS_RANGE);
}

/*
 * result_t eeprom_write(UINT16 address, BYTE data)
 *
 * Function to simply rite a given data BYTE to an input EEPROM Address
 *
 * Input  : UINT16 address - EEPROM address to write to
 *
 * Input  : BYTE data - the data value to write to the given address.
 *
 * Return : ERR_ADDRESS_RANGE if the input address exceeds EEPROM_MAX_ADDRESS
 *          otherwise SUCCESS
 *
 */
result_t eeprom_write(uint16_t address, uint8_t data)
{
	result_t rc;
	
	if(address <= BRD_EEPROM_MAX_ADDRESS) {
		rc = spi_lock(device_id);
		RC_CHECK
		rc = clear_write_in_progress();
		RC_CHECK
			
		BRD_EEPROM_Select
		rc = spi_write_byte(device_id, SPI_EEPROM_WRITE_ENABLE);
		BRD_EEPROM_DeSelect
		RC_CHECK
		Nop();
		BRD_EEPROM_Select

		rc = spi_write_byte(device_id, SPI_EEPROM_WRITE);
		RC_CHECK
		rc = spi_write_byte(device_id, (uint8_t)address);
		RC_CHECK
		rc = spi_write_byte(device_id, data);
		RC_CHECK
		BRD_EEPROM_DeSelect
		Nop();
		BRD_EEPROM_Select
		rc = spi_write_byte(device_id, SPI_EEPROM_WRITE_DISABLE);
		RC_CHECK
		BRD_EEPROM_DeSelect

		rc = spi_unlock(device_id);
		RC_CHECK
		return(0);
        }
	return (-ERR_ADDRESS_RANGE);
}

/*
 * result_t eeprom_erase(UINT16 start_address)
 *
 * Function to erase the whole EEPROM chip from an input start address.
 *
 * Input  : UINT16 start_address - First EEPROM address to erase
 *
 * Return : ERR_ADDRESS_RANGE if the input address exceeds EEPROM_MAX_ADDRESS
 *          otherwise SUCCESS
 *
 */
result_t eeprom_erase(uint16_t start_address)
{
	result_t rc;
	uint16_t loop;

#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_INFO))
        LOG_I("eeprom_erase(0x%x)\n\r", start_address);
#endif
	if(start_address <= BRD_EEPROM_MAX_ADDRESS) {
		for (loop = start_address; loop <= BRD_EEPROM_MAX_ADDRESS ; loop++) {
			asm ("CLRWDT");
			rc = eeprom_write(loop, 0x00);
			RC_CHECK
		}

		return (0);
	}
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
        LOG_E("eeprom_erase Address Range Error!\n\r");
#endif
	return (-ERR_ADDRESS_RANGE);
}

/*
 * result_t eeprom_str_read(UINT16 address, BYTE *buffer, UINT16 *length)
 *
 * Function to read a null terminated C String from an address in EEPROM
 *
 * Input  : UINT16 address       - start EEPROM address of null terminated string
 *
 * Output : BYTE *buffer         - Data Buffer to write string into.
 *
 * Input/Output : UINT16 *length - Inputs the size of the input buffer so function
 *                                 leaves space for null termination and will not
 *                                 overflow the buffer.
 *                                 Outputs the actual number of characters written
 *                                 to the buffer
 *
 * Return : ERR_ADDRESS_RANGE if the input address exceeds EEPROM_MAX_ADDRESS
 *          otherwise SUCCESS
 *
 */
result_t eeprom_str_read(uint16_t address, uint8_t *buffer, uint16_t length)
{
	uint8_t       character;
	uint8_t      *ptr;
	uint8_t       num_read = 0;
	result_t      rc;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("eeprom_str_read()\n\r");
#endif
	ptr = buffer;

	rc = eeprom_read(address++);
	RC_CHECK
		
	character = (uint8_t)rc;

	while(  (character != 0)
	      &&(character != 0xff)
	      &&(num_read < (length - 1))) {

		*ptr++ = character;
		num_read++;
		rc = eeprom_read(address++);
		RC_CHECK
		character = (uint8_t)rc;
	}
	*ptr = 0x00;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("eeprom_str_read() read %s\n\r", buffer);
#endif
	return (num_read);
}

/*
 * result_t  eeprom_str_write(UINT16 address, BYTE *buffer, UINT16 *length)
 *
 * Function to write a null terminated C String to an address in EEPROM
 *
 * Input  : UINT16 address - start EEPROM address to write string to
 *
 * Input  : BYTE *buffer   - String to be written to EEPROM
 *
 * Output : UINT16 *length - The number of characters written to EEPROM
 *
 * Return : ERR_ADDRESS_RANGE if the input address exceeds EEPROM_MAX_ADDRESS
 *          otherwise 0
 *
 */
result_t  eeprom_str_write(uint16_t address, uint8_t *buffer, uint16_t length)
{
	uint8_t      *ptr;
	uint16_t      wrote = 0;
	result_t      rc = 0;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("eeprom_str_write()\n\r");
#endif
	ptr = buffer;

	while ( (*ptr) && (rc >= 0) && (wrote < (length - 1))) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Write to location %d value 0x%x\n\r", address, *ptr);
#endif
		rc = eeprom_write(address++, *ptr++);
		RC_CHECK
		wrote++;
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Write loop finished\n\r");
#endif
	rc = eeprom_write(address, 0x00);
	RC_CHECK

	return (wrote);
}

#endif // defined(SYS_EEPROM)
