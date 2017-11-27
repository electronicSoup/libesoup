/*********************************************************************
 *
 * \file libesoup/usb/xpad/usb_host_xpad.h
 *
 * Defines the Interface functions to the Microchip USB Host Stack. 
 *
 * Copyright 2017 electronicSoup Limited
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
#ifndef __USB_HOST_XPAD

#define __USB_HOST_XPAD

extern bool xpad_initialise ( uint8_t address, uint32_t flags, uint8_t clientDriverID );
extern bool xpad_event_handler( uint8_t address, USB_EVENT event, void *data, uint32_t size );
extern bool xpad_data_event_handler( uint8_t address, USB_EVENT event, void *data, uint32_t size );

#endif // __USB_HOST_XPAD
