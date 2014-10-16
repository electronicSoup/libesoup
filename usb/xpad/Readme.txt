This directory contains the USB Driver code for the Logitech F310 Gamepad
controller. 

To use this code simply add the c source files to your project and add the 
directory to the include path of the project. The directory has to be added
to the include path as the Microchip USB Host Stack will attempt to include
the header file system_config.h from somewhere in your project's include path.

In Additon to these files the following files should be added to your project:

usb_hal_local.h ->    .../microchip_solutions_v2013-12-20/framework/usb/src/usb_hal_local.h
usb_host_android.c -> .../microchip_solutions_v2013-12-20/framework/usb/src/usb_host_android.c
usb_host.c ->         .../microchip_solutions_v2013-12-20/framework/usb/src/usb_host.c
usb_host_local.h ->   .../microchip_solutions_v2013-12-20/framework/usb/src/usb_host_local.h

Your application code should implemtent the funciton
USB_ApplicationEventHandler() defined in system_config.h

#define USB_HOST_APP_EVENT_HANDLER USB_ApplicationEventHandler

This function will be passed the events defined in the file
usb_host_xpad_events.h for example:

bool USB_ApplicationEventHandler( BYTE address, USB_EVENT event, void *data, DWORD size )
{
	switch ((INT) event) {
		case USB_XPAD_D_DOWN_PRESS:
			LOG_D("USB_XPAD_D_DOWN_PRESS\n\r");
			break;

		case USB_XPAD_D_DOWN_RELEASE:
			LOG_D("USB_XPAD_D_DOWN_RELEASE\n\r");
			break;

		default:
			break;
	}
	return FALSE;
}
