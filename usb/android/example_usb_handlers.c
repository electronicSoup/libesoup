#include "system.h"

/*
 * Microchip USB Includes
 */
#include "usb/usb.h"
#include "usb/usb_host_android.h"

#include "es_lib/usb/android/state.h"
#include "es_lib/usb/android/android_comms.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "USB_Handlers"

bool USB_ApplicationDataEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
	LOG_D("USB_ApplicationDataEventHandler()\n\r");
	return FALSE;
}

bool USB_ApplicationEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
//	LOG_D("USB_ApplicationEventHandler()\n\r");

	asm ("CLRWDT");

	switch( event) {
		case EVENT_VBUS_REQUEST_POWER:
//			LOG_D("EVENT_VBUS_REQUEST_POWER current %d\n\r", ((USB_VBUS_POWER_EVENT_DATA*) data)->current);
			// The data pointer points to a byte that represents the amount of power
			// requested in mA, divided by two.  If the device wants too much power,
			// we reject it.
			if (((USB_VBUS_POWER_EVENT_DATA*) data)->current <= (MAX_ALLOWED_CURRENT / 2)) {
				return TRUE;
			} else {
				LOG_E("\r\n***** USB Error - device requires too much current *****\r\n");
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
			LOG_D("Device Detached\n\r");
			/*
			 * Pass a Detach event on to the state machine for processing.
			 */
			android_state.process_usb_event(EVENT_ANDROID_DETACH);
			android_init(NULL);
			return TRUE;
			break;

			// Android Specific events
		case EVENT_ANDROID_ATTACH:
			LOG_D("Device Attached\n\r");
			/*
			 * Pass an Attach even on to the State machine.
			 */
			android_state.process_usb_event(EVENT_ANDROID_ATTACH);
			android_init(data);
			return TRUE;

		case EVENT_OVERRIDE_CLIENT_DRIVER_SELECTION:
			LOG_D("Ignoring EVENT_OVERRIDE_CLIENT_DRIVER_SELECTION\n\r");
			break;

		default:
			LOG_W("Unknown Event 0x%x\n\r", event);
			break;
	}
	return FALSE;
}
