/**
 * @file libesoup/usb/xpad/usb_config.c
 *
 * @author John Whtimore
 * 
 * @brief The USB Driver for the Logitech xbox gamepad
 *
 * http://euc.jp/periphs/xbox-controller.ja.html
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
 *********************************************************************
 * 
 */
#include "libesoup_config.h"

#ifdef SYS_USB_XPAD

//#include <stdio.h>
#include "usb/inc/usb.h"
#include "usb_host_xpad_events.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "XPAD";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

/*
 * USB Descriptor Constants.
 */
#define USB_DESC_bLength                                0
#define USB_DESC_bDescriptorType                        1

/*
 * USB Descriptor Types.
 */
#define USB_DEVICE_DESCRIPTOR                           0x01
#define USB_CONFIG_DESCRIPTOR                           0x02
#define USB_STRING_DESCRIPTOR                           0x03
#define USB_INTERFACE_DESCRIPTOR                        0x04
#define USB_ENDPOINT_DESCRIPTOR                         0x05
#define USB_HID_DESCRIPTOR                              0x21

/*
 * USB Device Descriptor data offsets.
 */
#define USB_DEV_DESC_bDeviceClass                       4
#define USB_DEV_DESC_bDeviceSubClass                    5
#define USB_DEV_DESC_bDeviceProtocol                    6
#define USB_DEV_DESC_bMaxPacketSize0                    7
#define USB_DEV_DESC_VID_OFFSET                         8
#define USB_DEV_DESC_PID_OFFSET                         10
#define USB_DEV_DESC_NUM_CONFIGS_OFFSET                 17

/*
 * USB Config Descriptor data offsets.
 */
#define USB_CONFIG_DESC_wTotalLength                    2

/*
 * USB Endpoint Descriptor data offsets.
 */
#define USB_ENDPOINT_DESC_bEndpointAddress              2
#define USB_ENDPOINT_DESC_bmAttributes                  3
#define USB_ENDPOINT_DESC_wMaxPacketSize                4

/*
 * USB Interface Descriptor data offsets.
 */
#define USB_INTERFACE_DESC_bInterfaceNumber             2
#define USB_INTERFACE_DESC_bNumEndpoints                4
#define USB_INTERFACE_DESC_bInterfaceClass              5
#define USB_INTERFACE_DESC_bInterfaceSubClass           6
#define USB_INTERFACE_DESC_bInterfaceProtocol           7

/*
 * USB Human Interface Device Descriptor data offsets.
 */
#define USB_HID_DESC_bNumDescriptors                    5
#define USB_HID_DESC_bDescriptorType                    6
#define USB_HID_DESC_wDescriptorLength                  7
#define USB_HID_DESC_bOptionalDescriptorType            9
#define USB_HID_DESC_wOptionalDescriptorLength          10

/*
 * This driver will use a very simple state machine simply constantly
 * reading the data from the connected gamepad device.
 */
typedef enum {
	NO_DEVICE = 0,
	DEVICE_ATTACHED,
	READ,
	READING
} XPAD_DEVICE_STATE;

/*
 * Information on the connected USB xpad device
 */
typedef struct
{
    uint8_t address;
    uint8_t clientDriverID;
    uint8_t OUTEndpointNum;
    uint16_t OUTEndpointSize;
    uint8_t INEndpointNum;
    uint16_t INEndpointSize;
    XPAD_DEVICE_STATE state;
    uint16_t countDown;
    uint16_t protocol;

    struct
    {
        uint8_t tx_busy :1;
        uint8_t rx_busy :1;
	uint8_t rx_request_sent :1;
        uint8_t EP0TransferPending :1;
    } status;

    struct
    {
        uint8_t* data;
        uint8_t  length;
        uint8_t  offset;
        uint8_t  id;
        uint8_t  HIDEventSent      :1;
    } hid;

} XPAD_DEVICE_DATA;

static XPAD_DEVICE_DATA xpad_device;

/*
 * Data structure for the data received from the connected USB
 * device:
 *
 * http://euc.jp/periphs/xbox-controller.ja.html
 */
typedef struct {
	uint8_t zero;
	uint8_t size;

	struct {
		uint8_t D_pad_up : 1;
		uint8_t D_pad_down : 1;
		uint8_t D_pad_left : 1;
		uint8_t D_pad_right : 1;
		uint8_t start : 1;
		uint8_t back : 1;
		uint8_t left_stick : 1;
		uint8_t right_stick : 1;
	} digital_buttons;

	uint8_t reserved;
	uint8_t A;
	uint8_t B;
	uint8_t X;
	uint8_t Y;
	uint8_t black;
	uint8_t white;
	uint8_t l_trigger;
	uint8_t r_trigger;
	uint8_t l_stick_x;
	uint8_t l_stick_y;
	uint8_t r_stick_x;
	uint8_t r_stick_y;

} XPAD_DATA;

