/**
 *
 * \file es_can/utils/utils.c
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
#include "es_can/timers/timer_sys.h"
#endif
//#include "es_can/interrupts/interrupts.h"
#define DEBUG_FILE
#include "es_lib/logger/serial.h"

#if LOG_LEVEL < NO_LOGGING
#define TAG "UTILS"
#endif

/*
 * Flash addresses
 */
#define FIRMWARE_START_ADDRESS 0x8800
#define APP_HANDLE_PAGE        0x400


//#define EEPROM_ADDR_MIN 20

#if defined(PIC24FJ256GB106)
#define EEPROM_READ           0x03
#define EEPROM_WRITE          0x02
#define EEPROM_WRITE_DISABLE  0x04
#define EEPROM_WRITE_ENABLE   0x06
#define EEPROM_STATUS_READ    0x05
#define EEPROM_STATUS_WRITE   0x01
#endif

#ifdef HEARTBEAT
void heartbeat_on(union sigval data);
void heartbeat_off(union sigval data);
#endif

#if defined(PIC18F4585)
void flashTest(UINT32 address);
#endif

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

#if 0
BYTE generate_hash(char *string)
{
	BYTE loop;
	UINT16 hash = 0x00;
	DEBUG_D("generate Hash strlen %d\n\r", strlen(string));
	//http://research.cs.vt.edu/AVresearch/hashing/strings.php
	for(loop = 0; loop < strlen(string)/2; loop++) {
		hash = hash + ((string[loop * 2] << 8) | string[loop * 2 + 1]);
	}

	if(loop * 2 < strlen(string)) {
		hash = hash + string[loop * 2];
	}

	return (hash % 0xff);
}
#endif

#if defined(PIC18F4585)
BOOL flash_page_empty(UINT32 address)
{
	BYTE loop;

	//nodeLog(logDebug, TAG, "flash_page_empty(%lx)\n\r", address);
	//flashTest(address);

	// 64 Byte Boundry
	if(   ((address & 0x3f) == 0x00)
	      &&(address >= FIRST_FLASH_ADDRESS)
	      && (address < LAST_FLASH_ADDRESS)) {
		TBLPTR = address;

		for(loop = 0; loop < FLASH_PAGE_SIZE; loop++) {
			_asm TBLRDPOSTINC _endasm
				if(TABLAT != 0xff) {
					return (FALSE);
				}
		}
		return(TRUE);
	} else {
		DEBUG_D("page_empty() Address Error\n\r");
	}
	return(FALSE);
}
#elif defined(__C30__) || defined(__XC16__)
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
#endif

