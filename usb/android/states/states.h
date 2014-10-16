/**
 *
 * \file es_lib/usb/android/states/states.h
 *
 * Definitions for the Android states defined by the Firmware.
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
#ifndef STATES_H
#define STATES_H

#include "system.h"
#include "usb/usb.h"
#if defined(ANDROID_NODE) || defined(ANDROID_BOOT)
#include "node_ipc.h"
#elif defined(ANDROID_DONGLE)
#include "dongle_ipc.h"
#endif

typedef struct state_t
{
    void (*process_msg)(android_command_t, void *, UINT16);
    void (*main)(void);
    void (*process_usb_event)(USB_EVENT event);
} state_t;

extern state_t current_state;

extern void set_idle_state(void);
extern void set_android_connected_state(void);
#if defined(ANDROID_NODE) || defined(ANDROID_BOOT)
extern void set_node_connected_state(void);
#elif defined(ANDROID_DONGLE)
extern void set_dongle_connected_state(void);
#endif

#endif // STATES_H
