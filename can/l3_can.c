/**
 *
 * \file es_lib/can/l3_can.c
 *
 * Based on ISO_15765-2 but with a much reduced Message Length. The standard allows for 4KB
 * of a message whereas for the electronicSoup CAN Bus Nodes the limit is 74 Bytes.
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
#include <stdio.h>
#include <string.h>
#if defined(ES_LINUX)
    #include <stdlib.h>
#endif

#include "system.h"
#include "es_lib/can/es_can.h"
#include "es_lib/can/dcncp/dcncp.h"
#include "es_lib/timers/timers.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "CAN_L3"

typedef struct
{
    u8 used;
    u8 protocol;
    can_l3_msg_handler_t handler;
} L3_CanRegister;

static L3_CanRegister registered[CAN_L3_REGISTER_ARRAY_SIZE];
static void dispatcher_l3_msg_handler(can_l3_msg_t *message);

#define L3_SINGLE_FRAME_SIZE 7

/*
 * The CAN ID as used by the Layer 3 Protocol
 *
 *   28..26  25  24  |   23..16   |  15..8  |  7..0  |
 *    110     0    0 | Target Type| Target  | Source |
 */
typedef union
{
    struct
    {
        u8 source;
        u8 destination;
        u8 type;
        u8 layer3;
    } bytes;
    u32 can_id;
} l3_can_id;

#define L3_TARGET_PHYSICAL   218
#define L3_TARGET_FUNCTIONAL 219

#define L3_COMS 0x18

#define L3_SF 0x00
#define L3_FF 0x10
#define L3_CF 0x20
#define L3_FC 0x30

#define BLOCK_SIZE 2
#define SEPERATION_TIME 0x25

#define L3_EXTENDED TRUE
#define L3_MASK     0x1ffeff00
//#define L3_FILTER   0x18da0000

#define FS_CTS 0x00
#define FS_Wait 0x01
#define FS_Overflow 0x02

static l3_can_id tx_frame_id;

typedef struct {
	u8 block_size;
	u8 seperation_time;
	can_frame frame;
	u8 sequence;
	u8 data[L3_CAN_MAX_MSG];
	u8 index;
	u8 frames_sent_in_block;
	u8 bytes_to_send;
	u8 bytes_sent;
	u8 destination;
	es_timer consecutive_frame_timer;
	es_timer timer_N_Bs;
} tx_buffer_t;

typedef struct {
	u8 block_size;
	u8 seperation_time;
	u8 data[L3_CAN_MAX_MSG];
	u8 index;
	u8 sequence;
	u8 protocol;
	u8 bytes_expected;
	u8 bytes_received;
	u8 frames_received_in_block;
	u8 source;
	can_frame frame;
	can_l3_msg_t l3_msg;
	es_timer timer_N_Cr;
} rx_buffer_t;

#if defined(MCP)
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
 */
static can_status_t status;
static void (*status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud) = NULL;

/*
 * This node's local Layer 3 Node Address
 */
static u8 node_address;

static void l3_l2_frame_handler(can_frame *rxMsg);

static void init_tx_buffer(tx_buffer_t *);
static void init_rx_buffer(rx_buffer_t *);

static void exp_sendConsecutiveFrame(timer_t timer_id, union sigval);
static void sendFlowControlFrame(rx_buffer_t *rx_buffer, u8 flowStatus);
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

