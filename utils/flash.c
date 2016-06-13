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
 *******************************************************************************
 *
 */

#include "system.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "FLASH"

/*
 * BOOL flash_page_empty(u32 address)
 *
 * Function simply checks that a Flash page is empty. If the address is invalid False is returned.
 *
 * Input  : u16 address - Flash page address check
 *
 * Return : True if the given FLASH Page is Valid address and empty. False otherwise.
 *
 */
BOOL flash_page_empty(u32 address)
{
	u16 loop = 0;
	u16 offset;
	u16 highWord;
	u16 lowWord;

	/*
	 * Check that the given address is on a page boundary.
	 */
        if((address & (FLASH_PAGE_SIZE - 1)) != 0x00)
		return(FALSE);

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

/*
 * result_t flash_erase_page(u32 address)
 *
 * Function Erase a Flash page.
 *
 * Input  : u16 address - Flash page to erase.
 *
 * Return : result_t  -  ERR_ADDRESS_RANGE if the passed address is incorrect.
 *                    -  SUCCESS if the page has been erased.
 */
result_t flash_erase_page(u32 address)
{
	unsigned int offset;

	LOG_D("flash_erase_page(0x%lx)\n\r", address);
#if 0
	if(address >= 0x2a800) {
		LOG_D("Ignoring that one\n\r");
		return(SUCCESS);
	}
#endif
	/*
	 * Check that the given address is on a page boundary.
	 */
        if((address & (FLASH_PAGE_SIZE - 1)) != 0x00) {
		LOG_E("ERR_ADDRESS_RANGE(0x%lx)\n\r", address);
		return (ERR_ADDRESS_RANGE);
	}

	/*
	 * Check that the given address is in an area of Firmware Address space which we can erase.
	 */
        if((address < FLASH_FIRMWARE_START_ADDRESS) && (address != FLASH_APP_HANDLE_PAGE)) {
		LOG_E("ERR_ADDRESS_RANGE(0x%lx)\n\r", address);
		return (ERR_ADDRESS_RANGE);
        }

	TBLPAG = ((address & 0x7F0000)>>16);
	offset = (address & 0x00FFFF);
        
	// Initialize lower word of address
	__builtin_tblwtl(offset, 0x0000); // Set base address of erase block
                                          // with dummy latch write
	NVMCON = 0x4042; // Initialize NVMCON
	asm("DISI #5");
	__builtin_write_NVM();

	return(SUCCESS);
}

/*
 * result_t flash_write_row(u32 address, u8 *data)
 *
 * Function Write a Row of Flash.
 *
 * Input  : u16 address - Address of the Row to write.
 *
 * Input  : u8 *data     - The row of data to be written to the Flash Page.
 *
 * Return : result_t  -  ERR_ADDRESS_RANGE if the passed address is incorrect.
 *                    -  SUCCESS if the Row has been written.
 */
result_t flash_write_row(u32 address, u8 *data)
{
	u16  highWord = 0;
	u16  lowWord = 0;
	u32 offset;
	u32 i;

	LOG_D("flash_write(0x%lx)\n\r", address);
#if 0
	if(address >= 0x2a800) {
		LOG_D("Ignoring that one\n\r");
		return(SUCCESS);
	}

	if(address == 0x18000) {
		tmp = data;
		for (i = 0; i < FLASH_NUM_INSTRUCTION_PER_ROW * 4; i++) {
			printf("0x%x,", *tmp++);
			asm ("CLRWDT");
		}
	}

	if(address == 0x2ab80) {
		tmp = data;
		for (i = 0; i < FLASH_NUM_INSTRUCTION_PER_ROW * 4; i++) {
			printf("0x%x,", *tmp++);
			asm ("CLRWDT");
		}
	}
#endif
	/*
	 * Check that the given address is on a Flash Row boundary.
	 */
        if((address & (FLASH_NUM_INSTRUCTION_PER_ROW - 1)) != 0x00)
		return (ERR_ADDRESS_RANGE);
	
	/*
	 * Check that the given address is in an area of Firmware Address space which we can erase.
	 */
        if((address < FLASH_FIRMWARE_START_ADDRESS) && (address != FLASH_APP_HANDLE_PAGE)) {
		LOG_E("flash_write ERR_ADDRESS_RANGE 0x%lx\n\r", address);
		return (ERR_ADDRESS_RANGE);
        }

	/*
	 * Set up NVMCON for row programming
	 */
	NVMCON = 0x4001;

	/*
	 * Set up pointer to the first memory location to be written
	 */
	TBLPAG = address >> 16;

	/*
	 * Initialize PM Page Boundary SFR
	 */
	offset = address & 0xFFFF;

	/*
	 * Perform TBLWT instructions to write necessary number of latches
	 */
	for (i = 0; i < FLASH_NUM_INSTRUCTION_PER_ROW; i++) {
		highWord = data[(i * 4) + 3] << 8 | data[(i * 4) + 2];
		lowWord = data[(i * 4) + 1] << 8 | data[(i * 4)];

		__builtin_tblwtl(offset, lowWord);	// Write to address low word
		__builtin_tblwth(offset, highWord);     // Write to upper byte
		offset = offset + 2;      		// Increment address
	}

	/*
	 * Block all interrupts with priority < 7 for next 5 instructions
	 */
	asm("DISI #5"); 
	__builtin_write_NVM(); // Perform unlock sequence and set WR

	return(SUCCESS);
}

/*
 * result_t flash_write_row(u32 address, u8 *data)
 *
 * Function to copy a C null terminated string from Flash into RAM memory.
 *
 * Various string values, for example device manufacturer,
 * are stored in the Program Flash memory during project building, at the
 * linker stage. In order to operate on these strings they must first be 
 * copied to the RAM memory. This function carries out this function.
 *
 * Input  : char *dst - RAM Address where string is copied to.
 *
 * Input  : char *src - Flash Address where string is copied from.
 *
 * Input/Output : u16 *length - Input as the length of the destination buffer
 *                               - Output as the number of characters written to buffer.
 *
 * Return : result_t  -  SUCCESS
 */
result_t flash_strcpy(char *dst, __prog__ char *src, u16 *length)
{
	char *dst_p = dst;

	u16 i = 0;

	while ((*src != 0x00) && (*src != 0xff) && (i < (*length) - 1)) {
		*dst_p++ = *src++;
		i++;
	}
	*dst_p = 0x00;

	*length = i;
	return (SUCCESS);
}

u16 flash_strlen(__prog__ char *src)
{
	u16 i = 0;
	__prog__ char *ptr;

	ptr = src;

	while ((*ptr != 0x00) && (*ptr != 0xff)) {
		i++;
		ptr++;
	}
	return (i);
}