#if defined(PIC18F4585)
void flash_erase(UINT32 address)
{
	DEBUG_D("Flash Erase page %lx\n\r", address);
	// Page 64 Byte boundry max address
	if(   ((address & 0x3f) == 0x00)
	      &&(address >= FIRST_FLASH_ADDRESS)
	      && (address < LAST_FLASH_ADDRESS)) {
		//TODO Check page not already empty
		if(!flash_page_empty(address)) {
			TBLPTR = address;

			EECON1 = 0x94;

			// Disable interrupts
			INTCONbits.GIE = 0;

			EECON2 = 0x55;
			EECON2 = 0x0AA;

			//Start the write
			EECON1bits.WR = 1;

			// Re-enable the interrupts
			INTCONbits.GIE = 1;

			// Disable Writes again
			EECON1bits.WREN = 0;

			//nodeLog(logDebug, TAG, "After Erase:\n\r");
			//flashTest(0x5000);
		}
	} else {
		DEBUG_E("erase_page() Address Error\n\r");
	}
}
#elif defined(__C30__) || defined(__XC16__)
void flash_erase(UINT32 address)
{
	unsigned int offset;

	DEBUG_D("erasePage 0x%lx\n\r", address);

        if(  ((address & 0x3ff) != 0x00)
           ||((address < FIRMWARE_START_ADDRESS) && (address != APP_HANDLE_PAGE))) {
            DEBUG_E("Invalid address for Erase!\n\r");
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
#endif

#if defined(PIC18F4585)
void flash_write(UINT32 address, BYTE *data)
{
	BYTE loop;
#if 0
#if LOG_LEVEL <= LOG_DEBUG
	BYTE i;
	BYTE string[16 * 4];
	BYTE subStr[5];
#endif
#endif

	UINT32 localAddress = address;

	DEBUG_D("write_page(%lx)\n\r", address);

#if 0
#if LOG_LEVEL <= LOG_DEBUG
	// FLASH_PAGE_SIZE 64 = 16 * 4
	for(i = 0; i < 4; i++) {
		string[0] = 0;
		for (loop = 0; loop < 16; loop++) {
			sprintf(subStr, "0x%x ", data[(i * 16) + loop]);
			DEBUG_D("subStr %s", subStr);
			strcat(string, subStr);
			DEBUG_D("string %s", string);
		}
		DEBUG_D("%s", string);
	}
#endif
#endif
	// Page 64 Byte boundry max address
	if(   ((address & 0x3f) == 0x00)
	      &&(address >= FIRST_FLASH_ADDRESS)
	      &&(address < LAST_FLASH_ADDRESS)) {
		flash_erase(address);

		if(localAddress != address) {
			DEBUG_E("Address Corrupted\n\r");
		}
//        flashTest(address);

		TBLPTR = address;

		for(loop = 0; loop < FLASH_PAGE_SIZE; loop++) {
			TABLAT = data[loop];
			_asm TBLWTPOSTINC _endasm
		}

		EECON1 = 0x84;

		TBLPTR = address;
		// Disable interrupts
		INTCONbits.GIE = 0;

		EECON2 = 0x55;
		EECON2 = 0xAA;

		//Start the write
		EECON1bits.WR = 1;

		// Re-enable the interrupts
		INTCONbits.GIE = 1;

		// Disable Writes again
		EECON1bits.WREN = 0;

		TBLPTR = address;
		for(loop = 0; loop < FLASH_PAGE_SIZE; loop++) {
			_asm TBLRDPOSTINC _endasm
		}

		//flashTest(address);
	} else {
		DEBUG_D("Address Error\n\r");
	}
}
#elif defined(__C30__) || defined(__XC16__)
void flash_write(UINT32 address, BYTE *data)
{
#define NUM_INSTRUCTION_PER_ROW 64

	UINT16 highWord = 0;
	UINT16 lowWord = 0;
	unsigned int offset;
	unsigned int i;

        if(  ((address & 0x3ff) != 0x00)
           ||((address < FIRMWARE_START_ADDRESS) && (address != APP_HANDLE_PAGE))) {
            DEBUG_E("Invalid address for Erase!\n\r");
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
#endif

#if defined(PIC18F4585)
void flash_test(UINT32 address)
{
	BYTE loop;
	BYTE data[FLASH_PAGE_SIZE];

	for (loop = 0; loop < FLASH_PAGE_SIZE; loop++) {
		data[loop] = 0x00;
	}
	DEBUG_D("FLASH Address :0x%lx ", address);

	// 64 Byte Boundry
	if(   ((address & 0x3f) == 0x00)
	      &&(address >= FIRST_FLASH_ADDRESS)
	      && (address < LAST_FLASH_ADDRESS)) {
		EECON1 = 0x80;
		TBLPTR = address;

		for(loop = 0; loop < FLASH_PAGE_SIZE; loop++) {
			_asm TBLRDPOSTINC _endasm
//            _asm TBLRD*+ _endasm
			data[loop] = TABLAT;
		}

		for(loop = 0; loop < FLASH_PAGE_SIZE; loop++) {
			DEBUG_D("%x-", data[loop]);
		}
		DEBUG_D("\n\r");
	}
}
#endif


#ifdef EEPROM
result_t eeprom_read(UINT16 address, BYTE *data)
{
	if(address < EEPROM_MAX_ADDRESS) {
#if defined(PIC18F4585)
		EEADRH = (address >> 8) & 0xff;
		EEADR = address & 0xff;
		EECON1 = 0x00;
		EECON1 = 0x01;

		*data = EEDATA;
#elif defined(__C30__) || defined(__XC16__)
		EEPROM_Select();
		Nop();
		SPIWriteByte(EEPROM_READ);
		SPIWriteByte((BYTE)address);
		*data = SPIWriteByte(0x00);
		EEPROM_DeSelect();
#endif
		return(SUCCESS);
	}
	return (ERR_ADDRESS_RANGE);
}
#endif

#ifdef EEPROM
result_t eeprom_write(UINT16 address, BYTE data)
{
    DEBUG_D("eeprom_write(0x%x, 0x%x)\n\r", address, data);
	if(address <= EEPROM_MAX_ADDRESS) {
#if defined(PIC18F4585)
		EEADRH = (address >> 8) & 0xff;
		EEADR = address & 0xff;
		EEDATA = data;

		EECON1bits.WREN = 1;  // Enable writes

		// Disable interrupts
		disable_interrupts();
//        INTCONbits.GIE = 0;

		EECON2 = 0x55;
		EECON2 = 0xAA;

		//Start the write
		EECON1bits.WR = 1;

		// Re-enable the interrupts
		enable_interrupts();
//        INTCONbits.GIE = 1;

		EECON1bits.WREN = 0;  // Disable writes

#elif defined(__C30__) || defined(__XC16__)
		EEPROM_Select();
		Nop();

		SPIWriteByte(EEPROM_WRITE_ENABLE);
		EEPROM_DeSelect();

//        for(loop = 0; loop <10; loop++) Nop();

		EEPROM_Select();
//        Nop();

		SPIWriteByte(EEPROM_WRITE);
		SPIWriteByte((BYTE)address);
		SPIWriteByte(data);
		EEPROM_DeSelect();

//        Nop();
		EEPROM_Select();
		SPIWriteByte(EEPROM_WRITE_DISABLE);
		EEPROM_DeSelect();
#endif // C30
		return(SUCCESS);
        }
        DEBUG_E("eeprom_write Address Range Error!\n\r");
	return (ERR_ADDRESS_RANGE);
}
#endif

#ifdef EEPROM
result_t eeprom_erase(UINT16 addr)
{
    u16 loop;

    for(loop = addr; loop <= EEPROM_MAX_ADDRESS; loop++) {
        asm ("CLRWDT");
        eeprom_write(loop, 0x00);
    }

    return(SUCCESS);
}
#endif //EEPROM

#ifdef EEPROM
UINT16 eeprom_str_read(UINT16 addr, char *buffer, BYTE len)
{
    BYTE character;
    char *ptr;
    BYTE num = 0;

    DEBUG_D("eeprom_str_read()\n\r");

    ptr = buffer;

    eeprom_read(addr++, &character);

    while( (character != 0) && (character != 0xff) && (num < len) ) {
        *ptr++ = character;
        num++;
        eeprom_read(addr++, &character);
    }
    *ptr = 0x00;
    DEBUG_D("eeprom_str_read() read %s\n\r", buffer);

    return(num);
}
#endif //EEPROM

#ifdef EEPROM
UINT16 eeprom_str_write(UINT16 addr, char *buffer)
{
    char *ptr;
    BYTE copied = 0;

    DEBUG_D("eeprom_str_write()\n\r");

    ptr = buffer;

    while(*ptr) {
        DEBUG_D("Write to location %d value 0x%x\n\r", addr, *ptr);
        eeprom_write(addr++, *ptr++);
        copied++;
    }
    DEBUG_D("Write loop finished\n\r");
    eeprom_write(addr, 0x00);

    return(copied);
}
#endif //EEPROM

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

	DEBUG_D("Seed 0x%lx\n\r", seed);
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
#if 0
#if defined( __C30__ ) || defined(__XC16__)
UINT16 strcpypgmtoram(char *dest, const char *source, UINT16 len)
{
#if 0
	BOOL exit = FALSE;
	UINT16 offset;
	UINT16 number_copied = 0;
	volatile UINT16 high_word;
	volatile UINT16 low_word;
        char single_char;
#endif
	char *ptr = dest;

        _strncpy_p2d16(ptr, (_prog_addressT)source, (unsigned int)len);

        return(strlen(dest));

#if 0
        do {
        _strncpy_p2d16((void *)&single_char, (_prog_addressT)source, 3);
            *ptr++ = single_char;
            source++;
            DEBUG_D("Copied Character %c\n\r", single_char);
//            number_copied++;
        } while(single_char != '\0');

        return(strlen(dest));
#endif

#if 0
	TBLPAG = ((((UINT32)source) & 0x7F0000)>>16);
//        TBLPAG = __builtin_tbloffset(source);
        offset = (((UINT32)source) & 0x00FFFF);

	do {
		asm("tblrdh.w [%1], %0" : "=r"(highWord) : "r"(offset));
		asm("tblrdl.w [%1], %0" : "=r"(lowWord) : "r"(offset));

		if(lowWord == 0xffff) {
			DEBUG_D("Flash Appears to be empty\n\r");
			*ptr = 0x00;
			numberCopied++;
			return(numberCopied);
		}

		*ptr++ = lowWord & 0xff;
		numberCopied++;

		if((lowWord & 0xff) != 0x00) {
			*ptr++ = ((lowWord >> 8) & 0xff);
			numberCopied++;

			if(((lowWord >> 8) & 0xff) == 0x00) {
				exit = TRUE;
			}
		} else {
			exit = TRUE;
		}
		offset = offset + 2;

	} while (!exit);
	return(number_copied);
#endif
}
#endif
#endif

#if defined( __C30__ ) || defined(__XC16__)
UINT16 psv_strcpy(char *dst, __prog__ char *src, UINT16 len)
{
    char *ptr = dst;
    UINT16 i = 0;

    while((*src != 0x00) && (*src != 0xff) && (i < len - 1)) {
        *ptr++ = *src++;
        i++;
    }
    *ptr = 0x00;

    return i;
}
#endif // __C30__ || __XC16__

#ifdef HW_SPI
void spi_init(void)
{
	BYTE loop;

	DEBUG_D("spi_init()\n\r");

	/*
	 * short delay before init SPI
	 */
	for (loop = 0; loop < 0xff; loop++) Nop();

#ifdef EEPROM
	/*
	 * Initialise the EEPROM Chip Select Pin
	 */
	EEPROM_CS_PIN_DIRECTION = OUTPUT_PIN;
	EEPROM_DeSelect();
#endif

	SPI_SCK_DIRECTION = OUTPUT_PIN;
	SPI_MISO_DIRECTION = INPUT_PIN;
	SPI_MOSI_DIRECTION = OUTPUT_PIN;

#if defined(PIC24FJ256GB110)
	/*
	 *    - MISO
	 *
	 * RD8 = RP2 - SPI1 MISO Data Input RPINR20 SDI1R<5:0>
	 */
	RPINR20bits.SDI1R = 2;
	
	/*
	 * INT
	 *
	 * RA14 = RPI36 - External Interrup
	 */
//    RPINR0bits.INT1R0 = 36;
	RPINR0bits.INT1R = 36;

	/*
	 * OUTPUTS:
	 *  RD9  = RP4 - MOSI - SPI1 Data Output Function 7
	 *  RD10 = RP3 - SCK  - SPI1 Clock Output Function 8
	 *
	 *  RP3 IS on register RPOR1
	 *
	 */
	RPOR2bits.RP4R = 7;     //SPI1 MOSI
	RPOR1bits.RP3R = 8;     //SPI1 SCK

#elif defined(PIC24FJ256GB106)
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
#endif

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
#if defined(__C30__) || defined(__XC16__)
unsigned char SPIWriteByte(unsigned char write)
{
	SPI1BUF = write;
	while (!SPI_RW_FINISHED);
	return(SPI1BUF);
}
#endif
#endif

#ifdef TEST
void stall(void)
{
	while(1) {
		Nop();
	}
}
#endif
