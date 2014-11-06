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
 * Microchip USB Includes
 */ 
#include "usb/usb.h"
#include "usb/usb_host_android.h"

#include "es_lib/usb/android/state.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "Android-Com"

/*
 * Definitions of the Rx and Tx circular buffer sizes.
 */
#define TX_BUFFER_SIZE 300
#define RX_BUFFER_SIZE 300

/*
 * Forward declaration of funcitons in file
 */
static void process_msg_from_android(void);
static BOOL android_receive(BYTE *buffer, UINT16 *size, BYTE *error_code);
static void process_msg_from_android(void);

/*
 * Definitions for the two Circular buffers being used for Tx and Rx
 * messages To and From the connected Android device
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

/*
 * Device handle for the connected USB Device
 */
static void* device_handle = NULL;

/*
 * Interrupt Handler for the USB Host Functionality of Microchip USB Stack.
 */
void __attribute__((interrupt,auto_psv)) _USB1Interrupt()
{
        USB_HostInterruptHandler();
}

/*
 * void android_init(void *data)
 *
 * Function to initialise the two circular buffers being used for comms with 
 * connected Android Device.
 *
 * Input : void *data - The device handle of the connected Android Device or
 *                      NULL if the device has disconnected.
 */
void android_init(void* data)
{
	device_handle = data;

	rx_write_index = 0;
	rx_read_index = 0;
	rx_buffer_count = 0;
	receiver_busy = FALSE;

	tx_write_index = 0;
	tx_read_index = 0;
	tx_buffer_count = 0;
	transmitter_busy = FALSE;
}

/*
 * void android_tasks(void)
 *
 * Function called regularly to do house keeping of the connected Android 
 * device, if a device is connected. The function reads messages from the
 * Android device inserting the received data into the Rx circular buffer,
 * for later processing. It also processes messages in the Transmission 
 * circular buffer taking each complete Tx message and sending it to the 
 * Android device, when the transmiter is free to do so.
 *
 */
