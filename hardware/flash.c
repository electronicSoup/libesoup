/**
 * @file libesoup/hardware/flash.c
 *
 * @author John Whitmore
 *
 * @brief Flash functions of the electronicSoup Cinnamon Bun code Library
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
 */

#include "libesoup_config.h"

#ifdef SYS_FLASH_RW

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "FLASH";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

/*
 * boolean flash_page_empty(uint32_t address)
 *
 * Function simply checks that a Flash page is empty. If the address is invalid False is returned.
 *
 * Input  : uint16_t address - Flash page address check
 *
 * Return : True if the given FLASH Page is Valid address and empty. False otherwise.
 *
 */
boolean flash_page_empty(uint32_t address)
{
	uint16_t loop = 0;
	uint16_t offset;
	uint16_t highWord;
	uint16_t lowWord;

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
 * result_t flash_erase_page(uint32_t address)
 *
 * Function Erase a Flash page.
 *
 * Input  : uint16_t address - Flash page to erase.
 *
 * Return : result_t  -  -ERR_ADDRESS_RANGE if the passed address is incorrect.
 *                    -  SUCCESS if the page has been erased.
 */
result_t flash_erase_page(uint32_t address)
{
	unsigned int offset;

	LOG_D("flash_erase_page(0x%lx)\n\r", address);
	/*
	 * Check that the given address is on a page boundary.
	 */
        if((address & (FLASH_PAGE_SIZE - 1)) != 0x00) {
		LOG_E("ERR_ADDRESS_RANGE(0x%lx)\n\r", address);
		return (-ERR_ADDRESS_RANGE);
	}

	/*
	 * Check that the given address is in an area of Firmware Address space which we can erase.
	 */
        if((address < FLASH_FIRMWARE_START_ADDRESS) && (address != FLASH_APP_HANDLE_PAGE)) {
		LOG_E("ERR_ADDRESS_RANGE(0x%lx)\n\r", address);
		return (-ERR_ADDRESS_RANGE);
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
 * result_t flash_write_row(uint32_t address, uint8_t *data)
 *
 * Function Write a Row of Flash.
 *
 * Input  : uint16_t address - Address of the Row to write.
 *
 * Input  : uint8_t *data     - The row of data to be written to the Flash Page.
 *
 * Return : result_t  -  -ERR_ADDRESS_RANGE if the passed address is incorrect.
 *                    -  SUCCESS if the Row has been written.
 */
result_t flash_write_row(uint32_t address, uint8_t *data)
{
	uint16_t  highWord = 0;
	uint16_t  lowWord = 0;
	uint32_t offset;
	uint32_t i;

	LOG_D("flash_write(0x%lx)\n\r", address);
	/*
	 * Check that the given address is on a Flash Row boundary.
	 */
        if((address & (FLASH_NUM_INSTRUCTION_PER_ROW - 1)) != 0x00)
		return (-ERR_ADDRESS_RANGE);
	
	/*
	 * Check that the given address is in an area of Firmware Address space which we can erase.
	 */
        if((address < FLASH_FIRMWARE_START_ADDRESS) && (address != FLASH_APP_HANDLE_PAGE)) {
		LOG_E("flash_write ERR_ADDRESS_RANGE 0x%lx\n\r", address);
		return (-ERR_ADDRESS_RANGE);
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
 * result_t flash_write_row(uint32_t address, uint8_t *data)
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
 * Input/Output : uint16_t *length - Input as the length of the destination buffer
 *                               - Output as the number of characters written to buffer.
 *
 * Return : result_t  -  SUCCESS
 */
result_t flash_strcpy(char *dst, __prog__ char *src, uint16_t *length)
{
	char *dst_p = dst;

	uint16_t i = 0;

	while ((*src != 0x00) && (*src != 0xff) && (i < (*length) - 1)) {
		*dst_p++ = *src++;
		i++;
	}
	*dst_p = 0x00;

	*length = i;
	return (SUCCESS);
}

uint16_t flash_strlen(__prog__ char *src)
{
	uint16_t i = 0;
	__prog__ char *ptr;

	ptr = src;

	while ((*ptr != 0x00) && (*ptr != 0xff)) {
		i++;
		ptr++;
	}
	return (i);
}

#endif // #ifdef SYS_FLASH_RW
