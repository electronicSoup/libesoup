/**
 *
 * \file states/androidConnected.c
 *
 * The Android Device Connected state for the Android Communications.
 *
 * Basically the state waits for the Android Application to connect and moves
 * to the App Connected state.
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
#include "usb/usb.h"
#include "usb/usb_host_android.h"
#include "ipc.h"
#include "states.h"

#define DEBUG_FILE
#include "es_lib/logger/serial.h"

#if LOG_LEVEL < NO_LOGGING
#define TAG "Android"
#endif

/*
 * Forward declatation of functions in the file.
 */
void android_connected_process_msg(android_command_t, void *);
void android_connected_main(void);
void android_connected_process_usb_event(USB_EVENT event);

/*
 * Function to set up the functions used in this state.
 */
void set_android_connected_state(void)
{
    DEBUG_D("State --> AndroidConnected\n\r");

    current_state.process_msg = android_connected_process_msg;
    current_state.main = android_connected_main;
    current_state.process_usb_event = android_connected_process_usb_event;
}

/*
 * In this Android Device Connected state we're waiting for the Applicaiton
 * connected command from the Android Device and moving state accordingly.
 * we're not interested in any other messages from the Connected device.
 */
void android_connected_process_msg(android_command_t cmd, void *data)
{
    if(cmd == COMMAND_APP_CONNECT) {
        set_app_connected_state();
    } else {
        DEBUG_E("Android Connected State received Android message other then App connected\n\r");
    }
}

/*
 * This state has no polling functionality which must be preformed.
 */
void android_connected_main()
{
}

/*
 * In this state the only USB Event we have to process is a Detach event in
 * which case we move back to the idle state.
 */
void android_connected_process_usb_event(USB_EVENT event)
{
    switch(event)
    {
        case EVENT_ANDROID_DETACH:
            set_idle_state();
            break;

        default:
            break;
    }
}
