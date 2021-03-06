/**
 *
 * @file libesoup/comms/usb/android/state.h
 *
 * @author John Whitmore
 *
 * Definitions of structures for Android state.
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
#ifndef _ANDROID_STATE_H
#define _ANDROID_STATE_H

#include "libesoup_config.h"

#if defined(SYS_USB_ANDROID)

//#include "usb/usb.h"
#include "usb/inc/usb.h"

typedef struct android_state_t
{
    void (*process_msg)(uint8_t, void *, uint16_t);
    void (*tx_free)(void);
    void (*main)(void);
    void (*process_usb_event)(USB_EVENT event);
} android_state_t;

extern android_state_t android_state;

#endif // defined(SYS_USB_ANDROID)
#endif // _ANDROID_STATE_H
