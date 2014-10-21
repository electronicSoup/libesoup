/**
 *
 * \file states/dongle_connected.c
 *
 * The Android Application Connected state for the ESCanDongle Communications.
 *
 * State for processing comms from the Android Application.
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
#include <stdio.h>
#include "system.h"

/*
 * Microchip USB Includes
 */
#include "usb/usb.h"
#include "usb/usb_host_android.h"


#include "dongle_ipc.h"
#include "states.h"
#include "es_lib/utils/utils.h"
#include "es_lib/usb/android/android.h"
#include "es_lib/usb/android/android_state.h"
#include "main.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"
#undef DEBUG_FILE

#define TAG "DongleConnected"

//void dongle_connected_process_msg(android_command_t, void *, UINT16);
void dongle_connected_process_msg(BYTE, void *, UINT16);
void dongle_connected_main(void);
void dongle_connected_process_usb_event(USB_EVENT event);

void set_dongle_connected_state(void)
{
	LOG_D("Dongle Connected State\n\r");
	current_state.process_msg = dongle_connected_process_msg;
	current_state.main = dongle_connected_main;
	current_state.process_usb_event = dongle_connected_process_usb_event;
}

void dongle_connected_process_msg(BYTE cmd, void *data, UINT16 data_len)
{
	LOG_D("dongle_connected_process_msg data lenght %d\n\r", data_len);
	switch (cmd) {

		default:
			LOG_W("Unprocessed message 0x%x\n\r", cmd);
			break;
	}
}

void dongle_connected_main()
{
}

void dongle_connected_process_usb_event(USB_EVENT event)
{
	switch (event) {
		case EVENT_ANDROID_ATTACH:
			break;

		case EVENT_ANDROID_DETACH:
			set_idle_state();
			break;
	}
}
