/**
 *
 * \file es_lib/utils/eeprom.c
 *
 * General eeprom functions of the electronicSoup CAN code Library
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
 */

//#include <libpic30.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
#include "system.h"
//#include "es_lib/utils/utils.h"
//#ifdef HEARTBEAT
//#include "es_lib/timers/timer_sys.h"
//#endif

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "EEPROM"

result_t eeprom_read(UINT16 address, BYTE *data)
{
	BYTE use_address;

#ifdef EEPROM_USE_BOOT_PAGE
	if(address < EEPROM_BOOT_PAGE_SIZE) {
		use_address = (BYTE)address;
#else
	if((address + EEPROM_BOOT_PAGE_SIZE) < EEPROM_MAX_ADDRESS) {
		use_address = address + EEPROM_BOOT_PAGE_SIZE;
#endif
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
	if(address <= EEPROM_MAX_ADDRESS) {
		use_address = (BYTE)address;
#else
	if((address + EEPROM_BOOT_PAGE_SIZE) <= EEPROM_MAX_ADDRESS) {
		use_address = address + EEPROM_BOOT_PAGE_SIZE;
#endif
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

	for (loop = start_addr; loop <= EEPROM_MAX_ADDRESS; loop++) {
		asm ("CLRWDT");
		eeprom_write(loop, 0x00);
	}

	return (SUCCESS);
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
