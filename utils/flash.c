/**
 *
 * \file es_lib/utils/flash.c
 *
 * Flash functions of the electronicSoup Cinnamon Bun code Library
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

#include "system.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "FLASH"

BOOL flash_page_empty(UINT32 address)
{
	UINT16 loop = 0;
	UINT16 offset;
	UINT16 highWord;
	UINT16 lowWord;

	TBLPAG = ((address & 0x7F0000)>>16);
	offset = (address & 0x00FFFF);

	for(loop = 0; loop < 512; loop++) {
		asm("tblrdh.w [%1], %0" : "=r"(highWord) : "r"(offset));
		asm("tblrdl.w [%1], %0" : "=r"(lowWord) : "r"(offset));

		offset = offset + 2;

		if (highWord != 0xff || lowWord != 0xffff) {
			return (FALSE);
		}
	}
	return(TRUE);
}

void flash_erase(UINT32 address)
{
	unsigned int offset;

	LOG_D("erasePage 0x%lx\n\r", address);

        if(  ((address & 0x3ff) != 0x00)
           ||((address < FLASH_FIRMWARE_START_ADDRESS) && (address != APP_HANDLE_PAGE))) {
		LOG_E("Invalid address for Erase!\n\r");
		return;
        }
	TBLPAG = ((address & 0x7F0000)>>16);
	offset = (address & 0x00FFFF);
        
	// Initialize lower word of address
	__builtin_tblwtl(offset, 0x0000); // Set base address of erase block
                                          // with dummy latch write
	NVMCON = 0x4042; // Initialize NVMCON
	asm("DISI #5");
	__builtin_write_NVM();
}

void flash_write(UINT32 address, BYTE *data)
{
	UINT16 highWord = 0;
	UINT16 lowWord = 0;
	unsigned int offset;
	unsigned int i;

	LOG_D("flash_write(0x%lx)\n\r", address);
	
        if((address < FLASH_FIRMWARE_START_ADDRESS) && (address != APP_HANDLE_PAGE)) {
		LOG_E("Invalid address for Erase!\n\r");
		return;
        }

	//Set up NVMCON for row programming
	NVMCON = 0x4001;

	//Set up pointer to the first memory location to be written
	TBLPAG = address >> 16;

	// Initialize PM Page Boundary SFR

	offset = address & 0xFFFF;

	// Initialize lower word of address
	//Perform TBLWT instructions to write necessary number of latches
	for (i = 0; i < FLASH_NUM_INSTRUCTION_PER_ROW; i++) {
		highWord = data[(i * 4) + 3] << 8 | data[(i * 4) + 2];
		lowWord = data[(i * 4) + 1] << 8 | data[(i * 4)];

		__builtin_tblwtl(offset, lowWord);
		// Write to address low word
		__builtin_tblwth(offset, highWord);
		// Write to upper byte
		offset = offset + 2;
		// Increment address
	}

	asm("DISI #5"); // Block all interrupts with priority < 7
	                // for next 5 instructions
	__builtin_write_NVM(); // Perform unlock sequence and set WR
}
