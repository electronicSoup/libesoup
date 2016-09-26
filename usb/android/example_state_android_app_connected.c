/**
 *
 * \file es_lib/usb/android/example_state_android_app_connected.c
 *
 * An exmaple of an Android app connected state file.
 *
 * State for processing comms from the Android Application.
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
#include <stdio.h>
#include "system.h"
#include "usb/usb.h"
#include "usb/usb_host_android.h"
#include "es_lib/usb/android/state.h"
#include "es_lib/usb/android/state_idle.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "Android-AppConnected"

/*
 * Forward declarations of functions.
 */
void app_connected_process_msg(BYTE, void *, UINT16);
void app_connected_main(void);
void app_connected_process_usb_event(USB_EVENT event);

void example_set_app_connected_state(void)
{
	LOG_D("State -> App_connected\n\r");

	android_state.process_msg = app_connected_process_msg;
	android_state.main = app_connected_main;
	android_state.process_usb_event = app_connected_process_usb_event;
}

/*
 * app_connected_process_msg
 *
 * The function which processes messages received from the Connected Android Device.
 * NB The es_lib Android Comms code passes the data pointer to this function but as
 * soon as this function returns the buffer WILL be reused for the next message from 
 * the connected Android. If you want to keep this received after the function returns 
 * then copy it out to your own buffer!
 */
void app_connected_process_msg(BYTE cmd, void *data, UINT16 data_len)
{
}

void app_connected_main()
{
}

void app_connected_process_usb_event(USB_EVENT event)
{
	switch (event) {
		case EVENT_ANDROID_ATTACH:
			break;

		case EVENT_ANDROID_DETACH:
			set_idle_state();
			break;

		default:
			break;
	}
}

