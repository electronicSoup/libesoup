/**
 * @file libesoup/comms/can/l3_iso15765-2.c
 *
 * @author John Whitmore
 *
 * @brief Based on ISO_15765-2 but with a much reduced Message Length. 
 * 
 * The standard allows for 4KB of a message whereas for the electronicSoup 
 * CAN Bus Nodes the limit is 74 Bytes, including an initial protocol byte.
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
 */
#if !defined(XC16) && !defined(__XC8) && !defined(ES_LINUX)
#error Unrecognised Compiler!
#endif

#include "libesoup_config.h"

#ifdef SYS_CAN_ISO15765

#include <stdio.h>
#include <string.h>
#if defined(ES_LINUX)
    #include <stdlib.h>
#endif


#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "ISO-15765";
#include "libesoup/logger/serial_log.h"
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  SYS_LOG_LEVEL
#endif // SYS_SERIAL_LOGGING

#include "libesoup/comms/can/can.h"
#include "libesoup/timers/sw_timers.h"

#ifndef SYS_CAN_ISO15765_REGISTER_ARRAY_SIZE
#error "libesoup_config.h should define a value for SYS_CAN_ISO15765_REGISTER_ARRAY_SIZE"
#endif

#ifndef SYS_CAN_ISO15765_MAX_MSG
#error "libesoup_config.h should define a value for SYS_CAN_ISO15765_MAX_MSG"
#endif

typedef struct
{
    uint8_t used;
    uint8_t protocol;
    iso15765_msg_handler_t handler;
} iso15765_register_t;

static iso15765_msg_handler_t unhandled_handler;

static iso15765_register_t registered[SYS_CAN_ISO15765_REGISTER_ARRAY_SIZE];
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

struct tx_buffer_t {
	uint8_t             block_size;
	uint8_t             seperation_time;
	can_frame           frame;
	uint8_t             sequence;
	uint8_t             data[SYS_CAN_ISO15765_MAX_MSG];
	uint16_t            index;
	uint8_t             frames_sent_in_block;
	uint16_t            bytes_to_send;
	uint16_t            bytes_sent;
	uint8_t             destination;
	timer_id            consecutive_frame_timer;
	timer_id            timer_N_Bs;
} tx_buffer_t;

typedef struct {
	uint8_t             block_size;
	uint8_t             seperation_time;
	uint8_t             data[SYS_CAN_ISO15765_MAX_MSG];
	uint16_t            index;
	uint8_t             sequence;
	uint8_t             protocol;
	uint16_t            bytes_expected;
	uint16_t            bytes_received;
	uint8_t             frames_received_in_block;
	uint8_t             source;
	can_frame           frame;
	iso15765_msg_t      msg;
	timer_id            timer_N_Cr;
} rx_buffer_t;

#if defined(XC16) || defined(__XC8)
static boolean            mcp_transmitter_busy = FALSE;
static boolean            mcp_receiver_busy = FALSE;
static rx_buffer_t        mcp_rx_buffer;
static struct tx_buffer_t mcp_tx_buffer;
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

static void init_tx_buffer(struct tx_buffer_t *);
static void init_rx_buffer(rx_buffer_t *);

static void exp_sendConsecutiveFrame(timer_id timer, union sigval);
static void sendFlowControlFrame(rx_buffer_t *rx_buffer, uint8_t flowStatus);
static void startConsecutiveFrameTimer(struct tx_buffer_t *tx_buffer) ;

static void startTimer_N_Cr(rx_buffer_t *);
static void exp_timer_N_Cr_Expired(timer_id timer, union sigval);

static void startTimer_N_Bs(struct tx_buffer_t *);
static void exp_timer_N_Bs_Expired(timer_id timer, union sigval);

void init_tx_buffer(struct tx_buffer_t *tx_buf)
{
	tx_buf->block_size               = 0;
	tx_buf->seperation_time          = 0;
	tx_buf->sequence                 = 0x00;
	tx_buf->index                    = 0x00;
	tx_buf->frames_sent_in_block     = 0;
	tx_buf->bytes_to_send            = 0x00;
	tx_buf->bytes_sent               = 0x00;
	tx_buf->destination              = 0x00;
	tx_buf->consecutive_frame_timer  = BAD_TIMER_ID;
	tx_buf->timer_N_Bs               = BAD_TIMER_ID;
}

