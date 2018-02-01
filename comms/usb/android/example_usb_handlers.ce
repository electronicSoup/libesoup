/**
 *
 * \file libesoup/comms/usb/android/example_usb_handlers.c
 *
 * Copyright 2017 2018 electronicSoup Limited
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

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "USB_Handlers";
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

#include "libesoup/comms/usb/android/state.h"
#include "libesoup/comms/usb/android/android_comms.h"

bool USB_ApplicationDataEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
#if (defind(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("USB_ApplicationDataEventHandler()\n\r");
#endif
	return FALSE;
}

bool USB_ApplicationEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//	LOG_D("USB_ApplicationEventHandler()\n\r");
#endif
	asm ("CLRWDT");

	switch( event) {
		case EVENT_VBUS_REQUEST_POWER:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//			LOG_D("EVENT_VBUS_REQUEST_POWER current %d\n\r", ((USB_VBUS_POWER_EVENT_DATA*) data)->current);
#endif
			// The data pointer points to a byte that represents the amount of power
			// requested in mA, divided by two.  If the device wants too much power,
			// we reject it.
			if (((USB_VBUS_POWER_EVENT_DATA*) data)->current <= (MAX_ALLOWED_CURRENT / 2)) {
				return TRUE;
			} else {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
				LOG_E("\r\n***** USB Error - device requires too much current *****\r\n");
#endif
			}
			break;

		case EVENT_VBUS_RELEASE_POWER:
		case EVENT_HUB_ATTACH:
		case EVENT_UNSUPPORTED_DEVICE:
		case EVENT_CANNOT_ENUMERATE:
		case EVENT_CLIENT_INIT_ERROR:
		case EVENT_OUT_OF_MEMORY:
		case EVENT_UNSPECIFIED_ERROR: // This should never be generated.
		case EVENT_DETACH: // USB cable has been detached (data: BYTE, address of device)
		case EVENT_ANDROID_DETACH:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Device Detached\n\r");
#endif
			/*
			 * Pass a Detach event on to the state machine for processing.
			 */
			android_state.process_usb_event(EVENT_ANDROID_DETACH);
			android_init(NULL);
			return TRUE;
			break;

			// Android Specific events
		case EVENT_ANDROID_ATTACH:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Device Attached\n\r");
#endif
			/*
			 * Pass an Attach even on to the State machine.
			 */
			android_state.process_usb_event(EVENT_ANDROID_ATTACH);
			android_init(data);
			return TRUE;

		case EVENT_OVERRIDE_CLIENT_DRIVER_SELECTION:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Ignoring EVENT_OVERRIDE_CLIENT_DRIVER_SELECTION\n\r");
#endif
			break;

		default:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_WARNING))
			LOG_W("Unknown Event 0x%x\n\r", event);
#endif
			break;
	}
	return FALSE;
}
