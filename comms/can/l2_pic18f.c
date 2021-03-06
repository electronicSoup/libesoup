/**
 *
 * @file libesoup/comms/can/l2_pic18f.c
 *
 * @author John Whitmore
 * 
 * @brief Functions for retrieving the CinnamonBun Info Strings
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
#if defined( __18F2680) || defined(__18F4585)

#include "libesoup_config.h"

#ifdef SYS_CAN_BUS

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "l2_pic18f.h"

#include "libesoup/timers/sw_timers.h"

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_SLAVE)
#include "libesoup/comms/can/ping.h"
#endif

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
#include "libesoup/logger/serial_log.h"
static const char *TAG = "18F_CAN";
#endif // SYS_SERIAL_LOGGING

#include "libesoup/comms/can/can.h"

#undef L2_CAN_INTERRUPT_DRIVEN

/*
 * Interrupt Flags
 */
#define MERRE 0x80
#define WAKIE 0x40
#define ERRIE 0x20
#define TX2IE 0x10
#define TX1IE 0x08
#define TX0IE 0x04
#define RX1IE 0x02
#define RX0IE 0x01

#define CNTL_RXFUL 0x80
#define TXREQ 0x08
#define DCL_RNR 0x40
#define SIDL_EXIDE 0x08

#define TX_CON_TXPRI0   0x01
#define TX_CON_TXPRI1   0x02
#define TX_CON_TXREQ    0x08
#define TX_CON_TXERR    0x10
#define TX_CON_TXLARB   0x20
#define TX_CON_TXABT    0x40
#define TX_CON_TXBIF    0x80

/*
 * CAN Modes of Opperation
 */
#define MODE_MASK     0xe0

#define NORMAL_MODE   0x00
#define SLEEP_MODE    0x20
#define LOOPBACK_MODE 0x40
#define LISTEN_MODE   0x60
#define CONFIG_MODE   0x80

#define SIDL_SRTR   0x10
#define SIDL_EXIDE  0x08

#define DCL_ERTR    0x40


typedef struct
{
    uint8_t ctrl;
    uint8_t sidh;
    uint8_t sidl;
    uint8_t eid8;
    uint8_t eid0;
    uint8_t dcl;
    uint8_t data[8];
} canBuffer_t;

typedef struct can_mask
{
    uint8_t   *sidh;
    uint8_t   *sidl;
    uint8_t   *eidh;
    uint8_t   *eidl;
} can_mask;

/**
 * @brief Network Idle functionality
 *
 * We need to have CAN Bus Network traffic so that when devices connect
 * to listen to the Network to establish the CAN Bus Baud Rate. As a result
 * we'll keep a timer and if nothing has been received or transmitted in this
 * time we'll fire a ping message.
 */
#ifdef SYS_CAN_PING_PROTOCOL
static uint16_t         networkIdleDuration = 0;
static timer_id         networkIdleTimer = BAD_TIMER_ID;
static struct timer_req idle_timer_request;
#endif

static void set_baud_rate(can_baud_rate_t baudRate);
static void finaliseBaudRateChange(timer_id timer, union sigval data);

#ifdef L2_CAN_INTERRUPT_DRIVEN

static uint32_t rxMsgCount = 0;

#define SYS_CAN_RX_CIR_BUFFER_SIZE 5

canBuffer_t cirBuffer[CAN_RX_CIR_BUFFER_SIZE];
uint8_t     cirBufferNextRead = 0;
uint8_t     cirBufferNextWrite = 0;
uint8_t     cirBufferCount = 0;

can_msg_t   rxCanMsg;

#define     TX_BUFFERS  3

canBuffer_t *tx_buffers[TX_BUFFERS];

#else
can_frame rxMsg;

#define     TX_BUFFERS  3
#define     RX_BUFFERS  8

canBuffer_t *tx_buffers[TX_BUFFERS];
canBuffer_t *rx_buffers[RX_BUFFERS];

#endif

#define MASKS 2

