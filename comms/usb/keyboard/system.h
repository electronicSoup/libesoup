/**
 * @file libesoup/comms/usb/keyboard/system.h
 *
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
 *
 * This file is required by USB functionality of the Microchip Libraries for
 * Applications (MLA):
 * 
 * (https://www.microchip.com/mplab/microchip-libraries-for-applications).
 * 
 * This is my interpretation of the example USB Keyboard project given in the
 * MLA. 
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
#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <xc.h>

typedef enum
{
    SYSTEM_STATE_USB_START,
    SYSTEM_STATE_USB_SUSPEND,
    SYSTEM_STATE_USB_RESUME
} SYSTEM_STATE;

void SYSTEM_Initialize( SYSTEM_STATE state );

#define SYSTEM_Tasks()

#endif // _SYSTEM_H
