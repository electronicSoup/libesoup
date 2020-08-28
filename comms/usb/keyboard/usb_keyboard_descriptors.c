/**
 * @file libesoup/comms/usb/keyboard/usb_keyboard_descriptors.c
 *
 * @author John Whitmore
 *
 * This file is pretty much a straight copy of the example given in the
 * Microchip MLA:
 *
 * (https://www.microchip.com/mplab/microchip-libraries-for-applications).
 * 
 * I've simple changed the descriptor strings for the USB Device
 *
 * Copyright 2020 electronicSoup Limited
 *
 * This file contains the Public API used by the Application layer. As such
 * it's pretty basic. At presnt all the applicaiton can do is request that
 * a key be transmitted to the USB máistir.
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
#include <stdint.h>

#include "usb.h"
#include "usb_device_hid.h"

const USB_DEVICE_DESCRIPTOR device_dsc=
{
	0x12,    // Size of this descriptor in bytes
	USB_DESCRIPTOR_DEVICE,                // DEVICE descriptor type
	0x0200,                 // USB Spec Release Number in BCD format
	0x00,                   // Class Code
	0x00,                   // Subclass code
	0x00,                   // Protocol code
	USB_EP0_BUFF_SIZE,          // Max packet size for EP0, see usb_config.h
	MY_VID,                 // Vendor ID
	MY_PID,                 // Product ID: Keyboard fw demo
	0x0001,                 // Device release number in BCD format
	0x01,                   // Manufacturer string index
	0x02,                   // Product string index
	0x00,                   // Device serial number string index
	0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
const uint8_t configDescriptor1[]={
	/* Configuration Descriptor */
	0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
	USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
	DESC_CONFIG_WORD(0x0029),   // Total length of data for this cfg
	1,                      // Number of interfaces in this cfg
	1,                      // Index value of this configuration
	0,                      // Configuration string index
	_DEFAULT | _SELF,               // Attributes, see usb_device.h
	50,                     // Max power consumption (2X mA)
	
	/* Interface Descriptor */
	0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
	USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
	0,                      // Interface Number
	0,                      // Alternate Setting Number
	2,                      // Number of endpoints in this intf
	HID_INTF,               // Class code
	BOOT_INTF_SUBCLASS,     // Subclass code
	HID_PROTOCOL_KEYBOARD,     // Protocol code
	0,                      // Interface #include <string.h>ing index

	/* HID Class-Specific Descriptor */
	0x09,//sizeof(USB_HID_DSC)+3,    // Size of this descriptor in bytes RRoj hack
	DSC_HID,                // HID descriptor /* TODO:  */ype
	DESC_CONFIG_WORD(0x0111),                 // HID Spec Release Number in BCD format (1.11)
	0x00,                   // Country Code (0x00 for Not supported)
	HID_NUM_OF_DSC,         // Number of class descriptors, see usbcfg.h
	DSC_RPT,                // Report descriptor type
	DESC_CONFIG_WORD(63),   //sizeof(hid_rpt01),      // Size of the report descriptor
	
	/* Endpoint Descriptor */
	0x07,/*sizeof(USB_EP_DSC)*/
	USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
	HID_EP | _EP_IN,            //EndpointAddress
	_INTERRUPT,                       //Attributes
	DESC_CONFIG_WORD(8),        //size
	0x01,                        //Interval
	
	/* Endpoint Descriptor */
	0x07,/*sizeof(USB_EP_DSC)*/
	USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
	HID_EP | _EP_OUT,            //EndpointAddress
	_INTERRUPT,                       //Attributes
	DESC_CONFIG_WORD(8),        //size
	0x01                        //Interval

};

//Language code string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[1];}sd000={
sizeof(sd000),USB_DESCRIPTOR_STRING,{0x0409
}};

//Manufacturer string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[19];}sd001={
sizeof(sd001),USB_DESCRIPTOR_STRING,
{'e','l','e','c','t','r','o','n','i','c','S','o','u','p',' ','L','t','d','.'}};

//Product string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[16];}sd002={
sizeof(sd002),USB_DESCRIPTOR_STRING,
{'O','n','e',' ','K','e','y',' ','K','e','y','b','o','a','r','d'}};

//Array of configuration descriptors
const uint8_t *const USB_CD_Ptr[]=
{
	(const uint8_t *const)&configDescriptor1
};

//Array of string descriptors
const uint8_t *const USB_SD_Ptr[]=
{
	(const uint8_t *const)&sd000,
	(const uint8_t *const)&sd001,
	(const uint8_t *const)&sd002
};
