/**
 *
 * @file libesoup/hardware/flash.h
 *
 * @author John Whitmore
 *
 * Flash function prototypes for electronicSoup Cinnamon Bun code Library
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
 */

#ifndef FLASH_H
#define FLASH_H

#include "libesoup_config.h"

#if defined(SYS_FLASH_RW)

extern boolean  flash_page_empty(uint32_t address);
extern result_t flash_erase_page(uint32_t address);
extern result_t flash_write_row(uint32_t address, uint8_t *data);
extern result_t flash_strcpy(char *dst, __prog__ char *src, uint16_t *length);
extern uint16_t      flash_strlen(__prog__ char *src);

#endif // defined(SYS_FLASH_RW)
#endif //FLASH_H