void android_tasks(void)
{
	BYTE error_code = USB_SUCCESS;
	result_t rc = SUCCESS;
	UINT16 loop = 0;
	UINT32 size = 0;

	/*
	 * If there is not connected Android Device then simply perform state 
	 * processing and return.
	 */
	if(device_handle == NULL) {
		receiver_busy = FALSE;
		transmitter_busy = FALSE;

		current_state.main();

		return;
	}

	/*
	 * We have a connected Android device so check if the receiver is busy
	 * if its not busy then simply attempt to read a message from the 
	 * Android Device.
	 */
	if (!receiver_busy) {
		error_code = AndroidAppRead(device_handle, (BYTE*) & rx_buffer, (UINT32)sizeof (rx_buffer));

		if (error_code != USB_SUCCESS) {
			if (error_code != USB_ENDPOINT_BUSY) {
				LOG_E("Error trying to start read - %x\n\r", error_code);
			}
		} else {
			receiver_busy = TRUE;
		}
	}

	/*
	 * If the receiver is busy check to see if the current read operation 
	 * is complete. If the read is finished then write the received 
	 * data into the Rx Circular buffer.
	 */
	size = 0;

	if (receiver_busy) {
		if (AndroidAppIsReadComplete(device_handle, &error_code, &size)) {

			if (error_code == USB_SUCCESS) {

				/*
				 * Check if there's space for message in the
				 * Rx Circular Buffer.
				 */
				if ((rx_buffer_count + size) >= RX_BUFFER_SIZE) {
					LOG_E("Error Receive buffer overflow");
				} else {
					/*
					 * We have space so copy received 
					 * bytes into circular buffer.
					 */
					for (loop = 0; loop < size; loop++) {
						rx_circular_buffer[rx_write_index] = rx_buffer[loop];
						rx_buffer_count++;
						rx_write_index = ++rx_write_index % RX_BUFFER_SIZE;
					}
					LOG_D("Message transfered to RX circular buffer\n\r");
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
		/*
		 * Transmitter is busy so check if it's finished its current write.
		 */
		if (AndroidAppIsWriteComplete(device_handle, &error_code, &size) == TRUE) {
			transmitter_busy = FALSE;
		}
	}

	/*
	 * If the transmitter is free and there is data to send in the circular
	 * then send buffer then send the next message.
	 */
	if (!transmitter_busy && (tx_buffer_count > 0)) {
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
	 * Execute whatever functionality is required of the current state.
	 */
	current_state.main();
}

/*
 * static BOOL android_receive(BYTE *buffer, UINT16 *size, BYTE *error_code)
 *
 * Function if there is a complete message in the Receive circular buffer then
 * remove the message from the circular buffer and place it in the Input buffer
 * parameter for the caller to process.
 *
 * Input/Output : BYTE *buffer - location where received message should be copied.
 *
 * Input/Output : UINT16 *size - Input size of input buffer.
 *                               Output size of the received message.
 *
 * Output       : BYTE *error_code - Error code if an error occured.
 *
 * Return       : True if a message was received successfully
 *                Flase if no message was received or there was an error.
 *
 */
static BOOL android_receive(BYTE *buffer, UINT16 *size, BYTE *error_code)
{
	UINT16 msg_size;
	UINT16 loop;
	*error_code = USB_SUCCESS;

	/*
	 * The first word (2 Bytes) of the received message is the size of the
	 * incoming message. If there's not more then 2 bytes in the circular
	 * buffer we can't have a complete message so ignore it.
	 */
	if (rx_buffer_count <= 2) {
		*size = 0;
		return (FALSE);
	}

	/*
	 * determing the size of the incoming message
	 */
	msg_size = rx_circular_buffer[rx_read_index] << 8 | rx_circular_buffer[(rx_read_index + 1) % RX_BUFFER_SIZE];

	/*
	 * If we dont't have more bytes in the Rx Circular buffer then the 
	 * size of the incoming message then there is not a complete 
	 * message in the circular buffer so ignore the incomplete message.
	 */
	if(msg_size + 2 > rx_buffer_count) {
		*size = 0;
		return (FALSE);
	}

	/*
	 * We have a complete message in the Rx Circular buffer.
	 * First remove the 2 size bytes from the circular buffer
	 */
	msg_size = rx_circular_buffer[rx_read_index];
	rx_buffer_count--;
	rx_read_index = ++rx_read_index % RX_BUFFER_SIZE;

	msg_size = msg_size << 8 | rx_circular_buffer[rx_read_index];
	rx_buffer_count--;
	rx_read_index = ++rx_read_index % RX_BUFFER_SIZE;

	/*
	 * Check that the input buffer is big enough for received message
	 */
	if (msg_size > *size) {
		LOG_D("msg_size %d\n\r", msg_size);
		LOG_D("Read received buffer size %d not big enough for %d\n\r",*size, rx_buffer_count);

		/*
		 * Remove this huge message from circular buffer. It will be 
		 * Ignored.
		 */
		for (loop = 0; loop < msg_size; loop++) {
			rx_read_index = ++rx_read_index % RX_BUFFER_SIZE;
			rx_buffer_count--;
		}

		*error_code = USB_ILLEGAL_REQUEST;
		return (FALSE);
	}

	/*
	 * Have buffer space for message so copy message to buffer.
	 */
	for (loop = 0; loop < msg_size; loop++) {
		buffer[loop] = rx_circular_buffer[rx_read_index];
		rx_read_index = ++rx_read_index % RX_BUFFER_SIZE;
		rx_buffer_count--;
	}
	*size = msg_size;
	return (TRUE);
}

/*
 * static BYTE android_transmit(BYTE *buffer, BYTE size)
 *
 * Function to load the Transmit Circular buffer with a given message for
 * future transmission to the Android Device.
 *
 * Input : BYTE *buffer - Message to Transmit.
 *
 * Input : BYTE size    - size of input message.
 *
 * Return : USB_EVENT_QUEUE_FULL - No space for message in Circular buffer
 *          USB_SUCCESS            Message queued for future transmission.
 *
 */
BYTE android_transmit(BYTE *buffer, BYTE size)
{
	UINT16 loop;
	BYTE *buffer_ptr;

	/*
	 * Check we have enough free space in Tx Circular buffer for message.
	 */
	if ( (tx_buffer_count + size) >= TX_BUFFER_SIZE) {
		LOG_E("Buffer Full\n\r");
		return (USB_EVENT_QUEUE_FULL);
	}

	buffer_ptr = buffer;

	/*
	 * Copy message to Tx Circular buffer.
	 */
	for(loop = 0; loop < size; loop++) {
		tx_circular_buffer[tx_write_index] = *buffer_ptr++;
		tx_buffer_count++;
		tx_write_index = (++tx_write_index % TX_BUFFER_SIZE);
	}
	return (USB_SUCCESS);
}

/*
 * static void process_msg_from_android(void)
 *
 * Function to pull received messages from Rx Circular buffer and pass 
 * them on to applicaiton for further processing.
 *
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
		LOG_D("Process Received message (0x%x) in State Machine size %d\n\r",read_buffer[0], size-1);
		current_state.process_msg(read_buffer[0], data, size - 1);
	}
}