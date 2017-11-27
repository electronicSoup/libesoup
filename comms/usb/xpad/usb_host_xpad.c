/*********************************************************************
 *
 * \file libesoup/usb/xpad/usb_config.c
 *
 * The USB Driver for the Logitech xbox gamepad
 *
 * http://euc.jp/periphs/xbox-controller.ja.html
 *
 * Copyright 2017 electronicSoup Limited
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
#include <stdio.h>
#include "usb/inc/usb.h"
#include "usb_host_xpad_events.h"

#include "libesoup_config.h"

#define DEBUG_FILE
#define TAG "XPAD"

#include "libesoup/logger/serial_log.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif

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

/****************************************************************************
 *  Function: xpad_start()
 *
 * Simply initialises the driver by zero-ing data structures used in the 
 * driver.
 *
 ****************************************************************************/
void xpad_start()
{
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	log_i(TAG, "xpad_start()\n\r");
#endif
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
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "xpad_read returned Error 0x%x\n\r", error_code);
#endif
		}
		break;

	case READING:
		/*
		 * Ignore this as the Event hander will pick up the
		 * Transfer event.
		 */
		break;

	default:
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "xpad_tasks() - default state\n\r");
#endif
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
	uint8_t loop;
	uint16_t tmp_word;

	uint8_t endpoint_address;
	uint16_t endpoint_packet_size;

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	log_i("xpad_initialise(Address 0x%x)\n\r", address);
#endif
	xpad_device.address = address;

	descriptor = USBHostGetDeviceDescriptor(address);

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Descriptor Length %d\n\r", descriptor[USB_DESC_bLength]);
	log_d(TAG, "Descriptor Type 0x%x\n\r", descriptor[USB_DESC_bDescriptorType]);
#endif
	
	if(descriptor[USB_DESC_bDescriptorType] != USB_DEVICE_DESCRIPTOR){
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Expected the Device Descriptor\n\r");
#endif
		return(false);
	}

	for(loop = 0; loop < descriptor[USB_DESC_bLength]; loop++) {
		printf("-0x%x-", descriptor[loop]);
	}

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Device Class 0x%x\n\r", descriptor[USB_DEV_DESC_bDeviceClass]);
	log_d(TAG, "Device Sub Class 0x%x\n\r", descriptor[USB_DEV_DESC_bDeviceSubClass]);
	log_d(TAG, "Device Protocol 0x%x\n\r", descriptor[USB_DEV_DESC_bDeviceProtocol]);
	log_d(TAG, "Device MaxPacketSize0 0x%x\n\r", descriptor[USB_DEV_DESC_bMaxPacketSize0]);
#endif

	tmp_word = (uint16_t)descriptor[USB_DEV_DESC_VID_OFFSET];
	tmp_word |= ((uint16_t)descriptor[USB_DEV_DESC_VID_OFFSET + 1]) << 8;
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "VID 0x%x\n\r", tmp_word);
#endif

	tmp_word = (uint16_t)descriptor[USB_DEV_DESC_PID_OFFSET];
	tmp_word |= ((uint16_t)descriptor[USB_DEV_DESC_PID_OFFSET + 1]) << 8;
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "PID 0x%x\n\r", tmp_word);
	log_d(TAG, "Number of Configurations %d\n\r", descriptor[USB_DEV_DESC_NUM_CONFIGS_OFFSET]);
	log_d(TAG, "*******************************\n\r");
#endif
        if(xpad_device.state != NO_DEVICE) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("xpad_initilise() - device Not in NO_DEVICE State\n\r");
#endif
		return(false);
	}

	descriptor = USBHostGetCurrentConfigurationDescriptor(address);

	descriptor_length = descriptor[USB_DESC_bLength];
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Configuration Descriptor\n\r");
	log_d(TAG, "Descriptor Length %d\n\r", descriptor_length);
	log_d(TAG, "Descriptor Type 0x%x\n\r", descriptor[USB_DESC_bDescriptorType]);
#endif

	if(descriptor[USB_DESC_bDescriptorType] != USB_CONFIG_DESCRIPTOR){
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Expected the Device Descriptor\n\r");
#endif
		return(false);
	}

	for (loop = 0; loop < descriptor[USB_DESC_bLength]; loop++) {
		printf("-0x%x-", descriptor[loop]);
	}

	config_total_length = (uint16_t)descriptor[USB_CONFIG_DESC_wTotalLength];
	config_total_length |= ((uint16_t)descriptor[USB_CONFIG_DESC_wTotalLength + 1]) << 8;
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Total Length of config Descriptor %d\n\r", config_total_length);
	log_d(TAG, "*******************************\n\r");
