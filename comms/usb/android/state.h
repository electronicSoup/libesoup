/**
 *
 * \file es_lib/usb/android/android_state.h
 *
 * Definitions of structures for Android state.
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#ifndef ANDROID_STATE_H
#define ANDROID_STATE_H

#include "system.h"
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

#endif // ANDROID_STATE_H
