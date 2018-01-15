/**
 *
 * \file libesoup/can/iso15765.c
 *
 * Based on ISO_15765-2 but with a much reduced Message Length. The standard
 * allows for 4KB of a message whereas for the electronicSoup CAN Bus Nodes
 * the limit is 74 Bytes, including an initial protocol byte.
 *
 * Copyright 2017 2018 electronicSoup Limited
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
#if !defined(XC16) && !defined(__XC8) && !defined(ES_LINUX)
#error Unrecognised Compiler!
#endif

#include <stdio.h>
#include <string.h>
#if defined(ES_LINUX)
    #include <stdlib.h>
#endif

#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "ISO-15765";
#include "libesoup/logger/serial_log.h"
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  SYS_LOG_LEVEL
#endif // SYS_SERIAL_LOGGING

#include "libesoup/can/es_can.h"
#include "libesoup/can/dcncp/dcncp_can.h"
#include "libesoup/timers/sw_timers.h"

typedef struct
{
    uint8_t used;
    uint8_t protocol;
    iso15765_msg_handler_t handler;
} iso15765_register_t;

static iso15765_msg_handler_t unhandled_handler;

static iso15765_register_t registered[SYS_ISO15765_REGISTER_ARRAY_SIZE];
static void dispatcher_iso15765_msg_handler(iso15765_msg_t *message);

#define SINGLE_FRAME_SIZE 7

/*
 * The CAN ID as used by the Layer 3 Protocol
 *
 *   28..26 | 25  24  |   23..16   |  15..8  |  7..0  |
 *    110   |  1   1  | Target Type| Target  | Source |
 * Priority
 */
typedef union
{
    struct
    {
        uint8_t source;
        uint8_t destination;
        uint8_t type;
        uint8_t layer3;
    } bytes;
    uint32_t can_id;
} iso15765_id;

#define ISO15765_TARGET_PHYSICAL   218
#define ISO15765_TARGET_FUNCTIONAL 219

#define ISO15765_COMS 0x1B

#define ISO15765_SF 0x00
#define ISO15765_FF 0x10
#define ISO15765_CF 0x20
#define ISO15765_FC 0x30

#define BLOCK_SIZE 2
#define SEPERATION_TIME 0x25

#define ISO15765_EXTENDED TRUE
#define ISO15765_MASK     0xfffeff00

#define FS_CTS 0x00
#define FS_Wait 0x01
#define FS_Overflow 0x02

static iso15765_id tx_frame_id;

typedef struct {
	uint8_t             block_size;
	uint8_t             seperation_time;
	can_frame      frame;
	uint8_t             sequence;
	uint8_t             data[SYS_ISO15765_MAX_MSG];
	uint16_t            index;
	uint8_t             frames_sent_in_block;
	uint16_t            bytes_to_send;
	uint16_t            bytes_sent;
	uint8_t             destination;
	es_timer       consecutive_frame_timer;
	es_timer       timer_N_Bs;
} tx_buffer_t;

typedef struct {
	uint8_t             block_size;
	uint8_t             seperation_time;
	uint8_t             data[SYS_ISO15765_MAX_MSG];
	uint16_t            index;
	uint8_t             sequence;
	uint8_t             protocol;
	uint16_t            bytes_expected;
	uint16_t            bytes_received;
	uint8_t             frames_received_in_block;
	uint8_t             source;
	can_frame      frame;
	iso15765_msg_t msg;
	es_timer       timer_N_Cr;
} rx_buffer_t;

#if defined(XC16) || defined(__XC8)
static BOOL mcp_transmitter_busy = FALSE;
static BOOL mcp_receiver_busy = FALSE;

static rx_buffer_t mcp_rx_buffer;
static tx_buffer_t mcp_tx_buffer;
#elif defined(ES_LINUX)
typedef struct {
	tx_buffer_t *tx_buffer;
	rx_buffer_t *rx_buffer;
} node_buffers_t;

#define NUM_NODES 256

node_buffers_t node_buffers[NUM_NODES];
#endif


/*
 * This node's local Layer 3 Node Address
 */
