/**
 * @file libesoup/comms/usb/keyboard/usb_config.h
 *
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
 *
 * This file is required by USB functionality of the Microchip Libraries for
 * Applications (MLA):
 * 
 * (https://www.microchip.com/mplab/microchip-libraries-for-applications).
 * 
 * This is my interpretation of the example USB Keyboard project given in the
 * MLA. The file defines how the USB firmware is to behave, so Interrupt driven,
 * amd the number of endpoints and the like. If you're changing things be
 * worth having a look in the MLA Directories at other example usb_config.h
 * files.
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

#ifndef _USB_CONFIG_H
#define _USB_CONFIG_H

#define USB_EP0_BUFF_SIZE		8	// 8, 16, 32, or 64 bytes
#define USB_MAX_NUM_INT     	        1       // Single Interface
#define USB_MAX_EP_NUMBER	        1       // Single Endpoint

#define USB_PING_PONG_MODE USB_PING_PONG__FULL_PING_PONG
#define USB_INTERRUPT
#define USB_PULLUP_OPTION USB_PULLUP_ENABLE
#define USB_TRANSCEIVER_OPTION USB_INTERNAL_TRANSCEIVER
#define USB_SPEED_OPTION USB_FULL_SPEED

#define MY_VID 0x04D8
#define MY_PID 0x0055

#define USB_ENABLE_STATUS_STAGE_TIMEOUTS
#define USB_STATUS_STAGE_TIMEOUT     (uint8_t)45

#define USB_SUPPORT_DEVICE

#define USB_NUM_STRING_DESCRIPTORS 3    // The number of string descriptors that are implemented in the usb_keybard_descriptors.c file

#define USB_USE_HID

/* HID */
#define HID_INTF_ID             0x00
#define HID_EP 			1
#define HID_INT_OUT_EP_SIZE     1
#define HID_INT_IN_EP_SIZE      8
#define HID_NUM_OF_DSC          1
#define HID_RPT01_SIZE          63

#define USER_SET_REPORT_HANDLER USBHIDCBSetReportHandler	
#define USB_DEVICE_HID_IDLE_RATE_CALLBACK(reportID, newIdleRate)    USBHIDCBSetIdleRateHandler(reportID, newIdleRate)


#endif // _USB_CONFIG_H