result_t l3_init(void (*arg_status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud))
{
	u16 loop;
	can_l2_target_t target;

        status.byte = 0x00;
        status_handler = arg_status_handler;

	for(loop = 0; loop < CAN_L3_REGISTER_ARRAY_SIZE; loop++) {
		registered[loop].used = FALSE;
		registered[loop].protocol = 0x00;
		registered[loop].handler = (can_l3_msg_handler_t)NULL;
	}

#if defined(MCP)
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
        node_address = dcncp_get_can_l3_address();
	LOG_D("l3_init() node address = 0x%x\n\r", node_address);

	/*
	 * Initialise the static parts or our tx message header.
	 */
	tx_frame_id.bytes.layer3 = L3_COMS;
	tx_frame_id.bytes.type = L3_TARGET_PHYSICAL;
	tx_frame_id.bytes.source = node_address;
	tx_frame_id.bytes.destination = node_address;
    
	tx_frame_id.can_id |= CAN_EFF_FLAG;

	/*
	 * Define our target for Layer 2 Frames and register it.
	 */
	target.mask = (u32) (L3_MASK | CAN_EFF_FLAG);
	target.filter = tx_frame_id.can_id & 0xffff00ff; //Don't filter on the Source Byte
	target.handler = l3_l2_frame_handler;

	can_l2_reg_handler(&target);

	status.bit_field.l3_status = 1;

        if(status_handler)
			status_handler(L3_STATUS_MASK, status, no_baud);

	return(SUCCESS);
}

BOOL l3_initialised(void)
{
    return(status.bit_field.l3_status);
}

result_t l3_tx_msg(can_l3_msg_t *msg)
{
	u8 *dataPtr;
	u8 loop;
	l3_can_id can_id;
	tx_buffer_t *tx_buffer;

	LOG_D("Tx to 0x%x, Protocol-0x%x, len(0x%x)\n\r",
		   (u16)msg->address,
		   (u16)msg->protocol,
		   (u16)msg->size);

	if (!status.bit_field.l3_status) {
		LOG_E("L3_Can not Initialised\n\r");
		return(ERR_GENERAL_L3_ERROR);
	}

	if(msg->size == 0) {
		LOG_E("L3_Can Message Zero size not Sending\n\r");
		return(ERR_L3_ZERO_LENGTH);
	}

	if(msg->size > L3_CAN_MAX_MSG) {
		LOG_E("L3_Can Message exceeds size limit\n\r");
		return(ERR_L3_MAX_LENGTH);
	}

#if defined(MCP)
	if(mcp_transmitter_busy) {
		LOG_E("L3_Can transmitter already busy\n\r");
		return(ERR_L3_TX_BUSY);
	}
	tx_buffer = &mcp_tx_buffer;
#elif defined(ES_LINUX)
	/*
	 * Check for a transmit buffer already active to the destination
	 */
	if(node_buffers[msg->address].tx_buffer != NULL) {
		DEBUG_E("L3_Can transmitter already busy\n\r");
		return(ERR_L3_TX_BUSY);
	}

	tx_buffer = malloc(sizeof(tx_buffer_t));
	if(!tx_buffer) {
		DEBUG_E("Malloc Failed\n\r");
		exit(1);
	}
	node_buffers[msg->address].tx_buffer = tx_buffer;
#endif
	init_tx_buffer(tx_buffer);

	/*
	 * cut off for the single frame message.
	 */
	if(msg->size + 1 <= L3_SINGLE_FRAME_SIZE ) { // +1 for Protocol Byte 
		tx_buffer->frame.can_dlc = msg->size + 2;
		can_id.can_id = tx_frame_id.can_id;
		can_id.bytes.destination = msg->address;
		tx_buffer->frame.can_id = can_id.can_id;

		tx_buffer->frame.data[0] = L3_SF | ((msg->size + 1) & 0x0f);
		tx_buffer->frame.data[1] = msg->protocol;

		dataPtr = msg->data;
		for(loop = 2; loop <= msg->size; loop++) {
			tx_buffer->frame.data[loop] = *dataPtr++;
		}
		LOG_D("Tx Single Frame\n\r");
		can_l2_tx_frame(&(tx_buffer->frame));
	} else {
		/*
		 * Copy the l3 message to be sent into the Trasmit buffer.
		 */
		memcpy((void*)tx_buffer->data, (void *)msg->data, msg->size);
		tx_buffer->bytes_to_send = msg->size;
		tx_buffer->bytes_sent = 0x00;
		tx_buffer->destination = msg->address;
#if defined(MCP)
		mcp_transmitter_busy = TRUE;
#endif
		// Fill in the can id we're going to use for the transmission.
		can_id = tx_frame_id;
		can_id.bytes.destination = tx_buffer->destination;

		tx_buffer->frame.can_id = can_id.can_id;
	        tx_buffer->frame.can_dlc = CAN_DATA_LENGTH;
		tx_buffer->frame.data[0] = L3_FF;
		tx_buffer->frame.data[1] = msg->size + 1; // Add one for Protocol Byte
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
	}
	return(SUCCESS);
}

