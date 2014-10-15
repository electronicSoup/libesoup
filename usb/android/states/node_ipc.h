/**
 *
 * \file ipc.h
 *
 * Definitions for communications with an Android Device for Bootloader 
 * and CAN Node
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
#ifndef NODE_IPC_H
#define NODE_IPC_H

typedef enum
{
    ANDROID_APP_TYPE_REQ   = 0x01,
    ANDROID_APP_TYPE_RESP  = 0x02,
    HARDWARE_INFO_REQ      = 0x03,
    HARDWARE_INFO_RESP     = 0x04,
    BOOTCODE_INFO_REQ      = 0x05,
    BOOTCODE_INFO_RESP     = 0x06,
    FIRMWARE_INFO_REQ      = 0x07,
    FIRMWARE_INFO_RESP     = 0x08,
    APPLICATION_INFO_REQ   = 0x09,
    APPLICATION_INFO_RESP  = 0x0A,
    COMMAND_BOOT           = 0x0B,
    COMMAND_BEGIN_FLASH    = 0x0C,
    COMMAND_ERASE          = 0x0D,
    COMMAND_ROW            = 0x0E,
    COMMAND_READY          = 0x0F,
    COMMAND_REFLASHED      = 0x10,
    NODE_CONFIG_INFO_REQ   = 0x11,
    NODE_CONFIG_INFO_RESP  = 0x12,
    NODE_CONFIG_UPDATE     = 0x13,

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

#endif //NODE_IPC_H