static uint8_t node_address;
static uint8_t initialised = 0x00;

static void iso15765_frame_handler(can_frame *rxMsg);

static void init_tx_buffer(tx_buffer_t *);
static void init_rx_buffer(rx_buffer_t *);

static void exp_sendConsecutiveFrame(timer_t timer_id, union sigval);
static void sendFlowControlFrame(rx_buffer_t *rx_buffer, uint8_t flowStatus);
static void startConsecutiveFrameTimer(tx_buffer_t *tx_buffer) ;

static void startTimer_N_Cr(rx_buffer_t *);
static void stopTimer_N_Cr(rx_buffer_t *);
static void exp_timer_N_Cr_Expired(timer_t timer_id, union sigval);

static void startTimer_N_Bs(tx_buffer_t *);
static void stopTimer_N_Bs(tx_buffer_t *);
static void exp_timer_N_Bs_Expired(timer_t timer_id, union sigval);

void init_tx_buffer(tx_buffer_t *tx_buf)
{
	tx_buf->block_size = 0;
	tx_buf->seperation_time = 0;
	tx_buf->sequence = 0x00;
	tx_buf->index = 0x00;
	tx_buf->frames_sent_in_block = 0;
	tx_buf->bytes_to_send = 0x00;
	tx_buf->bytes_sent = 0x00;
	tx_buf->destination = 0x00;
	TIMER_INIT(tx_buf->consecutive_frame_timer);
	TIMER_INIT(tx_buf->timer_N_Bs);
}

void init_rx_buffer(rx_buffer_t *rx_buf)
{
	rx_buf->block_size = BLOCK_SIZE;
	rx_buf->seperation_time = SEPERATION_TIME;
	rx_buf->index = 0x00;
	rx_buf->sequence = 0x00;
	rx_buf->protocol = 0;
	rx_buf->bytes_expected = 0;
	rx_buf->bytes_received = 0;
	rx_buf->frames_received_in_block = 0x00;
	rx_buf->source = 0;
	TIMER_INIT(rx_buf->timer_N_Cr);
}

result_t iso15765_init(uint8_t address)
{
	uint16_t loop;
	can_l2_target_t target;

	for(loop = 0; loop < SYS_ISO15765_REGISTER_ARRAY_SIZE; loop++) {
		registered[loop].used = FALSE;
		registered[loop].protocol = 0x00;
		registered[loop].handler = (iso15765_msg_handler_t)NULL;
	}

	unhandled_handler = (iso15765_msg_handler_t)NULL;

#if defined(XC16) || defined(__XC8)
	/*
	 * Microchip Microcontrollers only have one Static RX and one Static TX Buffer
	 */
	init_rx_buffer(&mcp_rx_buffer);

	mcp_receiver_busy = FALSE;
	mcp_transmitter_busy = FALSE;
#elif defined(ES_LINUX)
	for(loop = 0; loop < NUM_NODES; loop++) {
		node_buffers[loop].tx_buffer = NULL;
		node_buffers[loop].rx_buffer = NULL;
	}
#endif
        node_address = address;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("l3_init() node address = 0x%x\n\r", node_address);
#endif
	/*
	 * Initialise the static parts or our tx message header.
	 */
	tx_frame_id.bytes.layer3 = ISO15765_COMS;
	tx_frame_id.bytes.type = ISO15765_TARGET_PHYSICAL;
	tx_frame_id.bytes.source = node_address;
	tx_frame_id.bytes.destination = node_address;

	tx_frame_id.can_id |= CAN_EFF_FLAG;

	/*
	 * Define our target for Layer 2 Frames and register it.
	 */
	target.mask = ISO15765_MASK;
	target.filter = tx_frame_id.can_id & 0xffffff00; //Don't filter on the Source Byte
	target.handler = iso15765_frame_handler;

	can_l2_dispatch_reg_handler(&target);

	initialised = 0x01;
	return(SUCCESS);
}

uint8_t iso15765_initialised(void)
{
    return(initialised);
}