can_mask masks[MASKS] =
{
    {(uint8_t *)&RXM0SIDH, (uint8_t *)&RXM0SIDL, (uint8_t *)&RXM0EIDH, (uint8_t *)&RXM0EIDL},
    {(uint8_t *)&RXM1SIDH, (uint8_t *)&RXM1SIDL, (uint8_t *)&RXM1EIDH, (uint8_t *)&RXM1EIDL}
};


static void set_mode(ty_can_mode mode);

#ifdef SYS_CAN_PING_PROTOCOL
static void restart_idle_timer(void);
#endif

/*
 * CAN Bus module uses the System status handler functionality so an application
 * can be notified of asynchronous changes in state of the CAN Bus
 */
static status_handler_t status_handler;

result_t can_l2_init(can_baud_rate_t arg_baud_rate, status_handler_t arg_status_handler)
{
	uint8_t loop;

	if (arg_baud_rate <= no_baud) {
		LOG_D("L2_CanInit() Baud Rate %s\n\r", can_baud_rate_strings[arg_baud_rate]);
	} else {
		/*
		 * No Baud rate specified should start baud rate detection if
		 * enabled.
		 */
		LOG_E("L2_CanInit() ToDo!!! No Baud Rate Specified\n\r");
		return (-ERR_BAD_INPUT_PARAMETER);
	}

	status_handler = arg_status_handler;

	/*
	 * Set up the SYS_CAN Configuration
	 */
	// IO Settings for SYS_CAN Tx and Rx on Port B
	TRISBbits.TRISB2 = OUTPUT_PIN;
	TRISBbits.TRISB3 = INPUT_PIN;

	set_mode(config);

	/*
	 * Set the Baud rate.
	 */
	set_baud_rate(arg_baud_rate);

	/*
	 * Set out Functional mode of operation
	 */
#ifdef L2_CAN_INTERRUPT_DRIVEN
	// Mode 0 Legacy
	ECANCONbits.MDSEL0 = 0;
	ECANCONbits.MDSEL1 = 0;

	tx_buffers[0] = (canBuffer_t *) & TXB0CON;
	tx_buffers[1] = (canBuffer_t *) & TXB1CON;
	tx_buffers[2] = (canBuffer_t *) & TXB2CON;

	RXB0CON = 0x00;
	RXB1CON = 0x00;
#else // L2_CAN_INTERRUPT_DRIVEN
	// Mode 2 FIFO mode
	ECANCONbits.MDSEL0 = 0;
	ECANCONbits.MDSEL1 = 1;

	/*
	 * Set 6 Additional buffers
	 * 6 (5,4,3, 2,1,0) Rx
	 */
	BSEL0 = 0x00;

	/*
	 * Set FIFO to interrupt when one RX Buffer left
	 */
	ECANCONbits.FIFOWM = 1;

	tx_buffers[0] = (canBuffer_t *) & TXB0CON;
	tx_buffers[1] = (canBuffer_t *) & TXB1CON;
	tx_buffers[2] = (canBuffer_t *) & TXB2CON;

	rx_buffers[0] = (canBuffer_t *) & RXB0CON;
	rx_buffers[1] = (canBuffer_t *) & RXB1CON;
	rx_buffers[2] = (canBuffer_t *) & B0CON;
	rx_buffers[3] = (canBuffer_t *) & B1CON;
	rx_buffers[4] = (canBuffer_t *) & B2CON;
	rx_buffers[5] = (canBuffer_t *) & B3CON;
	rx_buffers[6] = (canBuffer_t *) & B4CON;
	rx_buffers[7] = (canBuffer_t *) & B5CON;

	/*
	 * Recieve all valid messages
	 */
	for (loop = 0; loop < RX_BUFFERS; loop++) {
		rx_buffers[loop]->ctrl = 0;
	}
#endif  // L2_CAN_INTERRUPT_DRIVEN

	/*
	 * Disable all filters for the moment
	 */
	for (loop = 0; loop < MASKS; loop++) {
		*(masks[loop].sidh) = 0x00;
		*(masks[loop].sidl) = 0x00;
		*(masks[loop].eidh) = 0x00;
		*(masks[loop].eidl) = 0x00;
	}

	//    RXFCON0 = 0x00;
	//    RXFCON1 = 0x00;

	MSEL0 = 0x00;
	MSEL1 = 0x00;
	MSEL2 = 0x00;
	MSEL3 = 0x00;

#ifdef L2_CAN_INTERRUPT_DRIVEN
	PIE3 = 0xff;
#else
	// Disable all interrupts from SYS_CAN
	PIE3 = 0x00;
#endif

	/*
	 * Drop out of the configuration mode
	 * we're good to go
	 */
	set_mode(normal);

//	canStatus = Connected;

#ifdef SYS_CAN_PING_PROTOCOL
	// Create a random timer between 1 and 1.5 seconds for firing the
	// Network Idle Ping message
	networkIdleDuration = (UINT16) ((rand() % 500) + 1000);

	idle_timer_request.units = Seconds;
	idle_timer_request.duration = networkIdleDuration;
	idle_timer_request.type = single_shot;
	idle_timer_request.exp_fn = pingNetwork;
	idle_timer_request.data.sival_int = 0;
	restart_idle_timer();
	LOG_D("Network Idle Duration set to %d milliSeconds\n\r", networkIdleDuration);
#endif // SYS_CAN_PING_PROTOCOL
	
        return(SUCCESS);
}

