/**
 *
 * \file es_lib/utils/flash.h
 *
 * Flash function prototypes for electronicSoup Cinnamon Bun code Library
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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

#include "system.h"

extern BOOL     flash_page_empty(u32 address);
extern result_t flash_erase_page(u32 address);
extern result_t flash_write_row(u32 address, uint8_t *data);
extern result_t flash_strcpy(char *dst, __prog__ char *src, uint16_t *length);
extern uint16_t      flash_strlen(__prog__ char *src);

#endif //FLASH_H
