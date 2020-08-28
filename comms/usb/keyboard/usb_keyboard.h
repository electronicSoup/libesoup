/**
 * @file libesoup/comms/usb/keyboard/usb_keyboard.h
 *
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
 *
 * This file contains the Public API used by the Application layer. As such
 * it's pretty basic. At presnt all the applicaiton can do is request that
 * a key be transmitted to the USB máistir.
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

#ifdef SYS_USB_KEYBOARD

/*
 * Simple API function to send a key to the máistir Host.
 */
extern result_t send_key(uint8_t key, uint8_t shift);

#endif //SYS_USB_KEYBOARD