void init_rx_buffer(rx_buffer_t *rx_buf)
{
	rx_buf->block_size               = BLOCK_SIZE;
	rx_buf->seperation_time          = SEPERATION_TIME;
	rx_buf->index                    = 0x00;
	rx_buf->sequence                 = 0x00;
	rx_buf->protocol                 = 0;
	rx_buf->bytes_expected           = 0;
	rx_buf->bytes_received           = 0;
	rx_buf->frames_received_in_block = 0x00;
	rx_buf->source                   = 0;
	rx_buf->timer_N_Cr               = BAD_TIMER_ID;
}

result_t iso15765_init(uint8_t address)
{
	result_t        rc;
	uint16_t        loop;
	can_l2_target_t target;

	for(loop = 0; loop < SYS_CAN_ISO15765_REGISTER_ARRAY_SIZE; loop++) {
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
	LOG_I("l3_init() node address = 0x%x\n\r", node_address);
	/*
	 * Initialise the static parts or our tx message header.
	 */
	tx_frame_id.bytes.layer3      = ISO15765_COMS;
	tx_frame_id.bytes.type        = ISO15765_TARGET_PHYSICAL;
	tx_frame_id.bytes.source      = node_address;
	tx_frame_id.bytes.destination = node_address;

	tx_frame_id.can_id |= CAN_EFF_FLAG;

	/*
	 * Define our target for Layer 2 Frames and register it.
	 */
	target.mask    = ISO15765_MASK;
	target.filter  = tx_frame_id.can_id & 0xffffff00; //Don't filter on the Source Byte
	target.handler = iso15765_frame_handler;

	rc = frame_dispatch_reg_handler(&target);
	RC_CHECK

	initialised = 0x01;
	return(0);
}

uint8_t iso15765_initialised(void)
{
    return(initialised);
}

result_t iso15765_tx_msg(iso15765_msg_t *msg)
{
	uint8_t                 *data_ptr;
	uint16_t          loop;
	iso15765_id       id;
	struct tx_buffer_t      *tx_buffer;
	uint16_t          size;
	uint8_t           tmp;

	LOG_I("Tx to 0x%x, Protocol-0x%x, len(0x%x)\n\r",
		   (uint16_t)msg->address,
		   (uint16_t)msg->protocol,
		   (uint16_t)msg->size);

	data_ptr = msg->data;
//	for(loop = 0; loop < msg->size; loop++) {
//		printf("0x%2x,", *data_ptr++);
//	}
//	printf("\n\r");

        if(!initialised) {
		LOG_E("ISO15765 not Initialised\n\r");
		return(-ERR_UNINITIALISED);
	}

	if(msg->size == 0) {
		LOG_E("ISO15765 Message Zero size not Sending\n\r");
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	if(msg->size > SYS_CAN_ISO15765_MAX_MSG) {
		LOG_E("L3_Can Message exceeds size limit\n\r");
		return(-ERR_BAD_INPUT_PARAMETER);
	}

#if defined(XC16) || defined(__XC8)
	if(mcp_transmitter_busy) {
		LOG_E("ISO15765 transmitter already busy\n\r");
		return(-ERR_BUSY);
	}
	tx_buffer = &mcp_tx_buffer;
	mcp_transmitter_busy = TRUE;
#elif defined(ES_LINUX)
	/*
	 * Check for a transmit buffer already active to the destination
	 */
	if(node_buffers[msg->address].tx_buffer != NULL) {
		LOG_E("ISO15765 transmitter already busy\n\r");
		return(-ERR_BUSY);
	}
	tx_buffer = malloc(sizeof(struct tx_buffer_t));
	if(!tx_buffer) {
		LOG_E("Malloc Failed\n\r");
		exit(1);
	}
	node_buffers[msg->address].tx_buffer = tx_buffer;
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
		LOG_D("Tx Single Frame\n\r");
		for(loop = 0; loop < msg->size; loop++) {
			tx_buffer->frame.data[loop + 2] = *data_ptr++;
			LOG_D("ISO15765 TX Byte 0x%x\n\r", tx_buffer->frame.data[loop + 2]);
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
#else
#error Unrecognised Compiler!
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
	LOG_D("Tx First Frame\n\r");
	tx_buffer->sequence = (tx_buffer->sequence + 1) % 0x0f;
	can_l2_tx_frame(&tx_buffer->frame);

	// Expect a FC frame in timely fasion
	startTimer_N_Bs(tx_buffer);
	return(0);
}

void exp_sendConsecutiveFrame(timer_id timer, union sigval data)
{
	uint8_t             loop;
	struct tx_buffer_t *tx_buffer;

	/*
	 * Clear the compiler warning
	 */
	timer = timer;

	tx_buffer = (struct tx_buffer_t *)data.sival_ptr;

	LOG_D("Tx Consecutive Frame tx Seq %d\n\r", tx_buffer->sequence);

	tx_buffer->consecutive_frame_timer = BAD_TIMER_ID;

	if((tx_buffer->block_size == 0x00) || (tx_buffer->frames_sent_in_block < tx_buffer->block_size)) {
		tx_buffer->frame.data[0] = ISO15765_CF | (tx_buffer->sequence & 0x0f);

		for (loop = 0x01; loop < CAN_DATA_LENGTH; loop++) {
			tx_buffer->frame.data[loop] = tx_buffer->data[tx_buffer->index++];
			tx_buffer->bytes_sent++;

			LOG_D("Bytes Sent %d Bytes to send %d\n\r", tx_buffer->bytes_sent, tx_buffer->bytes_to_send);
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
		LOG_D("Send Flow Control Frame\n\r");
		can_l2_tx_frame(&frame);
	} else {
		LOG_W("Bad Flow Status\n\r");
	}
}

void iso15765_frame_handler(can_frame *frame)
{
	result_t            rc;
	uint8_t             type;
	uint8_t             loop;
	uint8_t             source;
	iso15765_id         rx_msg_id;
	rx_buffer_t        *rx_buffer;
	struct tx_buffer_t *tx_buffer;

	rx_msg_id.can_id = frame->can_id;

	if(rx_msg_id.bytes.destination != node_address) {
		// L3 Message but not for this node - Ignore it
		LOG_D("ISO15765 Message not for this node\n\r");
		return;
	}

	source = rx_msg_id.bytes.source;

	LOG_D("iso15765_frame_handler(0x%lx) got a frame from 0x%x\n\r",frame->can_id, source);
	type = frame->data[0] & 0xf0;

	if(type == ISO15765_SF) {
		uint8_t length;

		LOG_D("SF\n\r");
#if defined(XC16) || defined(__XC8)
                rx_buffer = &mcp_rx_buffer;
		if(mcp_receiver_busy) {
			LOG_E("ERROR: ISO15765 Received First Frame whilst RxBuffer Busy\n\r");
			return;
		}
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer != NULL) {
			LOG_E("ISO15765 transmitter already busy\n\r");
			return;
		}

		rx_buffer = malloc(sizeof(rx_buffer_t));
		if(!rx_buffer) {
			LOG_E("Malloc Failed\n\r");
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

			LOG_D("Rx Protocol %d L3 Length %d\n\r",
				   (uint16_t)rx_buffer->protocol,
				   (uint16_t)rx_buffer->bytes_expected);

			/*
			 * Subtract one from following loop for Protocol Byte
			 */
			for (loop = 2; loop < 2 + rx_buffer->bytes_expected -1; loop++) {
				LOG_D("Rx Data byte %d - 0x%x\n\r",
					   rx_buffer->index,
					   frame->data[loop]);
				rx_buffer->data[rx_buffer->index++] = frame->data[loop];
			}

			rx_buffer->msg.protocol = rx_buffer->protocol;
			rx_buffer->msg.data = rx_buffer->data;
			rx_buffer->msg.size = rx_buffer->bytes_expected - 1;   // subtract one for Protocol Byte
			rx_buffer->msg.address = rx_buffer->source;

			dispatcher_iso15765_msg_handler(&rx_buffer->msg);
#if defined(XC16) || defined(__XC8)
			mcp_receiver_busy = FALSE;
#elif defined(ES_LINUX)
			free(rx_buffer);
			node_buffers[source].rx_buffer = NULL;
#endif // XC16 || __XC8 - ES_LINUX
		}
		else {
			LOG_E("Error in received length");
		}
	} else if(type == ISO15765_FF) {
		uint16_t size = 0;
		LOG_D("Rx First Frame\n\r");
#if defined(XC16) || defined(__XC8)
		if(mcp_receiver_busy) {
			LOG_E("ERROR: Can L3 Received First Frame whilst RxBuffer Busy\n\r");
			return;
		}

		rx_buffer = &mcp_rx_buffer;
		mcp_receiver_busy = TRUE;
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer != NULL) {
			LOG_E("ISO15765 transmitter already busy\n\r");
			return;
		}

		rx_buffer = malloc(sizeof(rx_buffer_t));
		if(!rx_buffer) {
			LOG_E("Malloc Failed\n\r");
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

		if (size > SYS_CAN_ISO15765_MAX_MSG + 1) {
			LOG_E("Message received overflows Max Size\n\r");
			sendFlowControlFrame(rx_buffer, FS_Overflow); //source
			return;
		}

		rx_buffer->source = source;
		rx_buffer->bytes_expected = (uint16_t)size - 1;   // Subtracl one for Protocol Byte
		LOG_D("Size expected %d\n\r", rx_buffer->bytes_expected);
		if(frame->can_dlc == 8) {
			rx_buffer->source = source;
			rx_buffer->protocol = frame->data[2];

			for (loop = 3; loop < frame->can_dlc; loop++) {
				rx_buffer->data[rx_buffer->index++] = frame->data[loop];
				rx_buffer->bytes_received++;
			}
			rx_buffer->sequence = (rx_buffer->sequence + 1) % 0x0f;
			rx_buffer->frames_received_in_block = 0x00;

			sendFlowControlFrame(rx_buffer, FS_CTS);
		} else {
			LOG_E("expected a L2 Message of size 8\n\r");
		}
	} else if(type == ISO15765_CF) {
		LOG_D("Rx Consecutive Frame\n\r");
		for (loop = 0; loop < frame->can_dlc; loop++) {
			LOG_D("Add Byte %d 0x%x\n\r", loop, frame->data[loop]);
		}
#if defined(XC16) || defined(__XC8)
		// If the Receiver isn't busy not sure why we're gettting a CF
		if(!mcp_receiver_busy) {
			LOG_E("ERROR: ISO15765 Received CF whilst RxBuffer NOT Busy\n\r");
			return;
		}

		rx_buffer = &mcp_rx_buffer;
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer == NULL) {
			LOG_E("ISO15765 CF and NOT busy??\n\r");
			return;
		}

		rx_buffer = node_buffers[source].rx_buffer;
#endif // XC16 || __XC8 - ES_LINUX
		rc = sw_timer_cancel(&rx_buffer->timer_N_Cr);
		RC_CHECK_PRINT_CONT("SW TIM CANCEL")

		if (rx_buffer->sequence == (frame->data[0] & 0x0f)) {
			for (loop = 1; loop < frame->can_dlc; loop++) {
				rx_buffer->data[rx_buffer->index++] = frame->data[loop];
				rx_buffer->bytes_received++;
			}
			rx_buffer->sequence = (rx_buffer->sequence + 1) % 0x0f;
			rx_buffer->frames_received_in_block++;

			LOG_D("received %d bytes expecting %d\n\r", rx_buffer->bytes_received, rx_buffer->bytes_expected);

			if (rx_buffer->bytes_received == rx_buffer->bytes_expected) {
				LOG_D("Complete Message\n\r");
				rx_buffer->msg.protocol = rx_buffer->protocol;
				rx_buffer->msg.data = rx_buffer->data;
				rx_buffer->msg.size = rx_buffer->bytes_expected;
				rx_buffer->msg.address = rx_buffer->source;

				LOG_D("RX Msg from-0x%x, Protocol-0x%x, Size-0x%x\n\r",
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

			LOG_D("Bad Sequence Number: expected 0x%x received 0x%x\n\r", rx_buffer->sequence, (frame->data[0] & 0x0f));
		}
	} else if(type == ISO15765_FC) {
		uint8_t flowStatus;
		LOG_D("Rx Flow Control Frame: BlockSize %d, Seperation time %x\n\r", frame->data[1], frame->data[2]);
		LOG_D("BlockSize %d, Seperation time %x\n\r", frame->data[1], frame->data[2]);

#if defined(XC16) || defined(__XC8)
		// If the Receiver isn't busy not sure why we're gettting a CF
		if(!mcp_transmitter_busy) {
			LOG_E("ERROR: ISO15765 Received FC whilst TxBuffer NOT Busy\n\r");
			return;
		}

		tx_buffer = &mcp_tx_buffer;
#elif defined(ES_LINUX)
		if(node_buffers[source].tx_buffer == NULL) {
			LOG_E("ISO15765 FC and NOT busy??\n\r");
			return;
		}

		tx_buffer = node_buffers[source].tx_buffer;
#endif // XC16 || __XC8 - ES_LINUX

		rc = sw_timer_cancel(&tx_buffer->timer_N_Bs);
		RC_CHECK_PRINT_CONT("SW TIM_Cancel")
		flowStatus = frame->data[0] & 0x0f;

		tx_buffer->block_size = frame->data[1];
		tx_buffer->seperation_time = frame->data[2];

		switch(flowStatus) {
		case FS_CTS:
			tx_buffer->frames_sent_in_block = 0x00;
			exp_sendConsecutiveFrame(BAD_TIMER_ID, (union sigval)(void *)tx_buffer);
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

			rc = sw_timer_cancel(&tx_buffer->consecutive_frame_timer);
			RC_CHECK_PRINT_CONT("SW TIM_Cancel")			
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
		LOG_D("Unrecognised L3 CAN Frame type\n\r");
	}
}

void startConsecutiveFrameTimer(struct tx_buffer_t *tx_buffer)
{
	result_t          rc;
	struct timer_req  request;
	
	request.units = mSeconds;
	if ((tx_buffer->seperation_time > 0x06) && (tx_buffer->seperation_time <= 0x7f)) {
		request.duration = tx_buffer->seperation_time;
	} else {
		request.duration = 0x7f;
	}
	request.type           = single_shot;
	request.exp_fn         = exp_sendConsecutiveFrame;
	request.data.sival_ptr = (void *)tx_buffer;
	LOG_D("startConsecutiveFrameTimer %d mS\n\r", request.duration);
	rc = sw_timer_start(&request);
	RC_CHECK_PRINT_VOID("Failed to start frame Timer\n\r");
	tx_buffer->consecutive_frame_timer = rc;
}

void startTimer_N_Cr(rx_buffer_t *rx_buffer)
{
	result_t          rc;
	struct timer_req  request;

	request.units          = mSeconds;
	request.duration       = 1000;
	request.type           = single_shot;
	request.exp_fn         = exp_timer_N_Cr_Expired;
	request.data.sival_ptr = (void *)rx_buffer;

	rc = sw_timer_cancel(&rx_buffer->timer_N_Cr);
	RC_CHECK_PRINT_VOID("SW TIM_Cancel")
	
	rc = sw_timer_start(&request);
	RC_CHECK_PRINT_VOID("Failed to start N_Cr Timer\n\r");
	rx_buffer->timer_N_Cr = rc;
}

void exp_timer_N_Cr_Expired(timer_id timer __attribute__((unused)), union sigval data)
{
	rx_buffer_t *rx_buffer;

	LOG_D("timer_N_Cr_Expired\n\r");
	rx_buffer = (rx_buffer_t *)data.sival_ptr;

	rx_buffer->timer_N_Cr = BAD_TIMER_ID;
#if defined(XC16) || defined(__XC8)
	mcp_receiver_busy = FALSE;
	rx_buffer->sequence = 0;
	rx_buffer->index = 0;
	rx_buffer->bytes_received = 0;
#elif defined(ES_LINUX)
	free(rx_buffer);
#endif
}

void startTimer_N_Bs(struct tx_buffer_t *tx_buffer)
{
	result_t          rc;
	struct timer_req  request;

	rc = sw_timer_cancel(&tx_buffer->timer_N_Bs);
	RC_CHECK_PRINT_CONT("SW TIM_Cancel")

	request.units          = mSeconds;
	request.duration       = 1000;
	request.type           = single_shot;
	request.exp_fn         = exp_timer_N_Bs_Expired;
	request.data.sival_ptr = (void *)tx_buffer;

	rc = sw_timer_start(&request);
	RC_CHECK_PRINT_VOID("Failed to start N_Bs Timer\n\r");
	tx_buffer->timer_N_Bs = rc;
}

void exp_timer_N_Bs_Expired(timer_id timer __attribute__((unused)), union sigval data)
{
	result_t            rc;
	struct tx_buffer_t *tx_buffer;

	LOG_D("timer_N_Bs_Expired\n\r");
	tx_buffer = (struct tx_buffer_t *)data.sival_ptr;

	tx_buffer->timer_N_Bs = BAD_TIMER_ID;

	// Reset the Teansmitter
	tx_buffer->sequence = 0x00;
	tx_buffer->index = 0x00;

	rc = sw_timer_cancel(&tx_buffer->consecutive_frame_timer);
	RC_CHECK_PRINT_CONT("SW TIM_Cancel")

#if defined(XC16) || defined(__XC8)
	mcp_transmitter_busy = FALSE;
#endif
}


void dispatcher_iso15765_msg_handler(iso15765_msg_t *message)
{
	uint16_t loop;

	LOG_I("ISO15765 Dis from-0x%x Protocol-0x%x len(0x%x)\n\r",
		   (uint16_t)message->address,
		   (uint16_t)message->protocol,
		   (uint16_t)message->size);
	for (loop = 0; loop < SYS_CAN_ISO15765_REGISTER_ARRAY_SIZE; loop++) {
		if (registered[loop].used && (message->protocol == registered[loop].protocol) ) {
			LOG_D(" => Dispatch\n\r");
			registered[loop].handler(message);
			return;
		}
	}
	LOG_D(" No Handler found for Protocol 0x%x\n\r", (uint16_t)message->protocol);
}

result_t iso15765_dispatch_reg_handler(iso15765_target_t *target)
{
	uint8_t loop;

	target->handler_id = 0xff;

	LOG_I("iso15765_dispatch_register_handler(0x%x)\n\r", (uint16_t)target->protocol);
	/*
	 * Find a free slot and add the Protocol
	 */
	for(loop = 0; loop < SYS_CAN_ISO15765_REGISTER_ARRAY_SIZE; loop++) {
		if(registered[loop].used == FALSE) {
			registered[loop].used = TRUE;
			registered[loop].protocol = target->protocol;
			registered[loop].handler = target->handler;
			target->handler_id = loop;
			return(0);
		}
	}

	LOG_E("ISO15765 Dispatch full!\n\r");
	return(-ERR_NO_RESOURCES);
}

result_t iso15765_dispatch_unreg_handler(uint8_t id)
{
	if((id < SYS_CAN_ISO15765_REGISTER_ARRAY_SIZE) && (registered[id].used)) {
		registered[id].used = FALSE;
		registered[id].protocol = 0x00;
		registered[id].handler = (iso15765_msg_handler_t)NULL;
		return(0);
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

result_t iso15765_dispatch_set_unhandled_handler(iso15765_msg_handler_t handler)
{
	unhandled_handler = (iso15765_msg_handler_t)handler;
	return(0);
}

#endif // SYS_CAN_ISO15765
