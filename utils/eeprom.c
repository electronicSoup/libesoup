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
 * The Cinnamon Bun uses an EEPROM SPI chip with 128 Bytes of memory. The max
 * address is defined in core.h as EEPROM_MAX_ADDRESS any address passed to 
 * the eeprom function will return the ERR_ADDRESS_RANGE Erorr code. 
 *
 * The eeprom address map is split into two sections with the first 32 Bytes
 * being reserved for a Bootloader. This size is specified in core.h by the
 * constant EEPROM_BOOT_PAGE_SIZE. If your project is not using a Bootloader 
 * then defining the switch EEPROM_USE_BOOT_PAGE in your system.h file will
 * ignore this restriction and allow access to the full eeprom address space.
 */

#include "system.h"

/*
 * The EEPROM chip is connected to one of the SPI buses on the PIC24FJ256GB106
 * so spi code is required.
 */
#include "es_lib/utils/spi.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "EEPROM"

/*
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
	return (ERR_ADDRESS_RANGE);
}

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

result_t eeprom_erase(UINT16 start_addr)
{
	u16 loop;
	BYTE use_address;

#ifdef EEPROM_USE_BOOT_PAGE
	use_address = (BYTE)address;
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

UINT16 eeprom_str_read(UINT16 addr, char *buffer, BYTE len)
{
	BYTE character;
	char *ptr;
	BYTE num = 0;

	LOG_D("eeprom_str_read()\n\r");

	ptr = buffer;

	eeprom_read(addr++, &character);

	while ((character != 0) && (character != 0xff) && (num < len)) {
		*ptr++ = character;
		num++;
		eeprom_read(addr++, &character);
	}
	*ptr = 0x00;
	LOG_D("eeprom_str_read() read %s\n\r", buffer);

	return (num);
}

UINT16 eeprom_str_write(UINT16 addr, char *buffer)
{
	char *ptr;
	BYTE copied = 0;

	LOG_D("eeprom_str_write()\n\r");

	ptr = buffer;

	while (*ptr) {
		LOG_D("Write to location %d value 0x%x\n\r", addr, *ptr);
		eeprom_write(addr++, *ptr++);
		copied++;
	}
	LOG_D("Write loop finished\n\r");
	eeprom_write(addr, 0x00);

	return (copied);
}