result_t iso15765_tx_msg(iso15765_msg_t *msg)
{
	uint8_t          *data_ptr;
	uint16_t          loop;
	iso15765_id  id;
	tx_buffer_t *tx_buffer;
	uint16_t          size;
	uint8_t           tmp;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("Tx to 0x%x, Protocol-0x%x, len(0x%x)\n\r",
#endif
		   (uint16_t)msg->address,
		   (uint16_t)msg->protocol,
		   (uint16_t)msg->size);

	data_ptr = msg->data;
//	for(loop = 0; loop < msg->size; loop++) {
//		printf("0x%2x,", *data_ptr++);
//	}
//	printf("\n\r");

        if(!initialised) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("ISO15765 not Initialised\n\r");
#endif
		return(ERR_UNINITIALISED);
	}

	if(msg->size == 0) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("ISO15765 Message Zero size not Sending\n\r");
#endif
		return(ERR_BAD_INPUT_PARAMETER);
	}

	if(msg->size > SYS_ISO15765_MAX_MSG) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("L3_Can Message exceeds size limit\n\r");
#endif
		return(ERR_BAD_INPUT_PARAMETER);
	}

#if defined(XC16) || defined(__XC8)
	if(mcp_transmitter_busy) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("ISO15765 transmitter already busy\n\r");
#endif
		return(ERR_BUSY);
	}
	tx_buffer = &mcp_tx_buffer;
#elif defined(ES_LINUX)
	/*
	 * Check for a transmit buffer already active to the destination
	 */
	if(node_buffers[msg->address].tx_buffer != NULL) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("ISO15765 transmitter already busy\n\r");
#else
#error Unrecognised Compiler!
#endif
		return(ERR_BUSY);
	}

	tx_buffer = malloc(sizeof(tx_buffer_t));
	if(!tx_buffer) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Malloc Failed\n\r");
#endif
		exit(1);
	}
	node_buffers[msg->address].tx_buffer = tx_buffer;
#endif
	init_tx_buffer(tx_buffer);

	/*
	 * cut off for the single frame message.
	 */
	if(msg->size + 1 <= SINGLE_FRAME_SIZE ) { // +1 for Protocol Byte
		tx_buffer->frame.can_dlc = msg->size + 2;
		id.can_id = tx_frame_id.can_id;
		id.bytes.destination = msg->address;
		tx_buffer->frame.can_id = id.can_id;

		tx_buffer->frame.data[0] = ISO15765_SF | ((msg->size + 1) & 0x0f);
		tx_buffer->frame.data[1] = msg->protocol;

		data_ptr = msg->data;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Tx Single Frame\n\r");
#endif
		for(loop = 0; loop < msg->size; loop++) {
			tx_buffer->frame.data[loop + 2] = *data_ptr++;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("ISO15765 TX Byte 0x%x\n\r", tx_buffer->frame.data[loop + 2]);
#endif
		}
		can_l2_tx_frame(&(tx_buffer->frame));
	} else {
		/*
		 * Copy the l3 message to be sent into the Trasmit buffer.
		 */
		memcpy((void*)tx_buffer->data, (void *)msg->data, msg->size);
		tx_buffer->bytes_to_send = msg->size;
		tx_buffer->bytes_sent = 0x00;
		tx_buffer->destination = msg->address;
#if defined(XC16) || defined(__XC8)
		mcp_transmitter_busy = TRUE;
#endif
		// Fill in the can id we're going to use for the transmission.
		id = tx_frame_id;
		id.bytes.destination = tx_buffer->destination;

		tx_buffer->frame.can_id = id.can_id;
	        tx_buffer->frame.can_dlc = CAN_DATA_LENGTH;

		size = msg->size + 1; // Add one for Protocol Byte
		tmp = (uint8_t)(size >> 8);
		tmp = tmp & 0x0f;
		tx_buffer->frame.data[0] = ISO15765_FF | tmp;
		tx_buffer->frame.data[1] = (uint8_t)(size & 0xff);
		tx_buffer->frame.data[2] = msg->protocol;

		for (loop = 3; loop < CAN_DATA_LENGTH; loop++) {
			tx_buffer->frame.data[loop] = tx_buffer->data[tx_buffer->index++];
			tx_buffer->bytes_sent++;
		}
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Tx First Frame\n\r");
#endif
		tx_buffer->sequence = (tx_buffer->sequence + 1) % 0x0f;
		can_l2_tx_frame(&tx_buffer->frame);

		// Expect a FC frame in timely fasion
		startTimer_N_Bs(tx_buffer);
	}
	return(SUCCESS);
}

