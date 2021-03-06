/**
 *
 * @file libesoup/hardware/eeprom.h
 *
 * @author John Whitmore
 *
 * Cinnamon Bun EEPROM functions of the electronicSoup code Library
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
#ifndef EEPROM_H
#define EEPROM_H

#include "libesoup_config.h"

#if defined(SYS_EEPROM)

extern result_t eprom_init(uint8_t spi_chan);
extern result_t eeprom_read(uint16_t address);
extern result_t eeprom_write(uint16_t address, uint8_t data);
extern result_t eeprom_erase(uint16_t start_address);
extern result_t eeprom_str_read(uint16_t address, uint8_t *buffer, uint16_t length);
extern result_t eeprom_str_write(uint16_t address, uint8_t *buffer, uint16_t length);

#endif //  defined(SYS_EEPROM)

#endif // EEPROM_H
