/**
 *
 * @file libesoup/comms/usb/android/state_idle.h
 *
 * @author John Whitmore
 *
 * Definitions for the Android Idle state defined by the Firmware.
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
#ifndef _STATE_IDLE_H
#define _STATE_IDLE_H

#include "libesoup_config.h"

#if defined(SYS_USB_ANDROID)

extern void set_idle_state(void);

#endif // defined(SYS_USB_ANDROID)
#endif // STATES_IDLE_H
