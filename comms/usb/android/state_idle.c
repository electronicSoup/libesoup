/**
 * @file libesoup/comms/usb/android/state_idle.c
 *
 * @author John Whitmore
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
#include "libesoup_config.h"

#ifdef SYS_USB_ANDROID

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG  = "Idle";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

/*
 * Microchip USB Includes
 */
#include "usb/inc/usb.h"
#include "usb/inc/usb_host_android.h"

#include "libesoup/comms/usb/android/android_comms.h"
#include "libesoup/comms/usb/android/state.h"

/*
 * Variables to keep track of the state of comms with Android Device.
 */
android_state_t android_state;

/*
 * Forward declarations of the states functions.
 */
//void idle_process_msg(android_command_t, void *, uint16_t);
void idle_process_msg(uint8_t, void *, uint16_t);
void idle_main(void);
void idle_process_usb_event(USB_EVENT event);

/*
 * This function simply sets up the necessary state functions for the state.
 * It's called from main.c and other files depending on state changes.
 */
void set_idle_state(void)
{
	LOG_I("Android state -> Idle\n\r");
	android_state.process_msg = idle_process_msg;
	android_state.main = idle_main;
	android_state.process_usb_event = idle_process_usb_event;
}

/*
 * When in the Idle state there is no Android Device connected so we'd
 * not expect to receive any messages from the Android for processing.
 */
void idle_process_msg(uint8_t cmd, void *data, uint16_t data_len)
{
	if (cmd == APP_MSG_APP_CONNECT) {
		SYS_ANDROID_SET_APPLICATION_CONNECTED_STATE
	} else {
		LOG_E("Android Connected State received Android message other then App connected 0x%x\n\r", cmd);
	}
}

/*
 * This Idle state has no regular processing. It will do nothing until a
 * USB Event informs it that an Android device is connected.
 */
void idle_main()
{
#if defined(SYS_ANDROID_NO_APP_FN)
	SYS_ANDROID_NO_APP_FN
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
			break;

		case EVENT_ANDROID_DETACH:
			break;

		default:
			break;
	}
}

#endif // SYS_USB_ANDROID