#endif
	ptr = descriptor + descriptor_length;
	i = 0;

	while(ptr < (descriptor + config_total_length)) {
		descriptor_length = ptr[USB_DESC_bLength];
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Process Descriptor %d\n\r", i);
		log_d(TAG, "Descriptor %d Length %d\n\r", i, descriptor_length);
		log_d(TAG, "Descriptor %d Type 0x%x\n\r", i, ptr[USB_DESC_bDescriptorType]);
#endif

		for (loop = 0; loop < ptr[USB_DESC_bLength]; loop++) {
			printf("-0x%x-", ptr[loop]);
		}

		if(ptr[USB_DESC_bDescriptorType] == USB_ENDPOINT_DESCRIPTOR) {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Endpoint Descriptor\n\r");
#endif
			endpoint_address = ptr[USB_ENDPOINT_DESC_bEndpointAddress];
			endpoint_packet_size = (uint16_t) ptr[USB_ENDPOINT_DESC_wMaxPacketSize];
			endpoint_packet_size |= ((uint16_t) ptr[USB_ENDPOINT_DESC_wMaxPacketSize + 1]) << 8;

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Endpoint number - 0x%x\n\r", endpoint_address);
			log_d(TAG, "Maximum packet size %d\n\r", endpoint_packet_size);
			log_d(TAG, "Attributes 0x%x\n\r", ptr[USB_ENDPOINT_DESC_bmAttributes]);
#endif
			if(endpoint_address & 0x80) {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "IN\n\r");
#endif
				xpad_device.INEndpointNum = endpoint_address;
				xpad_device.INEndpointSize = endpoint_packet_size;
			} else {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "OUT\n\r");
#endif
				xpad_device.OUTEndpointNum = endpoint_address;
				xpad_device.OUTEndpointSize = endpoint_packet_size;
			}
		} else if (ptr[USB_DESC_bDescriptorType] == USB_INTERFACE_DESCRIPTOR) {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Interface Descriptor\n\r");
			log_d(TAG, "Interface Number 0x%x\n\r", ptr[USB_INTERFACE_DESC_bInterfaceNumber]);
			log_d(TAG, "Interface Number of Endpoints 0x%x\n\r", ptr[USB_INTERFACE_DESC_bNumEndpoints]);
			log_d(TAG, "Interface Class 0x%x\n\r", ptr[USB_INTERFACE_DESC_bInterfaceClass]);
			log_d(TAG, "Interface SubClass 0x%x\n\r", ptr[USB_INTERFACE_DESC_bInterfaceSubClass]);
			log_d(TAG, "Interface Protocol 0x%x\n\r", ptr[USB_INTERFACE_DESC_bInterfaceProtocol]);
#endif
		} else if (ptr[USB_DESC_bDescriptorType] == USB_HID_DESCRIPTOR) {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "HID Descriptor\n\r");
			log_d(TAG, "HID Num Descriptors 0x%x\n\r", ptr[USB_HID_DESC_bNumDescriptors]);
			log_d(TAG, "HID Descriptor Type 0x%x\n\r", ptr[USB_HID_DESC_bDescriptorType]);
#endif

			tmp_word = (uint16_t) ptr[USB_HID_DESC_wDescriptorLength];
			tmp_word |= ((uint16_t) ptr[USB_HID_DESC_wDescriptorLength + 1]) << 8;
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "HID Descriptor Length 0x%x\n\r", tmp_word);
			log_d(TAG, "HID Optional Descriptor Type 0x%x\n\r", ptr[USB_HID_DESC_bOptionalDescriptorType]);
#endif
			tmp_word = (uint16_t) ptr[USB_HID_DESC_wOptionalDescriptorLength];
			tmp_word |= ((uint16_t) ptr[USB_HID_DESC_wOptionalDescriptorLength + 1]) << 8;
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "HID Optional Descriptor Lenght 0x%x\n\r", tmp_word);
#endif
		}
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "*******************************\n\r");
#endif
		i++;
		ptr += descriptor_length;
	}

	xpad_device.state = DEVICE_ATTACHED;
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "End of descriptors\n\r");
#endif
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
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "EVENT_DETACH\n\r");
#endif
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
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "EVENT_RESUME\n\r");
#endif
			return true;

		case EVENT_SUSPEND:
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "EVENT_SUSPEND\n\r");
#endif
			return true;

		case EVENT_RESET:
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "EVENT_RESET\n\r");
#endif
			return true;

		case EVENT_STALL:
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "EVENT_STALL\n\r");
#endif
			return true;

		case EVENT_BUS_ERROR:
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "EVENT_BUS_ERROR\n\r");
#endif
			return true;

		default:
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			LOG_E("default unprocessed Even 0x%x\n\r", event);
#endif
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
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "EVENT_SOF\n\r");
#endif
			return true;

		case EVENT_1MS: // 1ms timer
			xpad_tasks();
			return true;
		default:
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Default event do nothing! 0x%x\n\r", event);
#endif
			break;
	}
	return false;
}
