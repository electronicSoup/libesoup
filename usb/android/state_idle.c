/**
 *
 * \file es_lib/usb/android/states/idle.c
 *
 * The Bootloader Idle state for the Android Communications.
 *
 * Basically the state waits for an Android device to connect and moves
 * to the Connected state.
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
#include "system.h"

/*
 * Microchip USB Includes
 */
#include "usb/usb.h"
#include "usb/usb_host_android.h"

#include "es_lib/usb/android/state.h"
#include "es_lib/usb/android/ipc.h"
#include "es_lib/usb/android/state_android_connected.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "Idle"

/*
 * Forward declarations of the states functions.
 */
//void idle_process_msg(android_command_t, void *, UINT16);
void idle_process_msg(BYTE, void *, UINT16);
void idle_main(void);
void idle_process_usb_event(USB_EVENT event);

/*
 * This function simply sets up the necessary state functions for the state.
 * It's called from main.c and other files depending on state changes.
 */
void set_idle_state(void)
{
	LOG_D("State --> Idle\n\r");

	current_state.process_msg = idle_process_msg;
	current_state.main = idle_main;
	current_state.process_usb_event = idle_process_usb_event;
}

/*
 * When in the Idle state there is no Android Device connected so we'd
 * not expect to receive any messages from the Android for processing.
 */
void idle_process_msg(BYTE cmd, void *data, UINT16 data_len)
{
	LOG_E("Received Android Msg in the Idle state command 0x%x data Lentgh %d\n\r", cmd, data_len);
}

/*
 * This Idle state has no regular processing. It will do nothing until a
 * USB Event informs it that an Android device is connected.
 */
void idle_main()
{
#if defined(NO_ANDROID_APP_FN)
	NO_ANDROID_APP_FN
#endif
}

/*
 * In the Idle state we're only interest in an Attach event. When that happens
 * we simply change state to reflect the change. No other event causes any
 * change or action.
 */
void idle_process_usb_event(USB_EVENT event)
{
	switch (event) {
		case EVENT_ANDROID_ATTACH:
			set_android_connected_state();
			break;

		default:
			break;
	}
}