#ifdef L2_CAN_INTERRUPT_DRIVEN
void L2_ISR(void)
{
	uint8_t flags = 0x00;
	uint8_t txFlags = 0x00;
	uint8_t ctrl;
	uint8_t loop;
	uint8_t *fromPtr;
	uint8_t *toPtr;

	/*
	 * Have to work of a snapshot of the Interrupt Flags as they
	 * are volatile.
	 */
	flags = PIR3;
	LOG_D("CAN L2 ISR Flag-%x\n\r", flags);
	if (flags & RX0IE) {
		/*
		 * Incrememnt the rx count incase we're listening for Baud
		 * Rate seettings.
		 */
		rxMsgCount++;

		if (cirBufferCount < SYS_CAN_RX_CIR_BUFFER_SIZE) {
			fromPtr = &RXB0SIDH;
			toPtr = &(cirBuffer[cirBufferNextWrite].sidh);

			for (loop = 0; loop < 13; loop++) {
				*toPtr++ = *fromPtr++;
			}
			cirBufferNextWrite = (cirBufferNextWrite + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
			cirBufferCount++;
		} else {
			LOG_E("Circular Buffer overflow!");
		}

		PIR3bits.RXB0IF = 0;
	}

	if (flags & RX1IE) {
		/*
		 * Incrememnt the rx count incase we're listening for Baud
		 * Rate seettings.
		 */
		rxMsgCount++;

		if (cirBufferCount < SYS_CAN_RX_CIR_BUFFER_SIZE) {
			fromPtr = &RXB1SIDH;
			toPtr = &(cirBuffer[cirBufferNextWrite].sidh);

			for (loop = 0; loop < 13; loop++) {
				*toPtr++ = *fromPtr++;
			}
			cirBufferNextWrite = (cirBufferNextWrite + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
			cirBufferCount++;
		} else {
			LOG_E("Circular Buffer overflow!");
		}

		PIR3bits.RXB1IF = 0;
	}
//    }
}
#endif

#ifdef L2_CAN_INTERRUPT_DRIVEN
void L2_CanTasks(void)
{
	uint8_t loop;

	LOG_D("L2_CanTasks()\n\r");
	while (cirBufferCount > 0) {
		// Check if it's an extended
		if (cirBuffer[cirBufferNextRead].sidl & SIDL_EXIDE) {
			rxCanMsg.header.extended_id = TRUE;
			rxCanMsg.header.rnr_frame = cirBuffer[cirBufferNextRead].dcl & DCL_ERTR;
			rxCanMsg.header.can_id.id = cirBuffer[cirBufferNextRead].sidh;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 3 | (cirBuffer[cirBufferNextRead].sidl >> 5) & 0x07;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 2 | cirBuffer[cirBufferNextRead].sidl & 0x03;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 8 | cirBuffer[cirBufferNextRead].eid8;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 8 | cirBuffer[cirBufferNextRead].eid0;
		} else
			rxCanMsg.header.extended_id = FALSE;
		rxCanMsg.header.rnr_frame = cirBuffer[cirBufferNextRead].sidl & SIDL_SRTR;
		rxCanMsg.header.can_id.id = cirBuffer[cirBufferNextRead].sidh;
		rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 3 | (cirBuffer[cirBufferNextRead].sidl >> 5) & 0x07;
	}

        /*
	 * Fill out the Data Length
         */
        rxCanMsg.header.data_length = cirBuffer[cirBufferNextRead].dcl & 0x0f;

        for (loop = 0; loop < rxCanMsg.header.data_length; loop++) {
		rxCanMsg.data[loop] = cirBuffer[cirBufferNextRead].data[loop];
	}

        LOG_D("Received a message id - %lx\n\r", rxCanMsg.header.can_id.id);
	cirBufferNextRead = (cirBufferNextRead + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
	cirBufferCount--;

        if(l2Handler) {
		l2Handler(&rxCanMsg);
	}
	//        processCanL2MsgFn(&rxCanMsg);
}

#else
void can_l2_tasks(void)
{
	result_t  rc = SUCCESS;
	uint8_t   i;
	uint8_t   buffer;
	uint8_t  *ptr;

	buffer = CANCON & 0x0f;

	/*
	 * Read all the messages present and process them
	 */
	while (rx_buffers[buffer]->ctrl & CNTL_RXFUL) {
		/*
		 * cancel the timer if running we've received a frame
		 */
#ifdef SYS_CAN_PING_PROTOCOL
		LOG_D("Rx L2 Message so restart Idle Timer\n\r");
		restart_idle_timer();
#endif

		if (rx_buffers[buffer]->sidl & SIDL_EXIDE) {
			/*
			 * Extended message identifer ???
			 */
			rxMsg.can_id = rx_buffers[buffer]->sidh;
			rxMsg.can_id = rxMsg.can_id << 3 | ((rx_buffers[buffer]->sidl >> 5) & 0x07);
			rxMsg.can_id = rxMsg.can_id << 2 | (rx_buffers[buffer]->sidl & 0x03);
			rxMsg.can_id = rxMsg.can_id << 8 | rx_buffers[buffer]->eid8;
			rxMsg.can_id = rxMsg.can_id << 8 | rx_buffers[buffer]->eid0;
			rxMsg.can_id |= CAN_EFF_FLAG;
		} else {
			/*
			 * Pull out the identifier
			 */
			rxMsg.can_id = rx_buffers[buffer]->sidh;
			rxMsg.can_id = rxMsg.can_id << 3 | ((rx_buffers[buffer]->sidl >> 5) & 0x07);
		}

		/*
		 * Pull out Data Length
		 */
		rxMsg.can_dlc = rx_buffers[buffer]->dcl & 0x0f;
		if(rx_buffers[buffer]->dcl & DCL_RNR) {
			rxMsg.can_id |= CAN_RTR_FLAG;
		}

		/*
		 * Pull out the data bytes
		 */
		ptr = rx_buffers[buffer]->data;
		for (i = 0; i < rxMsg.can_dlc; i++, ptr++) {
			rxMsg.data[i] = *ptr;
		}

		/* Clear the received flag */
		rx_buffers[buffer]->ctrl &= ~CNTL_RXFUL;
		LOG_D("rxMsg %lx\n\r", rxMsg.can_id);
		// Todo
//		if(l2Handler) {
//			l2Handler(&rxMsg);
//		} else {
			LOG_D("No Handler so ignoring received message\n\r");
//		}
//		buffer = SYS_CANCON & 0x0f;
	}
}
#endif

result_t can_l2_tx_frame(can_frame *frame)
{
	//    can_message_id_t  *id;
	uint8_t buffer;
	uint8_t i;
	uint8_t *ptr;
#if 0
	if (canStatus != Connected) {
		return (CAN_ERROR);
	}
#endif

	LOG_D("L2_CanTxMessage(0x%lx)\n\r", frame->can_id);
	if (frame->can_dlc > 8) {
		LOG_E("Bad Data length %d\n\r", frame->can_dlc);
		return (-ERR_BAD_INPUT_PARAMETER);
	}
	
	/*
	 * Find a free buffer
	 */
	for (buffer = 0; buffer < TX_BUFFERS; buffer++) {
		if (!(tx_buffers[buffer]->ctrl & TXREQ)) {
			break;
		}
	}

	if (buffer == TX_BUFFERS) {
		LOG_E("No empty TX buffer\n\r");
		return (-ERR_NO_RESOURCES); //No Empty buffers
	}

	/*
	 * Transmit buffer with index "buffer" is empty
	 * so fill in the registers with data.
	 */
	if (frame->can_id & CAN_EFF_FLAG) {
		//debug("Transmit an extended SYS_CAN message\n\r");

		tx_buffers[buffer]->sidh = ((frame->can_id & CAN_EFF_MASK) >> 21) & 0xff;
		tx_buffers[buffer]->sidl = (((frame->can_id & CAN_EFF_MASK) >> 18) & 0x07) << 5;
		tx_buffers[buffer]->sidl |= (((frame->can_id & CAN_EFF_MASK) >> 16) & 0x03);
		tx_buffers[buffer]->sidl |= SIDL_EXIDE;
		tx_buffers[buffer]->eid8 = ((frame->can_id & CAN_EFF_MASK) >> 8) & 0xff;
		tx_buffers[buffer]->eid0 = (frame->can_id & CAN_EFF_MASK) & 0xff;
	} else {
		//debug("Transmit a standard SYS_CAN message\n\r");
		tx_buffers[buffer]->sidh = ((frame->can_id & CAN_SFF_MASK) >> 3) & 0xff;
		tx_buffers[buffer]->sidl = ((frame->can_id & CAN_SFF_MASK) & 0x07) << 5;
	}

	tx_buffers[buffer]->dcl = frame->can_dlc & 0x0f;

	if (frame->can_id & CAN_RTR_FLAG) {
		tx_buffers[buffer]->dcl |= DCL_RNR;
	}

	ptr = tx_buffers[buffer]->data;
	for (i = 0; i < frame->can_dlc; i++, ptr++) {
		*ptr = frame->data[i];
	}

	/*
	 * Mark the buffer for transmission
	 */
	tx_buffers[buffer]->ctrl |= TXREQ;

	/*
	 * cancel the timer if running we've received a frame
	 */
#ifdef SYS_CAN_PING_PROTOCOL
	LOG_D("Transmitting L2 Message so restart Idle Timer\n\r");
	restart_idle_timer();
#endif
	return (SUCCESS);
}

//ToDo
#if 0
void L2_CanTxError(uint8_t node_type, uint8_t node_number, UINT32 errorCode) 
{
	can_error_t error;

	error.header.can_id.id = canIds[can_error].id;
	error.header.extended_id = canIds[can_error].extended;
	error.header.rnr_frame = FALSE;
	error.header.data_length = 6;

	error.node.node_type = node_type;
	error.node.node_number = node_number;

	error.errorCode = errorCode;

	L2_CanTxMessage((can_msg_t *) & error);
}
#endif

#if 0
void L2_SetCanNodeBuadRate(can_baud_rate_t baudRate)
{
	timer_id         timer;
	struct timer_req timer_request;	
	
	TIMER_INIT(timer);
	
//	can_baud_rate_t testRate;

	LOG_D("L2_SetCanNodeBuadRate()\n\r");
	// Todo
	//sys_eeprom_write(NETWORK_BAUD_RATE, (uint8_t) baudRate);

	//sys_eeprom_read(NETWORK_BAUD_RATE, (uint8_t *) & testRate);

//	if (testRate != baudRate) {
//		LOG_E("Baud Rate NOT Stored!\n\r");
//	} else {
//		LOG_D("Baud Rate Stored\n\r");
//	}

//	canStatus = ChangingBaud;
//	set_mode(CONFIG_MODE);

	set_baud_rate(baudRate);

	/*
	 * The Baud rate is being changed so going to stay in config mode
	 * for 10 Seconds and let the Network settle down.
	 */
	timer_request.units = Seconds;
	timer_request.duration = 10;
	timer_request.type = single_shot;
	timer_request.exp_fn = finaliseBaudRateChange;
	timer_request.data.sival_int = 0;
	sw_timer_start(&timer, &timer_request);
}
#endif // 0

#if 0
static void finaliseBaudRateChange(timer_id timer, union sigval data)
{
	LOG_D("finaliseBaudRateChange()\n\r");
//	canStatus = Connected;
	set_mode(normal);
}
#endif // 0

#if 0
void L2_SetCanNetworkBuadRate(can_baud_rate_t baudRate)
{
	LOG_D("L2_SetCanNetworkBuadRate()\n\r");
	set_mode(config);
	set_baud_rate(baudRate);
	set_mode(normal);
}
#endif // 0

static void set_baud_rate(can_baud_rate_t baudRate)
{
	uint8_t sjw = 0;
	uint8_t brp = 0;
	uint8_t phseg1 = 0;
	uint8_t phseg2 = 0;
	uint8_t propseg = 0;

	switch (baudRate) {
	case baud_10K:
		brp = 49;
		propseg = 7;
		sjw = 3;
		phseg1 = 4;
		phseg2 = 4;
		break;

	case baud_20K:
		brp = 24;
		propseg = 7;
		sjw = 3;
		phseg1 = 4;
		phseg2 = 4;
		break;

	case baud_50K:
		brp = 9;
		propseg = 7;
		sjw = 3;
		phseg1 = 4;
		phseg2 = 4;
		break;

	case baud_125K:
		brp = 3;
		propseg = 7;
		sjw = 3;
		phseg1 = 4;
		phseg2 = 4;
		break;

	case baud_250K:
		brp = 1;
		propseg = 7;
		sjw = 3;
		phseg1 = 4;
		phseg2 = 4;
		break;

	case baud_500K:
		brp = 0;
		propseg = 7;
		sjw = 3;
		phseg1 = 4;
		phseg2 = 4;
		break;

	case baud_800K:
		brp = 0;
		propseg = 3;
		sjw = 2;
		phseg1 = 3;
		phseg2 = 3;
		break;

	case baud_1M:
		brp = 0;
		propseg = 3;
		sjw = 1;
		phseg1 = 2;
		phseg2 = 2;
		break;

	default:
		LOG_E("Invalid Baud Rate Specified\n\r");
		break;
	}

	BRGCON1 = (((sjw - 1) & 0x03) << 6) | ((brp) & 0x3f);

	BRGCON2 = (((phseg1 - 1) & 0x07) << 3) | ((propseg - 1) & 0x07);
	BRGCON2bits.SAM = 0;      //One sampe point
	BRGCON2bits.SEG2PHTS = 1; //Phase Segement 2 programmed

	BRGCON3 = ((phseg2 - 1) & 0x07);
}

/*
 * Function Header
 */
static void set_mode(ty_can_mode mode)
{
	uint8_t value;

	/*
	 * Enter SYS_CAN Configuration mode
	 */
	value = CANCON;

	value &= ~MODE_MASK;
	value |= (mode << 5);

	CANCON = value;

	/*
	 * SYS_CAN Mode is a request so we have to wait for confirmation
	 */
	while ((CANSTAT & MODE_MASK) != (CANCON & MODE_MASK));
}

#ifdef SYS_CAN_PING_PROTOCOL
static void restart_idle_timer(void)
{
	sw_timer_cancel(networkIdleTimer);
	TIMER_INIT(networkIdleTimer);
	sw_timer_start(networkIdleTimer, &timer_request);	
}
#endif

#endif // SYS_CAN_BUS
#endif // defined( __18F2680) || defined(__18F4585)