void exp_sendConsecutiveFrame(timer_t timer_id __attribute__((unused)), union sigval data)
{
	u8 loop;
	tx_buffer_t *tx_buffer;

	tx_buffer = (tx_buffer_t *)data.sival_ptr;

	LOG_D("Tx Consecutive Frame tx Seq %d\n\r", tx_buffer->sequence);

	tx_buffer->consecutive_frame_timer.status = INACTIVE;

	if((tx_buffer->block_size == 0x00) || (tx_buffer->frames_sent_in_block < tx_buffer->block_size)) {
		tx_buffer->frame.data[0] = L3_CF | (tx_buffer->sequence & 0x0f);

		for (loop = 0x01; loop < CAN_DATA_LENGTH; loop++) {
			tx_buffer->frame.data[loop] = tx_buffer->data[tx_buffer->index++];
			tx_buffer->bytes_sent++;

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
#if defined(MCP)
			mcp_transmitter_busy = FALSE;
#elif defined(ES_LINUX)
			/*
			 * Free the Transmit buffer
			 */
			node_buffers[the destination].rx_buffer = NULL;
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

void sendFlowControlFrame(rx_buffer_t *rx_buffer, u8 flowStatus)
{
	l3_can_id can_id;
	can_frame frame;

	if(flowStatus == FS_CTS || flowStatus == FS_Wait || flowStatus == FS_Overflow) {
		can_id.can_id = tx_frame_id.can_id;

		frame.can_dlc = 3;
		can_id.bytes.destination = rx_buffer->source;
	        frame.can_id = can_id.can_id;
		frame.data[0] = L3_FC | (flowStatus & 0x0f);
		frame.data[1] = rx_buffer->block_size;
		frame.data[2] = rx_buffer->seperation_time;
		LOG_D("Send Flow Control Frame\n\r");
		can_l2_tx_frame(&frame);
	} else {
		LOG_W("Bad Flow Status\n\r");
	}
}

void l3_l2_frame_handler(can_frame *rxMsg)
{
	u8 type;
	u8 loop;
	u8 source;
	l3_can_id rx_msg_id;
	rx_buffer_t *rx_buffer;
	tx_buffer_t *tx_buffer;

	rx_msg_id.can_id = rxMsg->can_id;

	if(rx_msg_id.bytes.destination != node_address) {
		// L3 Message but not for this node - Ignore it
		LOG_D("L3 Message not for this node\n\r");
		return;
	}

	source = rx_msg_id.bytes.source;

	LOG_D("l3_l2_frame_handler() got a frame from 0x%x\n\r", source);
	type = rxMsg->data[0] & 0xf0;

	if(type == L3_SF) {
		u8 l3Len;

		LOG_D("SF\n\r");
#if defined(MCP)
                rx_buffer = &mcp_rx_buffer;
		if(mcp_receiver_busy) {
			LOG_E("ERROR: Can L3 Received First Frame whilst RxBuffer Busy\n\r");
			return;
		}
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer != NULL) {
			DEBUG_E("L3_Can transmitter already busy\n\r");
			return;
		}

		rx_buffer = malloc(sizeof(rx_buffer_t));
		if(!rx_buffer) {
			DEBUG_E("Malloc Failed\n\r");
			exit(1);
		}
		node_buffers[source].rx_buffer = rx_buffer;
#endif // MCP - ES_LINUX
		init_rx_buffer(rx_buffer);
		l3Len = rxMsg->data[0] & 0x0f;
		rx_buffer->bytes_expected = l3Len;

		if( (l3Len > 0) && (l3Len <= L3_SINGLE_FRAME_SIZE)) {
			rx_buffer->index = 0;
			rx_buffer->protocol = rxMsg->data[1];

			LOG_D("Rx Protocol %d L3 Length %d\n\r",
				   (u16)rx_buffer->protocol,
				   (u16)rx_buffer->bytes_expected);

			/*
			 * Subtract one from following loop for Protocol Byte
			 */
			for (loop = 2; loop < 2 + rx_buffer->bytes_expected -1; loop++) {
				LOG_D("Rx Data byte %d - 0x%x\n\r",
					   (u16)rx_buffer->index,
					   (u16)rxMsg->data[loop]);
				rx_buffer->data[rx_buffer->index++] = rxMsg->data[loop];
			}

			rx_buffer->l3_msg.protocol = rx_buffer->protocol;
			rx_buffer->l3_msg.data = rx_buffer->data;
			rx_buffer->l3_msg.size = rx_buffer->bytes_expected;   // subtract one for Protocol Byte
			rx_buffer->l3_msg.address = rx_buffer->source;

			dispatcher_l3_msg_handler(&rx_buffer->l3_msg);
#if defined(MCP)
			mcp_receiver_busy = FALSE;
#elif defined(ES_LINUX)
			free(rx_buffer);
			node_buffers[source].rx_buffer = NULL;			
#endif // MCP - ES_LINUX
		}
		else {
			LOG_E("Error in received length");
		}
	} else if(type == L3_FF) {
		u16 size = 0;
		LOG_D("Rx First Frame\n\r");
#if defined(MCP)
		if(mcp_receiver_busy) {
			LOG_E("ERROR: Can L3 Received First Frame whilst RxBuffer Busy\n\r");
			return;
		}

		rx_buffer = &mcp_rx_buffer;
		mcp_receiver_busy = TRUE;
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer != NULL) {
			LOG_E("L3_Can transmitter already busy\n\r");
			return;
		}

		rx_buffer = malloc(sizeof(rx_buffer_t));
		if(!rx_buffer) {
			LOG_E("Malloc Failed\n\r");
			exit(1);
		}
		node_buffers[source].rx_buffer = rx_buffer;
#endif // MCP - ES_LINUX
		init_rx_buffer(rx_buffer);
		//  Could not get this single line to work so had to split it into 3
		//        size = ((rxMsg->data[0] & 0x0f) << 8) | rxMsg->data[1];
		size = rxMsg->data[0] & 0x0f;
		size = size << 8;
		size = size | rxMsg->data[1];

		if (size > L3_CAN_MAX_MSG + 1) {
			LOG_E("Message received overflows Max Size\n\r");
			sendFlowControlFrame(rx_buffer, FS_Overflow); //source
			return;
		}

		rx_buffer->bytes_expected = (u8)size - 1;   // Subtracl one for Protocol Byte
		LOG_D("Size expected %d\n\r", rx_buffer->bytes_expected);
		if(rxMsg->can_dlc == 8) {
			rx_buffer->source = source;
			rx_buffer->protocol = rxMsg->data[2];

			for (loop = 3; loop < rxMsg->can_dlc; loop++) {
//                              DEBUG_D("Add Byte 0x%x\n\r", (UINT16)rxMsg->data[loop]);
				rx_buffer->data[rx_buffer->index++] = rxMsg->data[loop];
				rx_buffer->bytes_received++;
			}
			rx_buffer->sequence = (rx_buffer->sequence + 1) % 0x0f;
			rx_buffer->frames_received_in_block = 0x00;

			sendFlowControlFrame(rx_buffer, FS_CTS);
		} else {
			LOG_E("expected a L2 Message of size 8\n\r");
		}
	} else if(type == L3_CF) {
		LOG_D("Rx Consecutive Frame\n\r");
		for (loop = 0; loop < rxMsg->can_dlc; loop++)
			LOG_D("Add Byte %d 0x%x\n\r", loop, rxMsg->data[loop]);

#if defined(MCP)
		// If the Receiver isn't busy not sure why we're gettting a CF
		if(!mcp_receiver_busy) {
             LOG_E("ERROR: Can L3 Received CF whilst RxBuffer NOT Busy\n\r");
			return;
		}

		rx_buffer = &mcp_rx_buffer;
#elif defined(ES_LINUX)
		if(node_buffers[source].rx_buffer == NULL) {
			DEBUG_E("L3_Can CF and NOT busy??\n\r");
			return;
		}

		rx_buffer = node_buffers[source].rx_buffer;
#endif // MCP - ES_LINUX
		stopTimer_N_Cr(rx_buffer);
		
		LOG_D("Compare Seq Numbers 0x%x=0x%x?\n\r", rx_buffer->sequence, (rxMsg->data[0] & 0x0f));
		if (rx_buffer->sequence == (rxMsg->data[0] & 0x0f)) {
			for (loop = 1; loop < rxMsg->can_dlc; loop++) {
//                    DEBUG_D("Add Byte 0x%x\n\r", (UINT16) rxMsg->data[loop]);
				rx_buffer->data[rx_buffer->index++] = rxMsg->data[loop];
				rx_buffer->bytes_received++;
			}
			rx_buffer->sequence = (rx_buffer->sequence + 1) % 0x0f;
			rx_buffer->frames_received_in_block++;

			if (rx_buffer->bytes_received == rx_buffer->bytes_expected) {
			LOG_D("Complete Message\n\r");
				rx_buffer->l3_msg.protocol = rx_buffer->protocol;
				rx_buffer->l3_msg.data = rx_buffer->data;
				rx_buffer->l3_msg.size = rx_buffer->bytes_expected;
				rx_buffer->l3_msg.address = rx_buffer->source;

				LOG_D("RX Msg from-0x%x, Protocol-0x%x, Size-0x%x\n\r",
					   (u16)rx_buffer->l3_msg.address,
					   (u16)rx_buffer->l3_msg.protocol,
					   (u16)rx_buffer->l3_msg.size);
				dispatcher_l3_msg_handler(&rx_buffer->l3_msg);

				/*
				 * Compete L3 message received so Rx Buffer Available
				 */
#if defined(MCP)
				mcp_receiver_busy = FALSE;
#elif defined(ES_LINUX)
				free(rx_buffer);
				node_buffers[source].rx_buffer = NULL;			
#endif // MCP - ES_LINUX
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
#if defined(MCP)
			mcp_receiver_busy = FALSE;
			rx_buffer->sequence = 0;
			rx_buffer->index = 0;
			rx_buffer->bytes_received = 0;
#elif defined(ES_LINUX)
			free(rx_buffer);
#endif

			LOG_D("Bad Sequence Number: expected 0x%x received 0x%x\n\r", rx_buffer->sequence, (rxMsg->data[0] & 0x0f));
		}
	} else if(type == L3_FC) {
		u8 flowStatus;
		LOG_D("Rx Flow Control Frame: BlockSize %d, Seperation time %x\n\r", rxMsg->data[1], rxMsg->data[2]);
		LOG_D("BlockSize %d, Seperation time %x\n\r", rxMsg->data[1], rxMsg->data[2]);

#if defined(MCP)
		// If the Receiver isn't busy not sure why we're gettting a CF
		if(!mcp_transmitter_busy) {
			LOG_E("ERROR: Can L3 Received FC whilst TxBuffer NOT Busy\n\r");
			return;
		}

		tx_buffer = &mcp_tx_buffer;
#elif defined(ES_LINUX)
		if(node_buffers[source].tx_buffer == NULL) {
			LOG_E("L3_Can FC and NOT busy??\n\r");
			return;
		}

		tx_buffer = node_buffers[source].tx_buffer;
#endif // MCP - ES_LINUX

		stopTimer_N_Bs(tx_buffer);
		flowStatus = rxMsg->data[0] & 0x0f;

		tx_buffer->block_size = rxMsg->data[1];
		tx_buffer->seperation_time = rxMsg->data[2];

		switch(flowStatus) {
		case FS_CTS:
			tx_buffer->frames_sent_in_block = 0x00;
			exp_sendConsecutiveFrame(0xff, (union sigval)(void *)tx_buffer);
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
#if defined(MCP)
			tx_buffer = &mcp_tx_buffer;
#elif defined(ES_LINUX)
			tx_buffer = node_buffers[source].tx_buffer;
#endif // MCP - ES_LINUX

			if (tx_buffer->consecutive_frame_timer.status == ACTIVE)
				timer_cancel(&tx_buffer->consecutive_frame_timer);
#if defined(MCP)
			init_tx_buffer(tx_buffer);
			mcp_transmitter_busy = FALSE;
#elif defined(ES_LINUX)
			free(tx_buffer);
			node_buffers[source].tx_buffer = NULL;
#endif // MCP - ES_LINUX
			break;
		}
	} else {
		LOG_D("Unrecognised L3 CAN Frame type\n\r");
	}
}

void startConsecutiveFrameTimer(tx_buffer_t *tx_buffer)
{
	u16 ticks = 0x01;
	result_t result;

	if ((tx_buffer->seperation_time > 0x06) && (tx_buffer->seperation_time <= 0x7f)) {
		ticks = MILLI_SECONDS_TO_TICKS((u16)tx_buffer->seperation_time);
	} else {
		ticks = MILLI_SECONDS_TO_TICKS((u16)0x7f);
	}
	LOG_D("startConsecutiveFrameTimer %d Ticks\n\r", ticks);
	result = timer_start(ticks, exp_sendConsecutiveFrame, (union sigval)(void *)tx_buffer, &tx_buffer->consecutive_frame_timer);
	if(result != SUCCESS) {
		LOG_E("Failed to start N_Cr Timer\n\r");
	}
}

void startTimer_N_Cr(rx_buffer_t *rx_buffer)
{
	result_t result;

	if(rx_buffer->timer_N_Cr.status == ACTIVE) {
		timer_cancel(&rx_buffer->timer_N_Cr);
	}
	result = timer_start(MILLI_SECONDS_TO_TICKS(1000), exp_timer_N_Cr_Expired, (union sigval)(void *)rx_buffer, &rx_buffer->timer_N_Cr);

	if(result != SUCCESS) {
		LOG_E("Failed to start N_Cr Timer\n\r");
	}
}

void stopTimer_N_Cr(rx_buffer_t *rx_buffer)
{
	if(rx_buffer->timer_N_Cr.status == ACTIVE)
		timer_cancel(&rx_buffer->timer_N_Cr);
}

void exp_timer_N_Cr_Expired(timer_t timer_id __attribute__((unused)), union sigval data)
{
	rx_buffer_t *rx_buffer;

	LOG_D("timer_N_Cr_Expired\n\r");
	rx_buffer = (rx_buffer_t *)data.sival_ptr;

	rx_buffer->timer_N_Cr.status = INACTIVE;
#if defined(MCP)
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
		timer_cancel(&tx_buffer->timer_N_Bs);

	result = timer_start(MILLI_SECONDS_TO_TICKS(1000), exp_timer_N_Bs_Expired, (union sigval)(void *)tx_buffer, &tx_buffer->timer_N_Bs);

	if(result != SUCCESS) {
		LOG_E("Failed to start N_Bs Timer\n\r");
	}
}

void stopTimer_N_Bs(tx_buffer_t *tx_buffer)
{
	if(tx_buffer->timer_N_Bs.status == ACTIVE)
		timer_cancel(&tx_buffer->timer_N_Bs);
}

void exp_timer_N_Bs_Expired(timer_t timer_id __attribute__((unused)), union sigval data)
{
	tx_buffer_t *tx_buffer;

	LOG_D("timer_N_Bs_Expired\n\r");
	tx_buffer = (tx_buffer_t *)data.sival_ptr;

	tx_buffer->timer_N_Bs.status = INACTIVE;

	// Reset the Teansmitter
	tx_buffer->sequence = 0x00;
	tx_buffer->index = 0x00;

	if (tx_buffer->consecutive_frame_timer.status == ACTIVE)
		timer_cancel(&tx_buffer->consecutive_frame_timer);
#if defined(MCP)
	mcp_transmitter_busy = FALSE;
#endif
}


void dispatcher_l3_msg_handler(can_l3_msg_t *message)
{
	u8 loop;

	LOG_D("L3 Dis from-0x%x Protocol-0x%x len(0x%x)\n\r",
		   (u16)message->address,
		   (u16)message->protocol,
		   (u16)message->size);

	for (loop = 0; loop < CAN_L3_REGISTER_ARRAY_SIZE; loop++) {
		if (registered[loop].used && (message->protocol == registered[loop].protocol) ) {
			LOG_D(" => Dispatch\n\r");
			registered[loop].handler(message);
			return;
		}
	}
	LOG_D(" No Handler found for Protocol 0x%x\n\r", (u16)message->protocol);
}

result_t l3_register_handler(u8 protocol, can_l3_msg_handler_t handler)
{
	u8 loop;
	LOG_D("l3_can_dispatch_register_handler(0x%x)\n\r", (u16)protocol);

	/*
	 * Check is there already a handler for the Protocol
	 */
	for(loop = 0; loop < CAN_L3_REGISTER_ARRAY_SIZE; loop++) {
		if(  (registered[loop].used == TRUE)
		     &&(registered[loop].protocol == protocol)) {
			LOG_D("Replacing existing handler for Protocol 0x%x\n\r", (u16)protocol);
			registered[loop].handler = handler;
			return(SUCCESS);
		}
	}

	/*
	 * Find a free slot and add the Protocol
	 */
	for(loop = 0; loop < CAN_L3_REGISTER_ARRAY_SIZE; loop++) {
		if(registered[loop].used == FALSE) {
			registered[loop].used = TRUE;
			registered[loop].protocol = protocol;
			registered[loop].handler = handler;
			return(SUCCESS);
		}
	}

	LOG_E("L3 Dispatch full!\n\r");
	return(ERR_GENERAL_ERROR);
}

result_t app_l3_can_dispatch_reg_handler(u8 protocol, can_l3_msg_handler_t handler, u8 *id)
{
	u8 loop;

	*id = 0xff;

	LOG_D("l3_can_dispatch_register_handler(0x%x)\n\r", (u16)protocol);

	/*
	 * Check is there already a handler for the Protocol
	 */
	for(loop = 0; loop < CAN_L3_REGISTER_ARRAY_SIZE; loop++) {
		if(  (registered[loop].used == TRUE)
		     &&(registered[loop].protocol == protocol)) {
			LOG_D("Replacing existing handler for Protocol 0x%x\n\r", (u16)protocol);
			registered[loop].handler = handler;
			*id = loop;
			return(SUCCESS);
		}
	}

	/*
	 * Find a free slot and add the Protocol
	 */
	for(loop = 0; loop < CAN_L3_REGISTER_ARRAY_SIZE; loop++) {
		if(registered[loop].used == FALSE) {
			registered[loop].used = TRUE;
			registered[loop].protocol = protocol;
			registered[loop].handler = handler;
			*id = loop;
			return(SUCCESS);
		}
	}

	LOG_E("L3 Dispatch full!\n\r");
	return(ERR_NO_RESOURCES);
}

result_t app_l3_can_dispatch_unreg_handler(u8 id)
{
	if((id < CAN_L3_REGISTER_ARRAY_SIZE) && (registered[id].used)) {
		registered[id].used = FALSE;
		registered[id].protocol = 0x00;
		registered[id].handler = (can_l3_msg_handler_t)NULL;
		return(SUCCESS);
	}
	return(ERR_GENERAL_L3_ERROR);
}
