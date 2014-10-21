/**
 *
 * \file es_lib/utils/eeprom.h
 *
 * Cinnamon Bun EEPROM functions of the electronicSoup code Library
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
#ifndef EEPROM_H
#define EEPROM_H

extern result_t eeprom_read(UINT16 address, BYTE *data);
extern result_t eeprom_write(UINT16 address, BYTE data);
extern result_t eeprom_erase(UINT16 start_address);
extern result_t eeprom_str_read(UINT16 address, BYTE *buffer, UINT16 *length);
extern result_t eeprom_str_write(UINT16 address, BYTE *buffer, UINT16 *length);

#endif // EEPROM_H