void exp_sendConsecutiveFrame(timer_t timer_id, union sigval data)
{
	uint8_t loop;
	tx_buffer_t *tx_buffer;

	/*
	 * Clear the compiler warning
	 */
	timer_id = timer_id;

	tx_buffer = (tx_buffer_t *)data.sival_ptr;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Tx Consecutive Frame tx Seq %d\n\r", tx_buffer->sequence);
#endif

	tx_buffer->consecutive_frame_timer.status = INACTIVE;

	if((tx_buffer->block_size == 0x00) || (tx_buffer->frames_sent_in_block < tx_buffer->block_size)) {
		tx_buffer->frame.data[0] = ISO15765_CF | (tx_buffer->sequence & 0x0f);

		for (loop = 0x01; loop < CAN_DATA_LENGTH; loop++) {
			tx_buffer->frame.data[loop] = tx_buffer->data[tx_buffer->index++];
			tx_buffer->bytes_sent++;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Bytes Sent %d Bytes to send %d\n\r", tx_buffer->bytes_sent, tx_buffer->bytes_to_send);
#endif
			if(tx_buffer->bytes_sent == tx_buffer->bytes_to_send) {
				loop++;
				break;
			}
		}

		tx_buffer->frame.can_dlc = loop;
		can_l2_tx_frame(&tx_buffer->frame);
		tx_buffer->sequence = (tx_buffer->sequence + 1) % 0x0f;
		tx_buffer->frames_sent_in_block++;

		if (tx_buffer->bytes_sent == tx_buffer->bytes_to_send) {
#if defined(XC16) || defined(__XC8)
			mcp_transmitter_busy = FALSE;
#elif defined(ES_LINUX)
			/*
			 * Free the Transmit buffer
			 */
			node_buffers[tx_buffer->destination].rx_buffer = NULL;
                        free(tx_buffer);
#endif
		} else {
			if(tx_buffer->frames_sent_in_block < tx_buffer->block_size)
				startConsecutiveFrameTimer(tx_buffer);
			else
				startTimer_N_Bs(tx_buffer);
		}
	}
}

void sendFlowControlFrame(rx_buffer_t *rx_buffer, uint8_t flowStatus)
{
	iso15765_id can_id;
	can_frame frame;

	if(flowStatus == FS_CTS || flowStatus == FS_Wait || flowStatus == FS_Overflow) {
		can_id.can_id = tx_frame_id.can_id;

		frame.can_dlc = 3;
		can_id.bytes.destination = rx_buffer->source;
	        frame.can_id = can_id.can_id;
		frame.data[0] = ISO15765_FC | (flowStatus & 0x0f);
		frame.data[1] = rx_buffer->block_size;
		frame.data[2] = rx_buffer->seperation_time;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Send Flow Control Frame\n\r");
#endif
		can_l2_tx_frame(&frame);
	} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_WARNING))
		LOG_W("Bad Flow Status\n\r");
#endif
	}
}

void iso15765_frame_handler(can_frame *frame)
{
	uint8_t type;
	uint8_t loop;
	uint8_t source;
	iso15765_id rx_msg_id;
	rx_buffer_t *rx_buffer;
	tx_buffer_t *tx_buffer;

	rx_msg_id.can_id = frame->can_id;

	if(rx_msg_id.bytes.destination != node_address) {
		// L3 Message but not for this node - Ignore it
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("ISO15765 Message not for this node\n\r");
#endif
		return;
	}

	source = rx_msg_id.bytes.source;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("iso15765_frame_handler(0x%lx) got a frame from 0x%x\n\r",frame->can_id, source);
#endif
	type = frame->data[0] & 0xf0;

	if(type == ISO15765_SF) {
		uint8_t length;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("SF\n\r");
#endif
#if defined(XC16) || defined(__XC8)
                rx_buffer = &mcp_rx_buffer;
		if(mcp_receiver_busy) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("ERROR: ISO15765 Received First Frame whilst RxBuffer Busy\n\r");
			return;
		}
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer != NULL) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("ISO15765 transmitter already busy\n\r");
#endif
			return;
		}

		rx_buffer = malloc(sizeof(rx_buffer_t));
		if(!rx_buffer) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Malloc Failed\n\r");
