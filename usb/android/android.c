/**
 *
 * \file es_lib/usb/android/android.c
 *
 * Functions for communicating with Android Apps 
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

/*
 * Microchip Includes
 */ 
#include "usb/usb.h"
#include "usb/usb_host_android.h"

#include "es_lib/usb/android/android_state.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "Android"

/*
 * Definitions of the Rx and Tx circular buffer sizes.
 */
#define TX_BUFFER_SIZE 300
#define RX_BUFFER_SIZE 300

static void process_msg_from_android(void);

/*
 *
 */
static BYTE rx_buffer[RX_BUFFER_SIZE];
static BYTE rx_circular_buffer[RX_BUFFER_SIZE];

static UINT16 rx_write_index = 0;
static UINT16 rx_read_index = 0;
static UINT16 rx_buffer_count = 0;
static BOOL receiver_busy = FALSE;

static BYTE tx_buffer[TX_BUFFER_SIZE];
static BYTE tx_circular_buffer[TX_BUFFER_SIZE];

static UINT16 tx_write_index = 0;
static UINT16 tx_read_index = 0;
static UINT16 tx_buffer_count = 0;
static BOOL transmitter_busy = FALSE;

void android_init(void)
{
	rx_write_index = 0;
	rx_read_index = 0;
	rx_buffer_count = 0;
	receiver_busy = FALSE;

	tx_write_index = 0;
	tx_read_index = 0;
	tx_buffer_count = 0;
	transmitter_busy = FALSE;
}

BOOL android_receive(BYTE *buffer, UINT16 *size, BYTE *error_code)
{
	*error_code = USB_SUCCESS;

//        DEBUG_D("android_receive() rx_buffer_count is %d\n\r", rx_buffer_count);
	if (rx_buffer_count > 2) {
		UINT16 msg_size;
		UINT16 loop;

		// The first word to read is a size of the message
		msg_size = rx_circular_buffer[rx_read_index] << 8 | rx_circular_buffer[(rx_read_index + 1) % RX_BUFFER_SIZE];

                LOG_D("Receiving message size %d\n\r", msg_size);
		if(msg_size + 2 > rx_buffer_count) {
			*size = 0;
			return (FALSE);
		}

		msg_size = rx_circular_buffer[rx_read_index];
		rx_buffer_count--;
		rx_read_index = ++rx_read_index % RX_BUFFER_SIZE;

		msg_size = msg_size << 8 | rx_circular_buffer[rx_read_index];
		rx_buffer_count--;
		rx_read_index = ++rx_read_index % RX_BUFFER_SIZE;

		if (msg_size > *size) {
			LOG_D("msg_size %d\n\r", msg_size);
			LOG_D("Read received buffer size %d not big enough for %d\n\r",*size, rx_buffer_count);
			*error_code = USB_ILLEGAL_REQUEST;
			return (FALSE);
		}

		for (loop = 0; loop < msg_size; loop++) {
			buffer[loop] = rx_circular_buffer[rx_read_index];
			rx_read_index = ++rx_read_index % RX_BUFFER_SIZE;
			rx_buffer_count--;
		}
		*size = msg_size;
		return (TRUE);
	} else {
		*size = 0;
		return (FALSE);
	}
}

BYTE android_transmit(BYTE *buffer, BYTE size)
{
	UINT16 loop;
	BYTE *buffer_ptr;

	if ( (tx_buffer_count + size) >= TX_BUFFER_SIZE) {
		// ERROR Can't accept that much data at present
		LOG_E("Buffer Full\n\r");
		return (USB_EVENT_QUEUE_FULL);
	}

	buffer_ptr = buffer;

	for(loop = 0; loop < size; loop++) {
		tx_circular_buffer[tx_write_index] = *buffer_ptr++;
		tx_buffer_count++;
		tx_write_index = (++tx_write_index % TX_BUFFER_SIZE);
	}
	return (USB_SUCCESS);
}

BYTE android_tasks(void* device_handle)
{
	BYTE error_code = USB_SUCCESS;
	UINT16 loop = 0;
	UINT32 size = 0;

	if(device_handle == NULL) {
//            DEBUG_D("android_tasks() device_handle null");
		receiver_busy = FALSE;
		transmitter_busy = FALSE;
		return(USB_SUCCESS);
	}
	
	if (!receiver_busy) {
		error_code = AndroidAppRead(device_handle, (BYTE*) & rx_buffer, (UINT32)sizeof (rx_buffer));
		//If the device is attached, then lets wait for a command from the application
		if (error_code != USB_SUCCESS) {
			if (error_code != USB_ENDPOINT_BUSY) {
				LOG_E("Error trying to start read - %x\n\r", error_code);
			}
		} else {
			receiver_busy = TRUE;
		}
	}

	size = 0;

	if (receiver_busy) {
		if (AndroidAppIsReadComplete(device_handle, &error_code, &size) == TRUE) {
			//We've received a command over the USB from the Android device.
			if (error_code == USB_SUCCESS) {
				// Copy the Received Message into the Circular buffer.
				// Check if there's space for message
				if ((rx_buffer_count + size) >= RX_BUFFER_SIZE) {
					LOG_E("Error Receive buffer overflow");
				}
				for (loop = 0; loop < size; loop++) {
					rx_circular_buffer[rx_write_index] = rx_buffer[loop];
					rx_buffer_count++;
					rx_write_index = ++rx_write_index % RX_BUFFER_SIZE;
				}
				receiver_busy = FALSE;
			} else {
				//Error
				receiver_busy = FALSE;
				LOG_E("Error trying to complete read request %x\n\r", error_code);
			}
		}
	}

	/*
	 * Process the Transmitter
	 */
	if (transmitter_busy) {
		// CheckIsTheLastWriteFinished
		if (AndroidAppIsWriteComplete(device_handle, &error_code, &size) == TRUE) {
			transmitter_busy = FALSE;
		} else {
			error_code = USB_SUCCESS;
		}
	}

	if (!transmitter_busy && (tx_buffer_count > 0)) {
		// send whatever's in the queue
		UINT16 numberToSend = tx_buffer_count;
		for (loop = 0; loop < numberToSend; loop++) {
			tx_buffer[loop] = tx_circular_buffer[tx_read_index];
			tx_read_index = (++tx_read_index % TX_BUFFER_SIZE);
			tx_buffer_count--;
		}
		error_code = AndroidAppWrite(device_handle, tx_buffer, numberToSend);
		transmitter_busy = TRUE;
	}

	/*
	 * Check for any messages from the Android device.
	 */
	process_msg_from_android();

	/*
	 * Do whatever functionality is required of the current state.
	 */
	current_state.main();

	return (error_code);
}

/*!
 * \fn void process_msg_from_android(void)
 *
 * \brief processes messages from the android device
 *
 * This function polls the system for messages from the Android device
 */
static void process_msg_from_android(void)
{
	UINT16 size = 0;
	BYTE read_buffer[300];
	BYTE error_code = USB_SUCCESS;

	void *data = NULL;

	size = (UINT16)sizeof (read_buffer);
	while (android_receive((BYTE*) & read_buffer, (UINT16 *)&size, &error_code)) {
		if (error_code != USB_SUCCESS) {
			LOG_E("android_receive raised an error %x\n\r", error_code);
		}

		if (size > 1) {
			data = (void *) &read_buffer[1];
		} else {
			data = NULL;
		}

		/*
		 * Pass the received message onto the current state for processing.
		 */
		LOG_D("Process Received message in State Machine\n\r");
		current_state.process_msg(read_buffer[0], data, size - 1);
	}
}
