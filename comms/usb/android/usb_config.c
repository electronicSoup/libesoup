/**
 *
 * \file libesoup/usb/android/usb_config.c
 *
 * File sets up the Target Peripheral List required by the Microchip 
 * USB Host stack. It sets up the deriver functions and the VID/PID
 * numbers supported by this Android Driver.
 *
 * Copyright 2017 - 2018 electronicSoup Limited
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

#ifdef SYS_USB_ANDROID

//#include "usb/usb.h"
//#include "usb/usb_host_android.h"
#include "usb/inc/usb.h"
#include "usb/inc/usb_host_android.h"

CLIENT_DRIVER_TABLE usbClientDrvTable[NUM_CLIENT_DRIVER_ENTRIES] =
{                                        
    {
        AndroidAppInitialize,
        AndroidAppEventHandler,
        AndroidAppDataEventHandler,
        0
    },
    {
        AndroidAppInitialize,
        AndroidAppEventHandler,
        AndroidAppDataEventHandler,
        ANDROID_INIT_FLAG_BYPASS_PROTOCOL
    }
};

USB_TPL usbTPL[NUM_TPL_ENTRIES] =
{
    { INIT_VID_PID( 0x18D1ul, 0x2D00ul ), 0, 1, {0} }, // Android accessory
    { INIT_VID_PID( 0x18D1ul, 0x2D01ul ), 0, 1, {0} }, // Android accessory
    { INIT_VID_PID( 0xFFFFul, 0xFFFFul ), 0, 0, {0} }, // Enumerates everything
};

#endif // SYS_USB_ANDROID