#endif
			exit(1);
		}
		node_buffers[source].rx_buffer = rx_buffer;
#endif // XC16 || __XC8 - ES_LINUX
		init_rx_buffer(rx_buffer);
		length = frame->data[0] & 0x0f;
		rx_buffer->source = source;
		rx_buffer->bytes_expected = length;

		if( (length > 0) && (length <= SINGLE_FRAME_SIZE)) {
			rx_buffer->index = 0;
			rx_buffer->protocol = frame->data[1];

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Rx Protocol %d L3 Length %d\n\r",
#endif
				   (uint16_t)rx_buffer->protocol,
				   (uint16_t)rx_buffer->bytes_expected);

			/*
			 * Subtract one from following loop for Protocol Byte
			 */
			for (loop = 2; loop < 2 + rx_buffer->bytes_expected -1; loop++) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("Rx Data byte %d - 0x%x\n\r",
#endif
					   rx_buffer->index,
					   frame->data[loop]);
				rx_buffer->data[rx_buffer->index++] = frame->data[loop];
			}

			rx_buffer->msg.protocol = rx_buffer->protocol;
			rx_buffer->msg.data = rx_buffer->data;
			rx_buffer->msg.size = rx_buffer->bytes_expected - 1;   // subtract one for Protocol Byte
			rx_buffer->msg.address = rx_buffer->source;

			dispatcher_iso15765_msg_handler(&rx_buffer->msg);
#if defined(XC16) || define(__XC8)
			mcp_receiver_busy = FALSE;
#elif defined(ES_LINUX)
			free(rx_buffer);
			node_buffers[source].rx_buffer = NULL;
#endif // XC16 || __XC8 - ES_LINUX
		}
		else {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Error in received length");
#endif
		}
	} else if(type == ISO15765_FF) {
		uint16_t size = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Rx First Frame\n\r");
#endif
#if defined(XC16) || defined(__XC8)
		if(mcp_receiver_busy) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("ERROR: Can L3 Received First Frame whilst RxBuffer Busy\n\r");
#endif
			return;
		}

		rx_buffer = &mcp_rx_buffer;
		mcp_receiver_busy = TRUE;
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer != NULL) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("ISO15765 transmitter already busy\n\r");
#endif
			return;
		}

		rx_buffer = malloc(sizeof(rx_buffer_t));
		if(!rx_buffer) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Malloc Failed\n\r");
#endif
			exit(1);
		}
		node_buffers[source].rx_buffer = rx_buffer;
#endif // XC16 || __XC8 - ES_LINUX
		init_rx_buffer(rx_buffer);
		//  Could not get this single line to work so had to split it into 3
		//        size = ((rxMsg->data[0] & 0x0f) << 8) | rxMsg->data[1];
		size = frame->data[0] & 0x0f;
		size = size << 8;
		size = size | frame->data[1];

		if (size > SYS_ISO15765_MAX_MSG + 1) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Message received overflows Max Size\n\r");
