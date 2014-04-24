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
    ANDROID_APP_TYPE_REQ   = 0x01,
    ANDROID_APP_TYPE_RESP  = 0x02,
    HARDWARE_INFO_REQ      = 0x03,
    HARDWARE_INFO_RESP     = 0x04,
    BOOTCODE_INFO_REQ      = 0x05,
    BOOTCODE_INFO_RESP     = 0x06,
    FIRMWARE_INFO_REQ      = 0x07,
    FIRMWARE_INFO_RESP     = 0x08,
    COMMAND_BOOT           = 0x09,
    COMMAND_BEGIN_FLASH    = 0x0A,
    COMMAND_ERASE          = 0x0B,
    COMMAND_ROW            = 0x0C,
    COMMAND_READY          = 0x0D,
    NODE_CONFIG_INFO_REQ   = 0x0E,
    NODE_CONFIG_INFO_RESP  = 0x0F,
    NODE_CONFIG_UPDATE     = 0x10,

    COMMAND_APP_CONNECT    = 0xFE,
    COMMAND_APP_DISCONNECT = 0xFF
} android_command_t;

	/*
	 * Android App Types.
	 */
typedef enum android_app_t
{
    BOOTLOADER_APP   = 0x01,
    NODE_CONFIG_APP  = 0x02
} android_app_t;

#endif //IPC_H
