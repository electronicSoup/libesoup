/**
 *
 * \file ipc.h
 *
 * Definitions for communications with an Android Device
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
#ifndef IPC_H
#define IPC_H

typedef enum android_command_t
{
    COMMAND_BOOT           = 0x01,
    COMMAND_BEGIN_FLASH    = 0x02,
    COMMAND_ERASE          = 0x03,
    COMMAND_ROW            = 0x04,
    COMMAND_READY          = 0x05,
    HARDWARE_INFO_REQ      = 0x06,
    HARDWARE_INFO_RESP     = 0x07,
    BOOTCODE_INFO_REQ      = 0x08,
    BOOTCODE_INFO_RESP     = 0x09,
    FIRMWARE_INFO_REQ      = 0x0A,
    FIRMWARE_INFO_RESP     = 0x0B,

    COMMAND_APP_CONNECT    = 0xFE,
    COMMAND_APP_DISCONNECT = 0xFF
} android_command_t;

#endif //IPC_H