#endif
			sendFlowControlFrame(rx_buffer, FS_Overflow); //source
			return;
		}

		rx_buffer->source = source;
		rx_buffer->bytes_expected = (uint16_t)size - 1;   // Subtracl one for Protocol Byte
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Size expected %d\n\r", rx_buffer->bytes_expected);
#endif
		if(frame->can_dlc == 8) {
			rx_buffer->source = source;
			rx_buffer->protocol = frame->data[2];

			for (loop = 3; loop < frame->can_dlc; loop++) {
#if (defined(SYS_SERIAL_LOGGING) &* defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//                              LOG_D("Add Byte 0x%x\n\r", (UINT16)rxMsg->data[loop]);
#endif
				rx_buffer->data[rx_buffer->index++] = frame->data[loop];
				rx_buffer->bytes_received++;
			}
			rx_buffer->sequence = (rx_buffer->sequence + 1) % 0x0f;
			rx_buffer->frames_received_in_block = 0x00;

			sendFlowControlFrame(rx_buffer, FS_CTS);
		} else {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("expected a L2 Message of size 8\n\r");
#endif
		}
	} else if(type == ISO15765_CF) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Rx Consecutive Frame\n\r");
		for (loop = 0; loop < frame->can_dlc; loop++) {
			LOG_D("Add Byte %d 0x%x\n\r", loop, frame->data[loop]);
		}
#endif
#if defined(XC16) || defined(__XC8)
		// If the Receiver isn't busy not sure why we're gettting a CF
		if(!mcp_receiver_busy) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("ERROR: ISO15765 Received CF whilst RxBuffer NOT Busy\n\r");
#endif
			return;
		}

		rx_buffer = &mcp_rx_buffer;
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer == NULL) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("ISO15765 CF and NOT busy??\n\r");
#endif
			return;
		}

		rx_buffer = node_buffers[source].rx_buffer;
#endif // XC16 || __XC8 - ES_LINUX
		stopTimer_N_Cr(rx_buffer);

		if (rx_buffer->sequence == (frame->data[0] & 0x0f)) {
			for (loop = 1; loop < frame->can_dlc; loop++) {
				rx_buffer->data[rx_buffer->index++] = frame->data[loop];
				rx_buffer->bytes_received++;
			}
			rx_buffer->sequence = (rx_buffer->sequence + 1) % 0x0f;
			rx_buffer->frames_received_in_block++;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("received %d bytes expecting %d\n\r", rx_buffer->bytes_received, rx_buffer->bytes_expected);
#endif

			if (rx_buffer->bytes_received == rx_buffer->bytes_expected) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("Complete Message\n\r");
#endif
				rx_buffer->msg.protocol = rx_buffer->protocol;
				rx_buffer->msg.data = rx_buffer->data;
				rx_buffer->msg.size = rx_buffer->bytes_expected;
				rx_buffer->msg.address = rx_buffer->source;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("RX Msg from-0x%x, Protocol-0x%x, Size-0x%x\n\r",
#endif
					   (uint16_t)rx_buffer->msg.address,
					   (uint16_t)rx_buffer->msg.protocol,
					   (uint16_t)rx_buffer->msg.size);
				dispatcher_iso15765_msg_handler(&rx_buffer->msg);

				/*
				 * Compete L3 message received so Rx Buffer Available
				 */
#if defined(XC16) || defined(__XC8)
				mcp_receiver_busy = FALSE;
#elif defined(ES_LINUX)
				free(rx_buffer);
				node_buffers[source].rx_buffer = NULL;
#endif // XC16 || __XC8 - ES_LINUX
			} else if (rx_buffer->frames_received_in_block == rx_buffer->block_size) {
				rx_buffer->frames_received_in_block = 0;
				sendFlowControlFrame(rx_buffer, FS_CTS);

				// We'll be expecting another CF in a timely fasion
				startTimer_N_Cr(rx_buffer);
			} else {
				// We'll be expecting another CF in a timely fasion
				startTimer_N_Cr(rx_buffer);
			}
		} else {
#if defined(XC16) || defined(__XC8)
			mcp_receiver_busy = FALSE;
			rx_buffer->sequence = 0;
			rx_buffer->index = 0;
			rx_buffer->bytes_received = 0;
#elif defined(ES_LINUX)
			free(rx_buffer);
#endif

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Bad Sequence Number: expected 0x%x received 0x%x\n\r", rx_buffer->sequence, (frame->data[0] & 0x0f));
#endif
		}
	} else if(type == ISO15765_FC) {
		uint8_t flowStatus;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Rx Flow Control Frame: BlockSize %d, Seperation time %x\n\r", frame->data[1], frame->data[2]);
		LOG_D("BlockSize %d, Seperation time %x\n\r", frame->data[1], frame->data[2]);
#endif

#if defined(XC16) || defined(__XC8)
		// If the Receiver isn't busy not sure why we're gettting a CF
		if(!mcp_transmitter_busy) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("ERROR: ISO15765 Received FC whilst TxBuffer NOT Busy\n\r");
#endif
			return;
		}

		tx_buffer = &mcp_tx_buffer;
