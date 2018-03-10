/**
 *
 * \file libesoup/comms/one_wire/devices/ds2502.c
 *
 * Copyright 2018 electronicSoup Limited
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
#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "DS2502";
#include "libesoup/logger/serial_log.h"
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#define READ_ROM                 0x33
#define MATCH_ROM                0x55
#define SKIP_ROM                 0xCC
#define SEARCH_ROM               0xF0

#define READ_DATA                0xF0 // Followed by 2 Byte Address
#define READ_STATUS              0xAA // Followed by 2 Byte Address
#define READ_DATA_GENERATE_CRC   0xC3 // Followed by 2 Byte Address
#define WRITE_DATA               0x0F // Followed by 2 Byte Address
#define WRITE_STATUS             0x55 // Followed by 2 Byte Address

#define MAX_ADDRESS              0x7F

#define DS2502_FAMILY_CODE       0x09
