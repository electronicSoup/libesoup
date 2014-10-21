/**
 *
 * \file es_lib/utils/eeprom.c
 *
 * eeprom functions of the electronicSoup Cinnamon Bun
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************
 *
 * The Cinnamon Bun uses an SPI EEPROM chip with 128 Bytes of memory. The max
 * address is defined in core.h as EEPROM_MAX_ADDRESS any address passed to 
 * the eeprom functions exceedign this maximum address will cause a returned
 * error code of ERR_ADDRESS_RANGE. The error code type is also defined in
 * core.h of es_lib.
 *
 * The eeprom address map is split into two sections with the first 32 Bytes
 * being reserved for a Bootloader. This size is specified in core.h by the
 * constant EEPROM_BOOT_PAGE_SIZE. If your project is not using a Bootloader, 
 * i.e. is being programmed directly into the PIC Processor using a PicKit3, or
 * similar flash programmer, then defining the switch EEPROM_USE_BOOT_PAGE in
 * your system.h file will ignore this restriction and allow access to the full
 * eeprom address space.
 *
 * If you do rely on a bootloader which uses this flash page then comment out
 * or remove the definition of EEPROM_USE_BOOT_PAGE. With this commented out
 * the primitive read and write functions in this file will add
 * EEPROM_BOOT_PAGE_SIZE to any given address.
 */

#include "system.h"

/*
 * The EEPROM chip is connected to the first SPI bus of the PIC24FJ256GB106
 * so spi code is required.
 */
#include "es_lib/utils/spi.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "EEPROM"

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
result_t eeprom_read(UINT16 address, BYTE *data)
{
	BYTE use_address;

#ifdef EEPROM_USE_BOOT_PAGE
	use_address = (BYTE)address;
#else
	use_address = (BYTE)(address + EEPROM_BOOT_PAGE_SIZE);
#endif
	if(use_address <= EEPROM_MAX_ADDRESS) {
		EEPROM_Select();
		Nop();
		SPIWriteByte(EEPROM_READ);
		SPIWriteByte((BYTE)use_address);
		*data = SPIWriteByte(0x00);
		EEPROM_DeSelect();
		return(SUCCESS);
	}
        LOG_E("eeprom_read Address Range Error!\n\r");
	return (ERR_ADDRESS_RANGE);
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
result_t eeprom_write(UINT16 address, BYTE data)
{
	BYTE use_address;

	LOG_D("eeprom_write(0x%x, 0x%x)\n\r", address, data);

#ifdef EEPROM_USE_BOOT_PAGE
	use_address = (BYTE)address;
#else
	use_address = (BYTE)(address + EEPROM_BOOT_PAGE_SIZE);
#endif
	if(use_address <= EEPROM_MAX_ADDRESS) {
		EEPROM_Select();
		Nop();

		SPIWriteByte(EEPROM_WRITE_ENABLE);
		EEPROM_DeSelect();

		EEPROM_Select();

		SPIWriteByte(EEPROM_WRITE);
		SPIWriteByte((BYTE)use_address);
		SPIWriteByte(data);
		EEPROM_DeSelect();

		EEPROM_Select();
		SPIWriteByte(EEPROM_WRITE_DISABLE);
		EEPROM_DeSelect();
		return(SUCCESS);
        }
        LOG_E("eeprom_write Address Range Error!\n\r");
	return (ERR_ADDRESS_RANGE);
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
result_t eeprom_erase(UINT16 start_addr)
{
	u16 loop;
	BYTE use_address;

#ifdef EEPROM_USE_BOOT_PAGE
	use_address = (BYTE)start_addr;
#else
	use_address = (BYTE)(start_addr + EEPROM_BOOT_PAGE_SIZE);
#endif
	if(use_address <= EEPROM_MAX_ADDRESS) {
		for (loop = use_address; loop <= EEPROM_MAX_ADDRESS; loop++) {
			asm ("CLRWDT");
			eeprom_write(loop, 0x00);
		}

		return (SUCCESS);
	}	
        LOG_E("eeprom_erase Address Range Error!\n\r");
	return (ERR_ADDRESS_RANGE);
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
result_t eeprom_str_read(UINT16 address, BYTE *buffer, UINT16 *length)
{
	BYTE character;
	BYTE *ptr;
	BYTE num_read = 0;
	result_t rc;

	LOG_D("eeprom_str_read()\n\r");

	ptr = buffer;

	rc = eeprom_read(address++, &character);

	while(  (rc == SUCCESS)
	      &&(character != 0)
	      &&(character != 0xff)
	      &&(num_read < *length)) {

		*ptr++ = character;
		num_read++;
		rc = eeprom_read(address++, &character);
	}
	*ptr = 0x00;
	*length = num_read;
	LOG_D("eeprom_str_read() read %s\n\r", buffer);

	return (rc);
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
 *          otherwise SUCCESS
 *
 */
result_t  eeprom_str_write(UINT16 address, BYTE *buffer, UINT16 *length)
{
	BYTE     *ptr;
	UINT16   copied = 0;
	result_t rc = SUCCESS;

	LOG_D("eeprom_str_write()\n\r");

	ptr = buffer;

	while ( (*ptr) && (rc == SUCCESS)) {
		LOG_D("Write to location %d value 0x%x\n\r", address, *ptr);
		rc = eeprom_write(address++, *ptr++);
		copied++;
	}

	if(rc == SUCCESS) {
		LOG_D("Write loop finished\n\r");
		eeprom_write(address, 0x00);
	}

	*length = copied;
	return (rc);
}