#elif defined(ES_LINUX)
		if(node_buffers[source].tx_buffer == NULL) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("ISO15765 FC and NOT busy??\n\r");
#endif
			return;
		}

		tx_buffer = node_buffers[source].tx_buffer;
#endif // XC16 || __XC8 - ES_LINUX

		stopTimer_N_Bs(tx_buffer);
		flowStatus = frame->data[0] & 0x0f;

		tx_buffer->block_size = frame->data[1];
		tx_buffer->seperation_time = frame->data[2];

		switch(flowStatus) {
		case FS_CTS:
			tx_buffer->frames_sent_in_block = 0x00;
			exp_sendConsecutiveFrame((timer_t)0xff, (union sigval)(void *)tx_buffer);
			break;
		case FS_Wait:
			//TODO Have to count and limit the number of Wait's we accept
			startTimer_N_Bs(tx_buffer);
			break;
		case FS_Overflow:
		default:
			/*
			 * Bad Flow Control so dump the TX Buffer
			 */
#if defined(XC16) || defined(__XC8)
			tx_buffer = &mcp_tx_buffer;
#elif defined(ES_LINUX)
			tx_buffer = node_buffers[source].tx_buffer;
#endif // XC16 || __XC8 - ES_LINUX

			if (tx_buffer->consecutive_frame_timer.status == ACTIVE)
				sw_timer_cancel(&tx_buffer->consecutive_frame_timer);
#if defined(XC16) || defined(__XC8)
			init_tx_buffer(tx_buffer);
			mcp_transmitter_busy = FALSE;
#elif defined(ES_LINUX)
			free(tx_buffer);
			node_buffers[source].tx_buffer = NULL;
#endif // XC16 || __XC8 - ES_LINUX
			break;
		}
	} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Unrecognised L3 CAN Frame type\n\r");
#endif
	}
}

void startConsecutiveFrameTimer(tx_buffer_t *tx_buffer)
{
	uint16_t ticks = 0x01;
	result_t result;

	if ((tx_buffer->seperation_time > 0x06) && (tx_buffer->seperation_time <= 0x7f)) {
		ticks = MILLI_SECONDS_TO_TICKS((uint16_t)tx_buffer->seperation_time);
	} else {
		ticks = MILLI_SECONDS_TO_TICKS((uint16_t)0x7f);
	}
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("startConsecutiveFrameTimer %d Ticks\n\r", ticks);
#endif
	result = sw_timer_start(ticks, exp_sendConsecutiveFrame, (union sigval)(void *)tx_buffer, &tx_buffer->consecutive_frame_timer);
	if(result != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to start N_Cr Timer\n\r");
#endif
	}
}

void startTimer_N_Cr(rx_buffer_t *rx_buffer)
{
	result_t result;

	if(rx_buffer->timer_N_Cr.status == ACTIVE) {
		sw_timer_cancel(&rx_buffer->timer_N_Cr);
	}
	result = sw_timer_start(MILLI_SECONDS_TO_TICKS(1000), exp_timer_N_Cr_Expired, (union sigval)(void *)rx_buffer, &rx_buffer->timer_N_Cr);

	if(result != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to start N_Cr Timer\n\r");
#endif
	}
}

void stopTimer_N_Cr(rx_buffer_t *rx_buffer)
{
	if(rx_buffer->timer_N_Cr.status == ACTIVE)
		sw_timer_cancel(&rx_buffer->timer_N_Cr);
}

