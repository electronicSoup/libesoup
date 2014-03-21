/**
 *
 * \file es_can/utils/utils.h
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

#ifndef UTILS_H
#define UTILS_H

//#include "os_api.h"
#include "system.h"


#if defined(PIC18F4585)
  #define FLASH_PAGE_SIZE 64
  #define FIRST_FLASH_ADDRESS 0x6000
  #define LAST_FLASH_ADDRESS 0xBFFF
#endif

#if defined(PIC18F4585)
    #define GPO_4 0
    #define GPO_8 1
#endif

#ifdef HEARTBEAT
extern void heartbeat_on(union sigval data);
extern void heartbeat_off(union sigval data);
#endif

#if 0
extern BYTE generate_hash(char *string);
#endif

extern BOOL flash_page_empty(UINT32 address);
extern void flash_erase(UINT32 address);
extern void flash_write(UINT32 address, BYTE *data);

#if defined(PIC18F4585)

    extern void flash_test(UINT32 address);

#endif

#ifdef EEPROM
extern result_t eeprom_read(UINT16 addr, BYTE *data);
extern result_t eeprom_write(UINT16 addr, BYTE data);
extern result_t sys_eeprom_read(UINT16 address, BYTE *data);
extern result_t sys_eeprom_write(UINT16 address, BYTE data);
#endif

#ifdef HW_SPI
#if defined(PIC24FJ256GB106)
extern void spi_init(void);
extern unsigned char SPIWriteByte(unsigned char write);
#endif
#endif

extern void initRand(void);

#if defined( __C30__ )
    extern UINT16 strcpypgmtoram(char *dest, const char *source);
#endif

#ifdef TEST
    void stall(void);
#endif
#endif //UTILS_H
