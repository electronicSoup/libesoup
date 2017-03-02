/**
 *
 * \file es_lib/utils/eeprom.h
 *
 * Cinnamon Bun EEPROM functions of the electronicSoup code Library
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
#ifndef EEPROM_H
#define EEPROM_H

extern result_t eeprom_read(uint16_t address, uint8_t *data);
extern result_t eeprom_write(uint16_t address, uint8_t data);
extern result_t eeprom_erase(uint16_t start_address);
extern result_t eeprom_str_read(uint16_t address, uint8_t *buffer, u16 *length);
extern result_t eeprom_str_write(uint16_t address, uint8_t *buffer, u16 *length);

#endif // EEPROM_H