void exp_timer_N_Cr_Expired(timer_t timer_id __attribute__((unused)), union sigval data)
{
	rx_buffer_t *rx_buffer;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("timer_N_Cr_Expired\n\r");
#endif
	rx_buffer = (rx_buffer_t *)data.sival_ptr;

	rx_buffer->timer_N_Cr.status = INACTIVE;
#if defined(XC16) || defined(__XC8)
	mcp_receiver_busy = FALSE;
	rx_buffer->sequence = 0;
	rx_buffer->index = 0;
	rx_buffer->bytes_received = 0;
#elif defined(ES_LINUX)
	free(rx_buffer);
#endif
}

void startTimer_N_Bs(tx_buffer_t *tx_buffer)
{
	result_t result;

	if(tx_buffer->timer_N_Bs.status == ACTIVE)
		sw_timer_cancel(&tx_buffer->timer_N_Bs);

	result = sw_timer_start(MILLI_SECONDS_TO_TICKS(1000), exp_timer_N_Bs_Expired, (union sigval)(void *)tx_buffer, &tx_buffer->timer_N_Bs);

	if(result != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to start N_Bs Timer\n\r");
#endif
	}
}

void stopTimer_N_Bs(tx_buffer_t *tx_buffer)
{
	if(tx_buffer->timer_N_Bs.status == ACTIVE)
		sw_timer_cancel(&tx_buffer->timer_N_Bs);
}

void exp_timer_N_Bs_Expired(timer_t timer_id __attribute__((unused)), union sigval data)
{
	tx_buffer_t *tx_buffer;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("timer_N_Bs_Expired\n\r");
#endif
	tx_buffer = (tx_buffer_t *)data.sival_ptr;

	tx_buffer->timer_N_Bs.status = INACTIVE;

	// Reset the Teansmitter
	tx_buffer->sequence = 0x00;
	tx_buffer->index = 0x00;

	if (tx_buffer->consecutive_frame_timer.status == ACTIVE)
		timer_cancel(&tx_buffer->consecutive_frame_timer);
#if defined(XC16) || defined(__XC8)
	mcp_transmitter_busy = FALSE;
#endif
}


void dispatcher_iso15765_msg_handler(iso15765_msg_t *message)
{
	uint16_t loop;
//	uint8_t  *data;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("ISO15765 Dis from-0x%x Protocol-0x%x len(0x%x)\n\r",
#endif
		   (uint16_t)message->address,
		   (uint16_t)message->protocol,
		   (uint16_t)message->size);
	for (loop = 0; loop < SYS_ISO15765_REGISTER_ARRAY_SIZE; loop++) {
		if (registered[loop].used && (message->protocol == registered[loop].protocol) ) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D(" => Dispatch\n\r");
#endif
			registered[loop].handler(message);
			return;
		}
	}
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D(" No Handler found for Protocol 0x%x\n\r", (uint16_t)message->protocol);
#endif
}

result_t iso15765_dispatch_reg_handler(iso15765_target_t *target)
{
	uint8_t loop;

	target->handler_id = 0xff;

#if (defined(SYS_SERIAL_LOGGING) && define(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("iso15765_dispatch_register_handler(0x%x)\n\r", (uint16_t)target->protocol);
#endif
	/*
	 * Find a free slot and add the Protocol
	 */
	for(loop = 0; loop < SYS_ISO15765_REGISTER_ARRAY_SIZE; loop++) {
		if(registered[loop].used == FALSE) {
			registered[loop].used = TRUE;
			registered[loop].protocol = target->protocol;
			registered[loop].handler = target->handler;
			target->handler_id = loop;
			return(SUCCESS);
		}
	}

#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
	LOG_E("ISO15765 Dispatch full!\n\r");
#endif
	return(ERR_NO_RESOURCES);
}

result_t iso15765_dispatch_unreg_handler(uint8_t id)
{
	if((id < SYS_ISO15765_REGISTER_ARRAY_SIZE) && (registered[id].used)) {
		registered[id].used = FALSE;
		registered[id].protocol = 0x00;
		registered[id].handler = (iso15765_msg_handler_t)NULL;
		return(SUCCESS);
	}
	return(ERR_BAD_INPUT_PARAMETER);
}

result_t iso15765_dispatch_set_unhandled_handler(iso15765_msg_handler_t handler)
{
	unhandled_handler = (iso15765_msg_handler_t)handler;
	return(SUCCESS);
}
