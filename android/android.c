/**
 *
 * \file es_can/android/android.c
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

#include "usb/usb.h"
#include "usb/usb_host_android.h"

#define DEBUG_FILE
#include "es_can/logger/serial.h"

#if LOG_LEVEL < NO_LOGGING
#define TAG "Android"
#endif

#define TX_BUFFER_SIZE 300
#define RX_BUFFER_SIZE 300

static BYTE rxBuffer[RX_BUFFER_SIZE];
static BYTE rxCircularBuffer[RX_BUFFER_SIZE];

static UINT16 rxWriteIndex = 0;
static UINT16 rxReadIndex = 0;
static UINT16 rxBufferCount = 0;
static BOOL receiverBusy = FALSE;

static BYTE txBuffer[TX_BUFFER_SIZE];
static BYTE txCircularBuffer[TX_BUFFER_SIZE];

static UINT16 txWriteIndex = 0;
static UINT16 txReadIndex = 0;
static UINT16 txBufferCount = 0;
static BOOL transmitterBusy = FALSE;

BOOL androidReceive(BYTE *buffer, UINT16 *size, BYTE *errorCode)
{
	*errorCode = USB_SUCCESS;
    
	if (rxBufferCount > 2) {
		UINT16 msgSize;
		UINT16 loop;

		// The first word to read is a size of the message
		msgSize = rxCircularBuffer[rxReadIndex] << 8 | rxCircularBuffer[(rxReadIndex + 1) % RX_BUFFER_SIZE];

		if(msgSize + 2 > rxBufferCount) {
			*size = 0;
			return (FALSE);
		}

		msgSize = rxCircularBuffer[rxReadIndex];
		rxBufferCount--;
		rxReadIndex = ++rxReadIndex % RX_BUFFER_SIZE;

		msgSize = msgSize << 8 | rxCircularBuffer[rxReadIndex];
		rxBufferCount--;
		rxReadIndex = ++rxReadIndex % RX_BUFFER_SIZE;

		if (msgSize > *size) {
			DEBUG_D("msgSize %d\n\r", msgSize);
			DEBUG_D("Read received buffer size %d not big enough for %d\n\r",*size, rxBufferCount);
			*errorCode = USB_ILLEGAL_REQUEST;
			return (FALSE);
		}

		for (loop = 0; loop < msgSize; loop++) {
			buffer[loop] = rxCircularBuffer[rxReadIndex];
			rxReadIndex = ++rxReadIndex % RX_BUFFER_SIZE;
			rxBufferCount--;
		}
		*size = msgSize;
		return (TRUE);
	} else {
		*size = 0;
		return (FALSE);
	}
}

BYTE androidTransmit(BYTE *buffer, BYTE size)
{
	UINT16 loop;
	BYTE *bufferPtr;

	if ( (txBufferCount + size) >= TX_BUFFER_SIZE) {
		// ERROR Can't accept that much data at present
		DEBUG_E("Buffer Full\n\r");
		return (USB_EVENT_QUEUE_FULL);
	}

	bufferPtr = buffer;

	for(loop = 0; loop < size; loop++) {
		txCircularBuffer[txWriteIndex] = *bufferPtr++;
		txBufferCount++;
		txWriteIndex = (++txWriteIndex % TX_BUFFER_SIZE);
	}
	return (USB_SUCCESS);
}

BYTE androidTasks(void* device_handle)
{
	BYTE errorCode = USB_SUCCESS;
	UINT16 loop = 0;
	UINT32 size = 0;

	if(device_handle == NULL) {
		receiverBusy = FALSE;
		transmitterBusy = FALSE;
		return(USB_SUCCESS);
	}
	
	if (!receiverBusy) {
		errorCode = AndroidAppRead(device_handle, (BYTE*) & rxBuffer, (UINT32)sizeof (rxBuffer));
		//If the device is attached, then lets wait for a command from the application
		if (errorCode != USB_SUCCESS) {
			//Error
			if (errorCode != USB_ENDPOINT_BUSY) {
				DEBUG_E("Error trying to start read - %x\n\r", errorCode);
			}
		} else {
			receiverBusy = TRUE;
		}
	}

	size = 0;

	if (receiverBusy) {
		if (AndroidAppIsReadComplete(device_handle, &errorCode, &size) == TRUE) {
			//We've received a command over the USB from the Android device.
			if (errorCode == USB_SUCCESS) {
				// Copy the Received Message into the Circular buffer.
				// Check if there's space for message
				if ((rxBufferCount + size) >= RX_BUFFER_SIZE) {
					DEBUG_E("Error Receive buffer overflow");
				}
				for (loop = 0; loop < size; loop++) {
					rxCircularBuffer[rxWriteIndex] = rxBuffer[loop];
					rxBufferCount++;
					rxWriteIndex = ++rxWriteIndex % RX_BUFFER_SIZE;
				}
				receiverBusy = FALSE;
			} else {
				//Error
				receiverBusy = FALSE;
				DEBUG_E("Error trying to complete read request %x\n\r", errorCode);
			}
		}
	}

	/*
	 * Process the Transmitter
	 */
	if (transmitterBusy) {
		// CheckIsTheLastWriteFinished
		if (AndroidAppIsWriteComplete(device_handle, &errorCode, &size) == TRUE) {
			transmitterBusy = FALSE;
		} else {
			errorCode = USB_SUCCESS;
		}
	}

	if (!transmitterBusy && (txBufferCount > 0)) {
		// send whatever's in the queue
		UINT16 numberToSend = txBufferCount;
		for (loop = 0; loop < numberToSend; loop++) {
			txBuffer[loop] = txCircularBuffer[txReadIndex];
			txReadIndex = (++txReadIndex % TX_BUFFER_SIZE);
			txBufferCount--;
		}
		errorCode = AndroidAppWrite(device_handle, txBuffer, numberToSend);
		transmitterBusy = TRUE;
	}

	return (errorCode);
}
