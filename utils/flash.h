/**
 *
 * \file es_lib/utils/flash.h
 *
 * Flash function prototypes for electronicSoup Cinnamon Bun code Library
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

#ifndef FLASH_H
#define FLASH_H

#include "system.h"



extern BOOL flash_page_empty(UINT32 address);
extern void flash_erase(UINT32 address);
extern void flash_write(UINT32 address, BYTE *data);

#endif //FLASH_H