XPAD_DATA xpad_data;

#define RX_BUFFER_SIZE 32
uint8_t rx_buffer[RX_BUFFER_SIZE];

/*
 * Interrupt for the USB Peripheral
 */
void _ISR __attribute__((__no_auto_psv__)) _USB1Interrupt(void)
{
	LATDbits.LATD4 = ~PORTDbits.RD4;
	USB_HostInterruptHandler();
}

/****************************************************************************
 *  Function: xpad_start()
 *
 * Simply initialises the driver by zero-ing data structures used in the 
 * driver.
 *
 ****************************************************************************/
void xpad_start()
{
	LOG_I("xpad_start()\n\r");
	memset(&xpad_device,0x00,sizeof(xpad_device));
	memset(&xpad_data,0x00,sizeof(xpad_data));
}

/****************************************************************************
 *  Function: uint8_t xpad_read(uint8_t* data, uint32_t size)
 *
 * Initiates a read opperation from the connected xpad device. The read is
 * simply initiated, the Microchip Host Stack will process the read and return
 * any data to this driver via the EVENT_TRANSFER event. So data is processed
 * in the drivers event handler.
 *
 ****************************************************************************/
uint8_t xpad_read(uint8_t* data, uint32_t size)
{
	uint8_t errorCode;

	if (xpad_device.address == 0) {
		return USB_UNKNOWN_DEVICE;
	}

	if (xpad_device.state < READ) {
		return USB_INVALID_STATE;
	}

	if (xpad_device.status.rx_busy == 1) {
		return USB_ENDPOINT_BUSY;
	}

	errorCode = USBHostRead(xpad_device.address,
		                xpad_device.INEndpointNum,
                                data,
			        ((size / xpad_device.INEndpointSize) * xpad_device.INEndpointSize) );

	switch (errorCode) {
		case USB_SUCCESS:
		case USB_ENDPOINT_BUSY:
			xpad_device.status.rx_busy = 1;
			break;
		default:
			xpad_device.status.rx_busy = 0;
			break;
	}

	return errorCode;
}

/****************************************************************************
 * Function: xpad_tasks()
 *
 * xpad driver's state machine. The state machine simply initiates a USB
 * read oppearion when the state has been set to read. The driver constantly
 * reads from the connected deivce.
 *
 ****************************************************************************/
void xpad_tasks(void)
{
	uint8_t error_code;

        switch(xpad_device.state) {
	case NO_DEVICE:
		break;
		
	case DEVICE_ATTACHED:
		xpad_device.state = READ;
		break;

	case READ:
		error_code = xpad_read(rx_buffer, RX_BUFFER_SIZE);
		if (error_code == USB_SUCCESS) {
			xpad_device.state = READING;
		} else {
			LOG_E("xpad_read returned Error 0x%x\n\r", error_code);
		}
		break;

	case READING:
		/*
		 * Ignore this as the Event hander will pick up the
		 * Transfer event.
		 */
		break;

	default:
		LOG_E("xpad_tasks() - default state\n\r");
		break;
	}
}

/****************************************************************************
 * Function: xpad_change()
 *
 * This funciton simply process the 20Byte structure returned from the xpad
 * monitors for changes in the xpad controller and forwards events to the 
 * application's even handler for further processing.
 *
 * At present only processing the D Pad Down Button as an example.
 *
 ****************************************************************************/
void xpad_change()
{
	XPAD_DATA *data;

	data = (XPAD_DATA *)rx_buffer;

	if(data->digital_buttons.D_pad_down != xpad_data.digital_buttons.D_pad_down) {
		xpad_data.digital_buttons.D_pad_down = data->digital_buttons.D_pad_down;
		if(xpad_data.digital_buttons.D_pad_down) {
			USB_HOST_APP_EVENT_HANDLER(xpad_device.address,
			                           USB_XPAD_D_DOWN_PRESS,
						   NULL,
						   0);
		} else {
			USB_HOST_APP_EVENT_HANDLER(xpad_device.address,
			                           USB_XPAD_D_DOWN_RELEASE,
						   NULL,
						   0);
		}
	}
}

/****************************************************************************
 * Function: xpad_initialise
 *
 * Function called by the Microchip USB Host stack when a device is plugged
 * into the USB Port. It simply precesses the Device's descriptors and stores
 * retreived data in the xpad_device structure.
 ****************************************************************************/
