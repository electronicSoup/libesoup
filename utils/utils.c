/**
 *
 * \file es_lib/utils/utils.c
 *
 * General utility functions of the electronicSoup CAN code Library
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

#include <libpic30.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "system.h"
#include "es_lib/utils/utils.h"
#ifdef HEARTBEAT
#include "es_lib/timers/timer_sys.h"
#endif
#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "UTILS"

#define NUM_INSTRUCTION_PER_ROW 64

#ifdef HEARTBEAT
void heartbeat_on(union sigval data);
void heartbeat_off(union sigval data);
#endif

unsigned char SPIWriteByte(unsigned char write);

#ifdef HEARTBEAT
void heartbeat_on(union sigval data)
{
	es_timer timer;

	Heartbeat_on();

	start_timer(HEARTBEAT_ON_TIME, heartbeat_off, data, &timer);
}
#endif

#ifdef HEARTBEAT
void heartbeat_off(union sigval data)
{
	es_timer timer;

	Heartbeat_off();

	start_timer(HEARTBEAT_OFF_TIME, heartbeat_on, data, &timer);
}
#endif

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
           ||((address < FIRMWARE_START_ADDRESS) && (address != APP_HANDLE_PAGE))) {
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
	
        if((address < FIRMWARE_START_ADDRESS) && (address != APP_HANDLE_PAGE)) {
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
	for (i = 0; i < NUM_INSTRUCTION_PER_ROW; i++) {
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

void random_init(void)
{
	u8   loop;
	u32  seed;
	u8  *data;

	data = (u8 *)&IC1TMR;  //0x146

	seed = 0;

	for(loop = 0; loop < 0x1D8; loop++) {
		asm("CLRWDT");
		seed = seed + *data;
		data++;
	}

	LOG_D("Seed 0x%lx\n\r", seed);
	srand(seed);
}

/**
 * Various string values, for example device manufacturer,
 * are stored in the Program Flash memory during project building. In
 * order to operate on these strings they must first be copies to the
 * system's RAM memory. This function carries out this function.
 *
 * This code is specific to the PIC24 Processor and C30 Compiler
 *
 * @param dest: Destination String location in RAM
 * @param source: The source String located in Program Flash Memory
 * @return Function returns the number of characters copied across.
 */
UINT16 psv_strcpy(char *dst, __prog__ char *src, UINT16 len)
{
	char *ptr = dst;
	UINT16 i = 0;

	while ((*src != 0x00) && (*src != 0xff) && (i < len - 1)) {
		*ptr++ = *src++;
		i++;
	}
	*ptr = 0x00;

	return i;
}

#ifdef HW_SPI
void spi_init(void)
{
	BYTE loop;

	LOG_D("spi_init()\n\r");

	/*
	 * short delay before init SPI
	 */
	for (loop = 0; loop < 0xff; loop++) Nop();

	/*
	 * Initialise the EEPROM Chip Select Pin
	 */
	EEPROM_CS_PIN_DIRECTION = OUTPUT_PIN;
	EEPROM_DeSelect();

	SPI_SCK_DIRECTION = OUTPUT_PIN;
	SPI_MISO_DIRECTION = INPUT_PIN;
	SPI_MOSI_DIRECTION = OUTPUT_PIN;

	/*
	 *    - SI
	 *
	 * RD2 = RP23 - SPI1 Data Input RPINR20 SDI1R<5:0>
	 */
	RPINR20bits.SDI1R = 23;
	
	/*
	 * INT
	 *
	 * RA14 = RPI36 - External Interrup
	 */
//    RPINR0bits.INT1R0 = 36;
//    RPINR0bits.INT1R = 36;

	/*
	 * OUTPUTS:
	 *  RD3 = RP22 -  SO  - SPI1 Data Output Function 7
	 *  RD1 = RP24 - SCK  - SPI1 Clock Output Function 8
	 *
	 */
	RPOR11bits.RP22R = 7;     //SPI1 SO
	RPOR12bits.RP24R = 8;     //SPI1 SCK

	/* Init the SPI Config */
	SPI1CON1bits.SSEN = 0;    // Don't use SPIs chip select it's extrnal
	SPI1CON1bits.MSTEN = 1;   // Master mode
	SPI1CON1bits.PPRE = 0x02;
	SPI1CON1bits.SPRE = 0x07;

	SPI1CON1bits.CKE = 0;
	SPI1CON1bits.CKP = 1;
	
	SPI1CON2 = 0x00;
//    SPI1STATbits.SPIROV = 0;
	SPI1STATbits.SPIEN = 1;   // Enable the SPI
}
#endif

#ifdef HW_SPI
unsigned char SPIWriteByte(unsigned char write)
{
	SPI1BUF = write;
	while (!SPI_RW_FINISHED);
	return(SPI1BUF);
}
#endif
