/*********************************************************************
 *
 * @file libesoup/usb/xpad/usb_config.c
 *
 * @author John Whitmore
 * 
 * @brief Required by the Microchip USB Library 
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
 *********************************************************************
 * 
 * This file sets up the Target Peripheral List for the USB devices
 * supported by this USB Driver. The TPL Structure has to be setup for
 * the Microchip Host stack.
 */
#include "libesoup_config.h"

#ifdef SYS_USB_XPAD

//#include "usb/usb.h"
#include "usb/inc/usb.h"
#include "usb_host_xpad.h"

CLIENT_DRIVER_TABLE usbClientDrvTable[NUM_CLIENT_DRIVER_ENTRIES] =
{                                        
    {
        xpad_initialise,
        xpad_event_handler,
        xpad_data_event_handler,
        0
    }
};

USB_TPL usbTPL[NUM_TPL_ENTRIES] =
{
    { INIT_VID_PID( 0x046Dul, 0xC21Dul ), 0, 0, {0} }, // Logitech Gamepad F310
};

#endif // SYS_USB_XPAD