bool xpad_initialise ( uint8_t address, uint32_t flags, uint8_t clientDriverID )
{
	uint8_t *descriptor = NULL;
	uint8_t *ptr = NULL;
	uint8_t descriptor_length;
	uint16_t config_total_length;
	uint8_t i;
#ifdef SYS_SERIAL_LOGGING
	uint8_t loop;
#endif
	uint16_t tmp_word;

	uint8_t endpoint_address;
	uint16_t endpoint_packet_size;

	LOG_I("xpad_initialise(Address 0x%x)\n\r", address);
	xpad_device.address = address;

	descriptor = USBHostGetDeviceDescriptor(address);

	LOG_D("Descriptor Length %d\n\r", descriptor[USB_DESC_bLength]);
	LOG_D("Descriptor Type 0x%x\n\r", descriptor[USB_DESC_bDescriptorType]);
	
	if(descriptor[USB_DESC_bDescriptorType] != USB_DEVICE_DESCRIPTOR){
		LOG_E("Expected the Device Descriptor\n\r");
		return(false);
	}
#ifdef SYS_SERIAL_LOGGING
	for(loop = 0; loop < descriptor[USB_DESC_bLength]; loop++) {
		serial_printf("-0x%x-", descriptor[loop]);
	}
#endif
	LOG_D("Device Class 0x%x\n\r", descriptor[USB_DEV_DESC_bDeviceClass]);
	LOG_D("Device Sub Class 0x%x\n\r", descriptor[USB_DEV_DESC_bDeviceSubClass]);
	LOG_D("Device Protocol 0x%x\n\r", descriptor[USB_DEV_DESC_bDeviceProtocol]);
	LOG_D("Device MaxPacketSize0 0x%x\n\r", descriptor[USB_DEV_DESC_bMaxPacketSize0]);

	tmp_word = (uint16_t)descriptor[USB_DEV_DESC_VID_OFFSET];
	tmp_word |= ((uint16_t)descriptor[USB_DEV_DESC_VID_OFFSET + 1]) << 8;
	LOG_D("VID 0x%x\n\r", tmp_word);

	tmp_word = (uint16_t)descriptor[USB_DEV_DESC_PID_OFFSET];
	tmp_word |= ((uint16_t)descriptor[USB_DEV_DESC_PID_OFFSET + 1]) << 8;
	LOG_D("PID 0x%x\n\r", tmp_word);
	LOG_D("Number of Configurations %d\n\r", descriptor[USB_DEV_DESC_NUM_CONFIGS_OFFSET]);
	LOG_D("*******************************\n\r");
        if(xpad_device.state != NO_DEVICE) {
		LOG_E("xpad_initilise() - device Not in NO_DEVICE State\n\r");
		return(false);
	}

	descriptor = USBHostGetCurrentConfigurationDescriptor(address);

	descriptor_length = descriptor[USB_DESC_bLength];
	LOG_D("Configuration Descriptor\n\r");
	LOG_D("Descriptor Length %d\n\r", descriptor_length);
	LOG_D("Descriptor Type 0x%x\n\r", descriptor[USB_DESC_bDescriptorType]);

	if(descriptor[USB_DESC_bDescriptorType] != USB_CONFIG_DESCRIPTOR){
		LOG_E("Expected the Device Descriptor\n\r");
		return(false);
	}

	config_total_length = (uint16_t)descriptor[USB_CONFIG_DESC_wTotalLength];
	config_total_length |= ((uint16_t)descriptor[USB_CONFIG_DESC_wTotalLength + 1]) << 8;
	LOG_D("Total Length of config Descriptor %d\n\r", config_total_length);
	LOG_D("*******************************\n\r");
	ptr = descriptor + descriptor_length;
	i = 0;

	while(ptr < (descriptor + config_total_length)) {
		descriptor_length = ptr[USB_DESC_bLength];
		LOG_D("Process Descriptor %d\n\r", i);
		LOG_D("Descriptor %d Length %d\n\r", i, descriptor_length);
		LOG_D("Descriptor %d Type 0x%x\n\r", i, ptr[USB_DESC_bDescriptorType]);

		if(ptr[USB_DESC_bDescriptorType] == USB_ENDPOINT_DESCRIPTOR) {
			LOG_D("Endpoint Descriptor\n\r");
			endpoint_address = ptr[USB_ENDPOINT_DESC_bEndpointAddress];
			endpoint_packet_size = (uint16_t) ptr[USB_ENDPOINT_DESC_wMaxPacketSize];
			endpoint_packet_size |= ((uint16_t) ptr[USB_ENDPOINT_DESC_wMaxPacketSize + 1]) << 8;

			LOG_D("Endpoint number - 0x%x\n\r", endpoint_address);
			LOG_D("Maximum packet size %d\n\r", endpoint_packet_size);
			LOG_D("Attributes 0x%x\n\r", ptr[USB_ENDPOINT_DESC_bmAttributes]);
			if(endpoint_address & 0x80) {
				LOG_D("IN\n\r");
				xpad_device.INEndpointNum = endpoint_address;
				xpad_device.INEndpointSize = endpoint_packet_size;
			} else {
				LOG_D("OUT\n\r");
				xpad_device.OUTEndpointNum = endpoint_address;
				xpad_device.OUTEndpointSize = endpoint_packet_size;
			}
		} else if (ptr[USB_DESC_bDescriptorType] == USB_INTERFACE_DESCRIPTOR) {
			LOG_D("Interface Descriptor\n\r");
			LOG_D("Interface Number 0x%x\n\r", ptr[USB_INTERFACE_DESC_bInterfaceNumber]);
			LOG_D("Interface Number of Endpoints 0x%x\n\r", ptr[USB_INTERFACE_DESC_bNumEndpoints]);
			LOG_D("Interface Class 0x%x\n\r", ptr[USB_INTERFACE_DESC_bInterfaceClass]);
			LOG_D("Interface SubClass 0x%x\n\r", ptr[USB_INTERFACE_DESC_bInterfaceSubClass]);
			LOG_D("Interface Protocol 0x%x\n\r", ptr[USB_INTERFACE_DESC_bInterfaceProtocol]);
		} else if (ptr[USB_DESC_bDescriptorType] == USB_HID_DESCRIPTOR) {
			LOG_D("HID Descriptor\n\r");
			LOG_D("HID Num Descriptors 0x%x\n\r", ptr[USB_HID_DESC_bNumDescriptors]);
			LOG_D("HID Descriptor Type 0x%x\n\r", ptr[USB_HID_DESC_bDescriptorType]);

			tmp_word = (uint16_t) ptr[USB_HID_DESC_wDescriptorLength];
			tmp_word |= ((uint16_t) ptr[USB_HID_DESC_wDescriptorLength + 1]) << 8;
			LOG_D("HID Descriptor Length 0x%x\n\r", tmp_word);
			LOG_D("HID Optional Descriptor Type 0x%x\n\r", ptr[USB_HID_DESC_bOptionalDescriptorType]);
			tmp_word = (uint16_t) ptr[USB_HID_DESC_wOptionalDescriptorLength];
			tmp_word |= ((uint16_t) ptr[USB_HID_DESC_wOptionalDescriptorLength + 1]) << 8;
			LOG_D("HID Optional Descriptor Lenght 0x%x\n\r", tmp_word);
		}
		LOG_D("*******************************\n\r");
		i++;
		ptr += descriptor_length;
	}

	xpad_device.state = DEVICE_ATTACHED;
	LOG_D("End of descriptors\n\r");
	return true;
}

