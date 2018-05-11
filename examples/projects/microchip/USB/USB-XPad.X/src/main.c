/**
 * @file libesoup/examples/projects/microchip/USB/USB-Xpad/src/main.c
 *
 * @author John Whitmore
 * 
 * @brief An example main.c file for using USB Host to communicate with a
 *        Logitech XPad game controller.
 *
 * Copyright 2018 electronicSoup Limited
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
 *******************************************************************************
 */
/*
 * AN1140 and AN1141
 */
#include "libesoup_config.h"

#define DEBUG_FILE
static const char *TAG = "MAIN";
#include "libesoup/logger/serial_log.h"

#ifdef SYS_USB_XPAD
/*
 * Microchip MLA Include
 */
#include "usb/inc/usb.h"
#endif

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#ifdef SYS_USB_XPAD
#include "libesoup/comms/usb/xpad/usb_host_xpad_events.h"
#endif

int main(void)
{
	result_t rc;
	
	asm("CLRWDT");
	TRISDbits.TRISD0 = GPIO_OUTPUT_PIN;
	LATDbits.LATD0   = 0;
	
	TRISDbits.TRISD1 = GPIO_OUTPUT_PIN;
	LATDbits.LATD1   = 0;

	TRISDbits.TRISD3 = GPIO_OUTPUT_PIN;
	LATDbits.LATD3   = 0;

	TRISDbits.TRISD4 = GPIO_OUTPUT_PIN;
	LATDbits.LATD4   = 0;

	rc = libesoup_init();
	if(rc < 0) {
		TRISDbits.TRISD0 = GPIO_OUTPUT_PIN;
		LATDbits.LATD0   = 1;
		while(1);
	}
		
	LOG_D("************************\n\r");
	LOG_D("***    USB Gamepad   ***\n\r");
	LOG_D("************************\n\r");

	TRISEbits.TRISE1 = 0;
#ifdef SYS_USB_XPAD
	BRD_USB_HOST
#endif
	LOG_D("Entering the main loop\n\r");

	while (1) {
#ifdef SYS_USB_XPAD
		USBTasks();
#endif
	}
}

#ifdef SYS_USB_XPAD
bool USB_ApplicationEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
	LOG_D("USB_ApplicationEventHandler\n\r");
	switch ((uint16_t) event) {
		case EVENT_VBUS_REQUEST_POWER:
			// The data pointer points to a byte that represents the amount of power
				// requested in mA, divided by two.  If the device wants too much power,
				// we reject it.
			LOG_D("EVENT_VBUS_REQUEST_POWER\n\r");
			if (((USB_VBUS_POWER_EVENT_DATA*) data)->current <= (MAX_ALLOWED_CURRENT / 2)) {
				return TRUE;
			} else {
				LOG_D("\r\n***** USB Error - device requires too much current *****\r\n");
			}
			break;

		case EVENT_VBUS_RELEASE_POWER:
			LOG_D("EVENT_VBUS_RELEASE_POWER\n\r");
			// Turn off Vbus power.
				// The PIC24F with the Explorer 16 cannot turn off Vbus through software.
			return TRUE;
			break;

		case USB_XPAD_D_DOWN_PRESS:
			LOG_D("USB_XPAD_D_DOWN_PRESS\n\r");
			PORTEbits.RE1 = 1;
			return(TRUE);
			break;

		case USB_XPAD_D_DOWN_RELEASE:
			LOG_D("USB_XPAD_D_DOWN_RELEASE\n\r");
			PORTEbits.RE1 = 0;
			return(TRUE);
			break;

                case EVENT_UNSUPPORTED_DEVICE:
			LOG_D("EVENT_UNSUPPORTED_DEVICE\n\r");
                        break;
                        
                case EVENT_CANNOT_ENUMERATE:
			LOG_D("EVENT_CANNOT_ENUMERATE\n\r");
                        break;
                        
		default:
			LOG_E("Default case 0x%x!\n\r", event);
			break;
	}
	return FALSE;
}
#endif