/****************************************************************************
 * Function: xpad_event_handler()
 *
 * Handler for events from the Microchip USB Host Stack. The main event 
 * received is the EVENT_TRANSFER event which will pass data to this driver
 * received from the connected xpad controller.
 *
 ****************************************************************************/
bool xpad_event_handler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
	uint32_t count;

	switch (event) {
		case EVENT_NONE: // No event occured (NULL event)
			return true;

		case EVENT_DETACH:
			LOG_D("EVENT_DETACH\n\r");
			xpad_device.state = NO_DEVICE;
			return true;

		case EVENT_HUB_ATTACH:
			return true;

		case EVENT_TRANSFER:
			count = ((HOST_TRANSFER_DATA *)data)->dataCount;

			if ( ((HOST_TRANSFER_DATA *)data)->bEndpointAddress == (xpad_device.INEndpointNum) )
			{
				if (count) { // unless NAK or ZLP
					/*
					 * Process data received for button chages.
					 */
					xpad_change();
				}
			}

			xpad_device.status.rx_busy = 0;
			xpad_device.state = READ;
			return true;

		case EVENT_RESUME:
			LOG_D("EVENT_RESUME\n\r");
			return true;

		case EVENT_SUSPEND:
			LOG_D("EVENT_SUSPEND\n\r");
			return true;

		case EVENT_RESET:
			LOG_D("EVENT_RESET\n\r");
			return true;

		case EVENT_STALL:
			LOG_D("EVENT_STALL\n\r");
			return true;

		case EVENT_BUS_ERROR:
			LOG_D("EVENT_BUS_ERROR\n\r");
			return true;

		default:
			LOG_E("default unprocessed Even 0x%x\n\r", event);
			break;
	}
	return false;
}

/****************************************************************************
 * Function: xpad_data_event_handler()
 *
 * This data event handler is only used for the 1MS event which causes the 
 * driver's state machine to be checked for required processing.
 *
 ****************************************************************************/
bool xpad_data_event_handler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
	switch (event) {
		case EVENT_SOF: // Start of frame - NOT NEEDED
			LOG_D("EVENT_SOF\n\r");
			return true;

		case EVENT_1MS: // 1ms timer
			xpad_tasks();
			return true;
		default:
			LOG_D("Default event do nothing! 0x%x\n\r", event);
			break;
	}
	return false;
}

#endif // SYS_USB_XPAD
