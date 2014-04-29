/**
 *
 * \file es_can/can/l2_mcp2515.c
 *
 * CAN L2 Functionality for MCP2515 Chip
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
#include "es_lib/core.h"
#include "system.h"

#define DEBUG_FILE
#include "es_lib/logger/serial.h"
#include "es_lib/can/es_can.h"
#include "es_lib/can/l2_mcp2515.h"
#include "es_lib/timers/timer_sys.h"

#include "es_lib/utils/utils.h"

#define TAG "MCP2515"

#define LISTEN_TIME SECONDS_TO_TICKS(10)

typedef struct
{
	BYTE sidh;
	BYTE sidl;
	BYTE eid8;
	BYTE eid0;
	BYTE dcl;
	BYTE data[8];
} canBuffer_t;

#define CAN_RX_CIR_BUFFER_SIZE 5

canBuffer_t cirBuffer[CAN_RX_CIR_BUFFER_SIZE];
BYTE cirBufferNextRead = 0;
BYTE cirBufferNextWrite = 0;
BYTE cirBufferCount = 0;

#define REGISTER_ARRAY_SIZE 5

typedef struct
{
	BYTE used;
	can_target_t target;
} canRegister;

static canRegister registered[REGISTER_ARRAY_SIZE];

static u8 connectingErrors = 0;

/*
 * Byte to store current input values in TXRTSCTRL
 */
static u8 txrtsctrl = 0x00;

static result_t send_ping(void);
static void CANReset(void);
static void CANSetRegMaskValue(u8 reg, u8 mask, u8 value);
static void set_can_mode(u8 mode);
static void setBitRate(baud_rate_t baudRate);
static void exp_checkNetworkConnection(union sigval);
static void exp_finaliseBaudRateChange(union sigval data);
static void exp_resendBaudRateChange(union sigval data);
static void CANEnableRXInterrupts(void);
static void CANDisableRXInterrupts(void);
static u8 CANReadReg(u8 reg);
static void CANWriteReg(u8 reg, u8 value);
static void CANReadRxBuffer(u8 reg, u8 *buffer);
static u8 CANFindFreeTxBuffer(void);
static u8 CheckErrors(void);
static void checkSubErrors(void);

static void l2_dispatcher_frame_handler(can_frame *message);

#if LOG_LEVEL < NO_LOGGING
void printErrorCounts(void);
#endif

/*
 * Global record of CAN Bus error flags.
 */
static baud_rate_t connectedBaudRate = no_baud;
static baud_rate_t listenBaudRate = no_baud;
static BYTE ChangingBaudTxError;
static BYTE g_CanErrors = 0x00;
static UINT32 g_missedMessageCount = 0;
static UINT32 rxMsgCount = 0;
static UINT32 messageSentCount = 0;
static UINT32 wakeUpCount = 0;
static can_frame rxCanMsg;

static can_status_t status;
static baud_rate_t baud = no_baud;

static es_timer listen_timer;
static es_timer ping_timer;

//static can_status_handler status_handler = (can_status_handler)NULL;
static void (*status_handler)(u8 mask, can_status_t status, baud_rate_t baud) = NULL;

/**
 * \brief Initialise the CAN Bus.
 *
 * \param baudRate The Baud rate that the bus is operating at
 * or "no_baud" if we don't know the speed of the Bus and have to
 * search for it.
 *
 * \param processCanL2Message The function to process received
 * Layer 2 Can messages.
 */
result_t l2_init(baud_rate_t arg_baud_rate,
                 void (*arg_status_handler)(u8 mask, can_status_t status, baud_rate_t baud))
{
	u8 loop = 0x00;
	u8 exitMode = NORMAL_MODE;
	u32 delay;
        result_t result;

	DEBUG_D("l2_init()\n\r");

        status.byte = 0x00;
        baud = no_baud;

	TIMER_INIT(listen_timer);
	TIMER_INIT(ping_timer);

	for(loop = 0; loop < REGISTER_ARRAY_SIZE; loop++) {
		registered[loop].used = FALSE;
		registered[loop].target.mask = 0x00;
		registered[loop].target.filter = 0x00;
		registered[loop].target.handler = (l2_msg_handler_t)NULL;
	}

	status_handler = arg_status_handler;

	CAN_INTERRUPT_PIN_DIRECTION = INPUT_PIN;
	CAN_CS_PIN_DIRECTION = OUTPUT_PIN;
	CAN_DeSelect();

	CANReset();

	for(delay = 0; delay < 0x40000; delay++) {
		Nop();
	}

	DEBUG_D("Set CAN Mode to CONFIG\n\r");
	set_can_mode(CONFIG_MODE);
	DEBUG_D("Set CAN Mode SET\n\r");

	CANWriteReg(RXB0CTRL, 0x64);
	CANWriteReg(RXB1CTRL, 0x60);
	CANWriteReg(TXRTSCTRL, 0x00);
	CANWriteReg(BFPCTRL, 0x00);

	/**
	 * Have to set the baud rate if one has been passed into the function
	 */
	if(arg_baud_rate <= BAUD_MAX) {
		DEBUG_D("Valid Baud Rate specified - %s\n\r", baud_rate_strings[arg_baud_rate]);
		connectedBaudRate = arg_baud_rate;
		setBitRate(arg_baud_rate);
		exitMode = NORMAL_MODE;

		status.bit_field.l2_status = L2_Connecting;
		baud = arg_baud_rate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, baud);
	} else {
		/*
		 * Have to search for the Networks baud rate. Start at the bottom
		 */
		rxMsgCount = 0;
		DEBUG_D("Before trying 10K REC - %d, rxCount - %d\n\r", CANReadReg(REC), rxMsgCount);
		listenBaudRate = baud_10K;
		setBitRate(listenBaudRate);
		exitMode = LISTEN_MODE;

		connectingErrors = 0;
		status.bit_field.l2_status = L2_Listening;
		baud = listenBaudRate;
		DEBUG_D("Call the status handler\n\r");
		if(status_handler)
			status_handler(L2_STATUS_MASK, status, baud);
		DEBUG_D("Return from status handler\n\r");

		/* Now wait and see if we have errors */
		start_timer(LISTEN_TIME, exp_checkNetworkConnection, (union sigval)(void *)NULL, &listen_timer);
	}

	DEBUG_D("Set Exit Mode 0x%x\n\r", exitMode);
	set_can_mode(exitMode);

//        CANWriteReg(CANINTF, 0x00);
//	CANWriteReg(CANINTE, 0x00);
#ifdef L2_CAN_INTERRUPT_DRIVEN
//	CANWriteReg(CANINTE, MERRE | ERRIE | TX2IE | TX1IE | TX0IE | RX1IE | RX0IE);
	CANWriteReg(CANINTE, MERRE | RX1IE | RX0IE);
#endif

        /*
	 * If we're using an Interrupt driven approach turn on Interrupts
	 */
#ifdef L2_CAN_INTERRUPT_DRIVEN
        INTCON2bits.INT0EP = 1; //Interrupt on Negative edge
        IFS0bits.INT0IF = 0;    // Clear the flag
        IEC0bits.INT0IE = 1;    //Interrupt Enabled
#endif

	/*
	 * If we've been given a valid baud rate and connected send a test mesage to Network
	 */
	if(status.bit_field.l2_status == L2_Connecting) {
		DEBUG_D("Connecting send a test Ping message!\n\r");
		result = send_ping();
		if (result != SUCCESS) {
			DEBUG_E("Failed to send a test Ping message!\n\r");
		}
	}

// Create a random timer between 1 and 1.5 seconds for firing the
	// Network Idle Ping message
#if defined(CAN_IDLE_PING)
	ping_time = (u16)((rand() % 500) + 1000);
	start_timer(ping_time, exp_test_ping, (union sigval)(void *)NULL, &ping_timer);
#endif

	DEBUG_D("CAN Layer 2 Initialised\n\r");
	return(SUCCESS);
}

result_t send_ping(void)
{
	can_frame msg;

	msg.can_id = 0x808;
	msg.can_dlc = 0;

	return(l2_tx_frame(&msg));
}

#if defined(CAN_IDLE_PING)
void exp_test_ping(union sigval data __attribute__((unused)))
{
	result_t result = SUCCESS;
	BYTE flags;

        DEBUG_D("exp_test_ping()\n\r");
//    DEBUG_D("CANINTF %x\n\r", CANReadReg(CANINTF));
	flags = CANReadReg(CANINTF);
	TIMER_INIT(ping_timer);
	
//    if((flags != 0x00) && !PORTAbits.RA14)
	if((flags != 0x00) && !CAN_INTERRUPT) {
		DEBUG_D("-> Interrupt line Low Flags Set!\n\r");
//        L2_ISR();
	}

	result = send_ping();
	if (result == SUCCESS)
		start_timer(ping_time, exp_test_ping, (union sigval)(void *)NULL, &ping_timer);
}
#endif

void exp_checkNetworkConnection(union sigval data __attribute__((unused)))
{
	result_t result;
	u8 rec = CANReadReg(REC);

	TIMER_INIT(listen_timer);

	if(listenBaudRate <= BAUD_MAX) {
		DEBUG_D("After trying %s Errors - %d, rxCount - %ld\n\r", baud_rate_strings[listenBaudRate], connectingErrors, rxMsgCount);
	} else {
		DEBUG_D("After trying %s Errors - %d, rxCount - %ld\n\r", "NO BAUD RATE", connectingErrors, rxMsgCount);
	}
	/*
	 * If we heard valid messages with no errors we've found the baud rate.
	 */
	if(rxMsgCount > 0 && connectingErrors == 0) {
		DEBUG_D("*** Network connected ***\n\r");
		connectedBaudRate = listenBaudRate;

		set_can_mode(NORMAL_MODE);

		status.bit_field.l2_status = L2_Connected;
		baud = connectedBaudRate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, baud);

		// Test code to periodically send message
		// message 708 now used by CancelNetLogger message
// start_timer(SECONDS_TO_TICKS(30), exp_test_ping, NULL);
	} else {
		if(listenBaudRate == BAUD_MAX)
			listenBaudRate = baud_10K;
		else
			listenBaudRate++;
		DEBUG_D("No joy try Baud Rate - %s\n\r", baud_rate_strings[listenBaudRate]);
		set_can_mode(CONFIG_MODE);
		setBitRate(listenBaudRate);
		set_can_mode(LISTEN_MODE);

                rxMsgCount = 0;
		connectingErrors = 0;
		status.bit_field.l2_status = L2_Listening;
	        baud = listenBaudRate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, baud);

		DEBUG_D("Restart timer\n\r");
		result = start_timer(LISTEN_TIME,
				     exp_checkNetworkConnection,
				     (union sigval)(void *)NULL,
				     &listen_timer);
		if(result != SUCCESS) {
			DEBUG_E("Failed to start listen timer, result 0x%x\n\r", result);
		}
	}
}

/*
 * This is the Flags driven ISR
 */
#if defined(L2_CAN_INTERRUPT_DRIVEN)
void _ISR __attribute__((__no_auto_psv__)) _INT0Interrupt(void)
#else
void L2_ISR(void)
#endif
{
	BYTE flags = 0x00;
	BYTE eflg;
	BYTE txFlags = 0x00;
	BYTE ctrl;
	BYTE loop;
	BYTE canstat;

	/*
	 * Have to work with a snapshot of the Interrupt Flags as they
	 * are volatile.
	 */
	flags = CANReadReg(CANINTF);

#ifdef L2_CAN_INTERRUPT_DRIVEN
	DEBUG_D("*** ISR *** flags 0x%x\n\r", flags);

	if(flags == 0x00) {
		canstat = CANReadReg(CANSTAT);
		CANWriteReg(CANINTF, 0x00);

		DEBUG_W("*** ISR with zero flags! IOCD %x\n\r", canstat & IOCD);
	}
#endif

	while(flags != 0x00) {
		canstat = CANReadReg(CANSTAT);
		DEBUG_D("*** ISR Flag-%x, IOCD-%x\n\r", flags, (canstat & IOCD));
		if (flags & ERRIE) {
			eflg = CANReadReg(EFLG);
			DEBUG_E("*** CAN ERRIR Flag!!!\n\r");
			DEBUG_E("*** CAN EFLG %x\n\r", eflg);
			if(status.bit_field.l2_status == L2_Listening)
				connectingErrors++;

			/*
			 * Clear any rx Frames as there's been an error
			 */
			CANSetRegMaskValue(EFLG, RX1OVR | RX0OVR, 0x00);

			CANSetRegMaskValue(CANINTF, ERRIE, 0x00);
			flags = flags & ~(RX0IE | RX1IE);
		}

		if (flags & MERRE) {
			DEBUG_W("CAN MERRE Flag\n\r");
			if(status.bit_field.l2_status == L2_Listening)
				connectingErrors++;

			/*
			 * We've got an error condition so dump all received messages
			 */
			CANSetRegMaskValue(CANINTF, RX0IE | RX1IE | MERRE, 0x00);
			flags = flags & ~(RX0IE | RX1IE | MERRE);
		}

		ctrl = TXB0CTRL;

		for (loop = 0; loop < 3; loop++) {
			txFlags = CANReadReg(ctrl);

			if((txFlags & TXREQ) && (txFlags & TXERR)) {
				DEBUG_E("Transmit Buffer Failed to send\n\r");
				CANSetRegMaskValue(ctrl, TXREQ, 0x00);
				if (status.bit_field.l2_status == L2_ChangingBaud)
					ChangingBaudTxError++;
			}
			ctrl = ctrl + 0x10;
		}

		if (flags & RX0IE) {
			DEBUG_D("RX0IE\n\r");
			/*
			 * Increment the rx count in case we're listening for Baud
			 * Rate seettings.
			 */
			if (status.bit_field.l2_status == L2_Listening) {
				rxMsgCount++;
			} else {
				if (cirBufferCount < CAN_RX_CIR_BUFFER_SIZE) {
					CANReadRxBuffer(RXB0SIDH, (BYTE *) & cirBuffer[cirBufferNextWrite]);
					cirBufferNextWrite = (cirBufferNextWrite + 1) % CAN_RX_CIR_BUFFER_SIZE;
					cirBufferCount++;
				} else {
					DEBUG_E("Circular Buffer overflow!\n\r");
				}
			}

			CANSetRegMaskValue(CANINTF, RX0IE, 0x00);
		}

		if (flags & RX1IE) {
			DEBUG_D("RX1IE\n\r");
			/*
			 * Incrememnt the rx count incase we're listening for Baud
			 * Rate seettings.
			 */
			if (status.bit_field.l2_status == L2_Listening) {
				rxMsgCount++;
			} else {
				if (cirBufferCount < CAN_RX_CIR_BUFFER_SIZE) {
					CANReadRxBuffer(RXB1SIDH, (BYTE *) & cirBuffer[cirBufferNextWrite]);
					cirBufferNextWrite = (cirBufferNextWrite + 1) % CAN_RX_CIR_BUFFER_SIZE;
					cirBufferCount++;
				} else {
					DEBUG_E("Circular Buffer overflow!\n\r");
				}
			}

			CANSetRegMaskValue(CANINTF, RX1IE, 0x00);
		}

		if (flags & TX2IE) {
			DEBUG_D("TX2IE\n\r");
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, baud);
			}

			CANSetRegMaskValue(CANINTF, TX2IE, 0x00);
		}

		if (flags & TX1IE) {
			DEBUG_D("TX1IE\n\r");
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, baud);
			}
			CANSetRegMaskValue(CANINTF, TX1IE, 0x00);
		}

		if (flags & TX0IE) {
			DEBUG_D("TX0IE\n\r");
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, baud);
			}

			CANSetRegMaskValue(CANINTF, TX0IE, 0x00);
		}
		flags = CANReadReg(CANINTF);
	}
        IFS0bits.INT0IF = 0;
#if defined(L2_CAN_INTERRUPT_DRIVEN)
	DEBUG_D("Exit ISR\n\r");
#endif
}

void L2_CanTasks(void)
{
#ifdef TEST
	static UINT16 count = 0;
#endif
	BYTE loop;
	BYTE byte;
        static BYTE last_flags = 0x00;
	BYTE flags;
	BYTE eflg;

#ifndef L2_CAN_INTERRUPT_DRIVEN
	L2_ISR();
#else
//        if(CAN_INTERRUPT) {
//		DEBUG_D("Call ISR from Tasks\n\r");
//        _INT0Interrupt();
//        }
#endif

#if 0
	count++;

	if(count == 0x00) {
		DEBUG_D("L2_CanTasks()\n\r");
	}
#endif

#if 0
	flags = CANReadReg(CANINTF);
	if(flags != last_flags) {
            last_flags = flags;
	    DEBUG_D("!!! - TASKS Flags 0x%x\n\r", flags);
	    eflg = CANReadReg(EFLG);
	    DEBUG_D("*** CAN EFLG %x\n\r", eflg);
	    if(!CAN_INTERRUPT) {
		    DEBUG_D("Interrupt line Low Flags Set!\n\r");
//        L2_ISR();
	    }
        }
#endif
	while(cirBufferCount > 0) {
		if (status.bit_field.l2_status == L2_Connecting) {
			status.bit_field.l2_status = L2_Connected;

			if (status_handler)
				status_handler(L2_STATUS_MASK, status, baud);
		}

		/*
		 * make sure the rx msg id is cleared
		 */
		rxCanMsg.can_id = 0x00;

		// Check if it's an extended
		if (cirBuffer[cirBufferNextRead].sidl & SIDL_EXIDE) {
			rxCanMsg.can_id = cirBuffer[cirBufferNextRead].sidh;
			rxCanMsg.can_id = rxCanMsg.can_id << 3 | (cirBuffer[cirBufferNextRead].sidl >> 5) & 0x07;
			rxCanMsg.can_id = rxCanMsg.can_id << 2 | cirBuffer[cirBufferNextRead].sidl & 0x03;
			rxCanMsg.can_id = rxCanMsg.can_id << 8 | cirBuffer[cirBufferNextRead].eid8;
			rxCanMsg.can_id = rxCanMsg.can_id << 8 | cirBuffer[cirBufferNextRead].eid0;
			rxCanMsg.can_id |= CAN_EFF_FLAG;

			if(cirBuffer[cirBufferNextRead].dcl & DCL_ERTR)
				rxCanMsg.can_id |= CAN_RTR_FLAG;
		} else {
			rxCanMsg.can_id = cirBuffer[cirBufferNextRead].sidh;
			rxCanMsg.can_id = rxCanMsg.can_id << 3 | (cirBuffer[cirBufferNextRead].sidl >> 5) & 0x07;

			if(cirBuffer[cirBufferNextRead].sidl & SIDL_SRTR)
				rxCanMsg.can_id |= CAN_RTR_FLAG;
		}

		/*
		 * Fill out the Data Length
		 */
		rxCanMsg.can_dlc = cirBuffer[cirBufferNextRead].dcl & 0x0f;

		for (loop = 0; loop < rxCanMsg.can_dlc; loop++) {
			rxCanMsg.data[loop] = cirBuffer[cirBufferNextRead].data[loop];
		}

//        DEBUG_D("Received a message id - %lx\n\r", rxCanMsg.header.can_id.id);
		cirBufferNextRead = (cirBufferNextRead + 1) % CAN_RX_CIR_BUFFER_SIZE;
		cirBufferCount--;
		l2_dispatcher_frame_handler(&rxCanMsg);
	}

#ifdef TEST
	if(count == 0x00) {

		CAN_Select();
		byte = CANReadReg(TXRTSCTRL);
		CAN_DeSelect();

#ifdef MCP2515_INPUT_2
		if ((txrtsctrl & B2RTS) != (byte & B2RTS)) {
			if (byte & B2RTS) {
				DEBUG_D("MCP2515 Input 2 On\n\r");
			} else {
				DEBUG_D(Debug, TAG, "MCP2515 Input 2 Off\n\r");
			}
		}
#endif
		txrtsctrl = byte;
	}
#endif
}

void CANActivate(void)
{
	set_can_mode(NORMAL_MODE);
	CANEnableRXInterrupts();
}

void CANDeactivate(void)
{
	set_can_mode(CONFIG_MODE);
	CANDisableRXInterrupts();
}

result_t l2_tx_frame(can_frame  *canMsg)
{
	result_t result = SUCCESS;
	canBuffer_t  txBuffer;
	BYTE *buff;
	BYTE loop = 0x00;
	BYTE ctrl;
	BYTE canBuffer;

	DEBUG_D("L2 => Id %lx\n\r", canMsg->can_id);

	if(connectedBaudRate == no_baud) {
		DEBUG_E("Can't Transmit network not connected!\n\r");
		return(ERR_GENERAL_CAN_ERROR);
	}

	if(canMsg->can_id & CAN_EFF_FLAG) {
		txBuffer.sidh = (canMsg->can_id >> 21) & 0xff;
		txBuffer.sidl = ((canMsg->can_id >> 18) & 0x07) << 5;
		txBuffer.sidl |= ((canMsg->can_id >> 16) & 0x03);
		txBuffer.sidl |= SIDL_EXIDE;
		txBuffer.eid8 = (canMsg->can_id >> 8) & 0xff;
		txBuffer.eid0 = canMsg->can_id & 0xff;
	} else {
		// Fill in the ID
		txBuffer.sidh = (canMsg->can_id >> 3) & 0xff;
		txBuffer.sidl = (canMsg->can_id & 0x07) << 5;
	}
	// Remote Transmission Request
	txBuffer.dcl = (canMsg->can_id & CAN_RTR_FLAG) ? 0x40 : 0x00;

	// Data Length
	txBuffer.dcl = txBuffer.dcl | (canMsg->can_dlc & 0x0f);

	for(loop = 0; loop < txBuffer.dcl; loop++) {
		txBuffer.data[loop] = canMsg->data[loop];
	}

	/*
	 * Find an empty txBuffer
	 */
	ctrl = CANFindFreeTxBuffer();

	if(ctrl == 0xff) {
		// Shipment of fail has arrived
		DEBUG_E("ERROR No free Tx Buffers\n\r");
		return(ERR_CAN_NO_FREE_BUFFER);
	} else if (ctrl == TXB0CTRL) {
		DEBUG_D("TXB0CTRL\n\r");
		canBuffer = TXB0SIDH;
	} else if (ctrl == TXB1CTRL) {
		DEBUG_D("TXB1CTRL\n\r");
		canBuffer = TXB1SIDH;
	} else if (ctrl == TXB2CTRL) {
		DEBUG_D("TXB2CTRL\n\r");
		canBuffer = TXB2SIDH;
	}

	/*
	 * Load up the transmit buffer
	 */
	CAN_Select();
	SPIWriteByte(CAN_WRITE_REG);
	SPIWriteByte(canBuffer);

	/*
	 * First pull out the five header bytes in the message
	 */
	buff = (BYTE *)&txBuffer;
	for(loop = 0; loop < 5; loop++, buff++) {
		SPIWriteByte(*buff);
	}

	for(loop = 0; loop < canMsg->can_dlc; loop++, buff++) {
		SPIWriteByte(*buff);
	}
	CAN_DeSelect();

	/*
	 * Right all set for Transmission but check the current network status
	 * and send in One Shot Mode if we're unsure of the Network.
	 */
	if (status.bit_field.l2_status == L2_Connecting) {
		DEBUG_D("Network not good so sending OSM\n\r");
		CANSetRegMaskValue(CANCTRL, OSM, OSM);
	} else {
		// Clear One Shot Mode
		CANSetRegMaskValue(CANCTRL, OSM, 0x00);
	}

	// Set the buffer for Transmission
	CANSetRegMaskValue(ctrl, TXREQ, TXREQ);

#if 0
	while(CANReadReg(ctrl) & 0x08) {
#if LOG_LEVEL <= LOG_ERROR
		DEBUG_E("Wait for transmission to complete\n\r");
#endif
	}

	if(CANReadReg(CANINTF) & MERRE) {
#if LOG_LEVEL <= LOG_ERROR
		DEBUG_E("MERRE\n\r");
#endif
		// Clear the error flag
		CANSetRegMaskValue(CANINTF, MERRE, 0x00);
		errorCode = CAN_ERROR;
	}

	if(CANReadReg(CANINTF) & ERRIE) {
#if LOG_LEVEL <= LOG_ERROR
		DEBUG_E("ERRIE\n\r");
#endif
		// Clear the error flag
		CANSetRegMaskValue(CANINTF, ERRIE, 0x00);
		errorCode = CAN_ERROR;
	}

	if(errorCode == CAN_SUCCESS)
		networkGood = TRUE;
#endif

	return(result);
}

BYTE CheckErrors(void)
{
	BYTE flags = 0x00;

	flags = CANReadReg(CANINTF);

	if(flags != 0x00) {
		DEBUG_D("CheckErrors() Flag set 0x%x\n\r", flags);
	}

	if (flags & TX0IE) {
		CANSetRegMaskValue(CANINTF, TX0IE, 0x00);
		messageSentCount++;
	}

	if (flags & TX1IE) {
		CANSetRegMaskValue(CANINTF, TX1IE, 0x00);
		messageSentCount++;
	}

	if (flags & TX2IE) {
		CANSetRegMaskValue(CANINTF, TX2IE, 0x00);
		messageSentCount++;
	}

	if (flags & MERRE) {
		CANSetRegMaskValue(CANINTF, MERRE, 0x00);
		DEBUG_E("Message Error Flag set! Flasg now 0x%x\n\r", CANReadReg(CANINTF));
	}

	if (flags & WAKIE) {
		CANSetRegMaskValue(CANINTF, WAKIE, 0x00);
		wakeUpCount++;
		DEBUG_D("Wake Up Count - now %ld\n\r", wakeUpCount);
	}

	if (flags & ERRIE) {
		/*
		 * This is a bigger bunch of checks on errors so do
		 * the processing in sub function
		 */
		checkSubErrors();
		CANSetRegMaskValue(CANINTF, ERRIE, 0x00);
	}
}

static void checkSubErrors(void)
{
	BYTE error = 0x00;
	BYTE difference = 0x00;
	BYTE mask = 0x00;
	BYTE loop = 0x00;

	/*
	 * Read the error flag
	 */
	error = CANReadReg(EFLG);

	if(error != 0x00) {
		DEBUG_E("checkSubErrors() errors - %x\n\r", error);
	}

	/*
	 * Process the two RX error flags out of the way and clear them
	 */
	if (error & RX1OVR) {
		g_missedMessageCount++;
		DEBUG_D("Missed Message Count - %ld\n\r", g_missedMessageCount);

		/*
		 * Clear this flag
		 */
		CANSetRegMaskValue(EFLG, RX1OVR, 0x00);
	}

	if (error & RX0OVR) {
		g_missedMessageCount++;
		DEBUG_D("Missed Message Count - %ld\n\r", g_missedMessageCount);

		/*
		 * Clear this flag
		 */
		CANSetRegMaskValue(EFLG, RX0OVR, 0x00);
	}

	/*
	 * The remaining error Flags are permanent in nature and will
	 * stay active until cleared by the CAN Module. We only want to
	 * flag these errors to Higher layers, or Android if they're occuring for
	 * the first time
	 */

	/*
	 * First get rid of any RX Error falgs from our read error flags
	 */
	error = error & 0x3f;

	/*
	 * If our remaining error flags are not the same as what we currently have
	 * then something has changed!
	 */
	if(error != g_CanErrors) {
		/*
		 * Work out what's changed
		 */
		difference = error ^ g_CanErrors;
		DEBUG_E("ERRORS Have changed! Difference is %x\n\r", difference);

		/*
		 * Loop through the lower 6 bits we're interested in
		 */
		mask = 0x01;

		for(loop = 0; loop < 6; loop++, mask << 1) {
			if(mask & difference) {
				if (mask & TXBO) {
					/*
					 * TXBO Has changed
					 */
					if(error & TXBO) {
						//TODO Must send an error to Android
						DEBUG_E("ERROR Bus OFF!\n\r");
					} else {
						DEBUG_E("Error Cleared CAN Bus Active again\n\r");
					}
				}

				if (mask & TXEP) {
					if (error & TXEP) {
						//ToDo Must send an error to Android
						DEBUG_E("Transmitter ERROR PASSIVE Error count > 128!\n\r");
					} else {
						DEBUG_E("Tx Error count < 128 ;-)\n\r");
					}
				}

				if (mask & RXEP) {
					if (error & RXEP) {
						//TODO Must send an error to Android
						DEBUG_E("Receiver ERROR PASSIVE Error count > 128!\n\r");
					} else {
						DEBUG_E("Rx Error < 128 :-)\n\r");
					}
				}

				if (mask & TXWAR) {
					if (error & TXWAR) {
						//TODO Must send an error to Android
						DEBUG_E("Transmitter WARNING Error count > 96!\n\r");
					} else {
						DEBUG_E("Tx Warning cleared Error Count < 96\n\r");
					}
				}

				if (mask & TXBO) {
					if (error & RXWAR) {
						//TODO Must send an error to Android
						DEBUG_E("Receiver WARNING Error count > 96!\n\r");
					} else {
						DEBUG_E("Rx Warning Cleared Error Count < 96\n\r");
					}
				}
			}
		}
		g_CanErrors = error;
	} else {
		DEBUG_D("Errors Have not changed\n\r");
	}
}

static void CANReset(void)
{
	/* Reset the Can Chip */
	CAN_Select();
	SPIWriteByte(CAN_RESET);
	CAN_DeSelect();
}

static void CANSetRegMaskValue(BYTE reg, BYTE mask, BYTE value)
{
	BYTE fail;

        //    do {
        CAN_Select();
        SPIWriteByte(CAN_BIT_MODIFY);
        SPIWriteByte(reg);
        SPIWriteByte(mask);
        SPIWriteByte(value);
        CAN_DeSelect();
       
        fail = (CANReadReg(reg) & mask) != value; 
        if(fail) {
            DEBUG_E("Bit Modify Failed!\n\r");
        }
//    } while (fail);
}

static void set_can_mode(BYTE mode)
{
	unsigned char result;
#ifdef TEST
	UINT16 delay;
	UINT16 loop = 0;
#endif

#ifdef TEST
	DEBUG_D("set_can_mode(0x%x)\n\r", mode);
#endif

	CANSetRegMaskValue(CANCTRL, MODE_MASK, mode);
#ifdef TEST
	result = CANReadReg(CANCTRL);

	while((result & MODE_MASK) != mode) {
		DEBUG_D("Before write CANCTRL 0x%x\n\r", result & MODE_MASK);
		CANSetRegMaskValue(CANCTRL, MODE_MASK, mode);
		result = CANReadReg(CANCTRL);
#ifdef TEST
		for (delay = 0; delay < 0x100; delay++) {
			Nop();
		}
#endif
	}
#endif

	do {
#ifdef TEST
		loop++;
		for (delay = 0; delay < 0x100; delay++) {
			Nop();
		}

		if(loop == 0) {
#if LOG_LEVEL <= LOG_ERROR
//            DEBUG_E("Error Failing to set CAN Mode\n\r");
//            DEBUG_E("CANCTRL 0x%x\n\r", result & MODE_MASK);
//            result = CANReadReg(CANSTAT);
//            DEBUG_E("CANSTAT 0x%x\n\r", result & MODE_MASK);
#endif
			//            stall();
		}
#endif
		result = CANReadReg(CANSTAT);
	} while ( (result & MODE_MASK) != mode );
}

/**
 * \brief Set the Baud rate of the CAN Connection to the BUS
 *
 * TQ = 2BRP/Fosc
 *
 * \param baudRate baud rate to set
 */
static void setBitRate(baud_rate_t baudRate)
{
	BYTE sjw = 0;
	BYTE brp = 0;
	BYTE phseg1 = 0;
	BYTE phseg2 = 0;
	BYTE propseg = 0;

	switch(baudRate)
	{
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
		DEBUG_E("Invalid Baud Rate Specified\n\r");
		break;
	}

	set_can_mode(CONFIG_MODE);

	CANSetRegMaskValue(CNF1_REG, SJW_MASK, (sjw - 1) << 6 );
	CANSetRegMaskValue(CNF1_REG, BRP_MASK, (brp) );

	CANSetRegMaskValue(CNF2_REG, BTLMODE_MASK, 0x80 );
	CANSetRegMaskValue(CNF2_REG, SAM_MASK, 0x00 );
	CANSetRegMaskValue(CNF2_REG, PSEG1_MASK, (phseg1 - 1) << 3 );
	CANSetRegMaskValue(CNF2_REG, PROPSEG_MASK, (propseg - 1) );
	
	CANSetRegMaskValue(CNF3_REG, WAKFIL_MASK, 0x00 );
	CANSetRegMaskValue(CNF3_REG, PSEG2_MASK, (phseg2 - 1) );
}

void L2_getStatus(can_status_t *arg_status, baud_rate_t *arg_baud)
{
	*arg_status = status;
	*arg_baud = baud;
}

baud_rate_t L2_GetCanBuadRate(void)
{
	return(connectedBaudRate);
}

void L2_SetCanNodeBuadRate(baud_rate_t baudRate)
{
	es_timer timer;

	DEBUG_D("L2_SetCanNodeBuadRate()\n\r");
	TIMER_INIT(timer);

	status.bit_field.l2_status = L2_ChangingBaud;
	baud = baudRate;

	if (status_handler)
		status_handler(L2_STATUS_MASK, status, baud);

	set_can_mode(CONFIG_MODE);

	setBitRate(baudRate);

	/*
	 * The Baud rate is being changed so going to stay in config mode
	 * for 10 Seconds and let the Network settle down.
	 */
	start_timer(SECONDS_TO_TICKS(5), exp_finaliseBaudRateChange, (union sigval)(void *)NULL, &timer);
}

static void exp_finaliseBaudRateChange(union sigval data __attribute__((unused)))
{
	DEBUG_D("exp_finaliseBaudRateChange()\n\r");

        set_can_mode(NORMAL_MODE);

	status.bit_field.l2_status = L2_Connected;

	if (status_handler)
		status_handler(L2_STATUS_MASK, status, baud);
}

/*
 * TODO Change name to initiate
 */
void L2_SetCanNetworkBuadRate(baud_rate_t rate)
{
	es_timer timer;
	can_frame msg;
	result_t result = SUCCESS;

	DEBUG_D("L2_SetCanNetworkBuadRate()\n\r");
	TIMER_INIT(timer);

	msg.can_id = 0x705;
	msg.can_dlc = 1;

	msg.data[0] = rate;

	result = l2_tx_frame(&msg);

	if (result == SUCCESS) {
		status.bit_field.l2_status = L2_ChangingBaud;
		baud = rate;
		ChangingBaudTxError = 0;

		if (status_handler)
			status_handler(L2_STATUS_MASK, status, baud);

        //ToDo
		start_timer(MILLI_SECONDS_TO_TICKS(500), exp_resendBaudRateChange, (union sigval)(void *)NULL, &timer);
	}
}

static void exp_resendBaudRateChange(union sigval data __attribute__((unused)))
{
	can_frame msg;
	es_timer timer;

        TIMER_INIT(timer);

	DEBUG_D("exp_resendBaudRateChange()\n\r");

	if(ChangingBaudTxError < 3) {
		DEBUG_D("resending Baud Rate Change Request %d\n\r", ChangingBaudTxError);
		msg.can_id = 0x705;
		msg.can_dlc = 1;

		msg.data[0] = baud;

		if(l2_tx_frame(&msg) != ERR_CAN_NO_FREE_BUFFER)
			start_timer(MILLI_SECONDS_TO_TICKS(500), exp_resendBaudRateChange, (union sigval)(void *)NULL, &timer);
		else {
			DEBUG_D("No Free Buffers so change the Baud Rate\n\r");
			CANSetRegMaskValue(TXB0CTRL, TXREQ, 0x00);
			CANSetRegMaskValue(TXB1CTRL, TXREQ, 0x00);
			CANSetRegMaskValue(TXB2CTRL, TXREQ, 0x00);
			L2_SetCanNodeBuadRate(baud);
		}
	} else {
		DEBUG_D("3 Errors so NOT Resending Baud Rate Change Request\n\r");
		L2_SetCanNodeBuadRate(baud);
	}
}

static void CANEnableRXInterrupts(void)
{
	DEBUG_D("CANEnableRXInterrupts\n\r");
	CANSetRegMaskValue(CANINTE, RX1IE, RX1IE);
	CANSetRegMaskValue(CANINTE, RX0IE, RX0IE);
}

static void CANDisableRXInterrupts(void)
{
	DEBUG_D("CANDisableRXInterrupts\n\r");
	CANSetRegMaskValue(CANINTE, RX1IE, 0x00);
	CANSetRegMaskValue(CANINTE, RX0IE, 0x00);
}

static u8 CANReadReg(BYTE reg)
{
	u8 value;
	CAN_Select();
	SPIWriteByte(CAN_READ_REG);
	SPIWriteByte(reg);
	value = SPIWriteByte(0x00);
	CAN_DeSelect();

	return(value);
}

static void CANWriteReg(BYTE reg, BYTE value)
{
	CAN_Select();
	SPIWriteByte(CAN_WRITE_REG);
	SPIWriteByte(reg);
	SPIWriteByte(value);
	CAN_DeSelect();
}

static void CANReadRxBuffer(BYTE reg, BYTE *buffer)
{
	BYTE loop = 0x00;
	BYTE *ptr;
	BYTE dataLength = 0x00;

	ptr = buffer;
	CAN_Select();
	SPIWriteByte(CAN_READ_REG);
	SPIWriteByte(reg);

	/*
	 * First pull out the five header bytes in the message
	 */
	for(loop = 0; loop < 5; loop++, ptr++) {
		*ptr = SPIWriteByte(0x00);
	}

	dataLength = buffer[4] & 0x0f;
	if(dataLength > CAN_DATA_LENGTH) {
		DEBUG_E("Invalid Data Length %x & 0x0f = %x\n\r", buffer[4], buffer[4] & 0x0f);
	} else {
		for (loop = 0; loop < dataLength; loop++, ptr++) {
			*ptr = SPIWriteByte(0x00);
		}
        }

	CAN_DeSelect();
}

BYTE CANFindFreeTxBuffer(void)
{
	BYTE ctrlValue = 0x00;

	ctrlValue = CANReadReg(TXB0CTRL);
	if(!(ctrlValue & 0x08))
		return(TXB0CTRL);

	ctrlValue = CANReadReg(TXB1CTRL);
	if(!(ctrlValue & 0x08))
		return(TXB1CTRL);

	ctrlValue = CANReadReg(TXB2CTRL);
	if(!(ctrlValue & 0x08))
		return(TXB2CTRL);
	else
		return(0xff);
}

#if LOG_LEVEL < NO_LOGGING
void printErrorCounts(void)
{
	BYTE byte;
	static BYTE rec = 0x00;
	static BYTE tec = 0x00;
	static BYTE eflg = 0x00;

	byte = CANReadReg(TEC);
	if(byte != tec) {
		tec = byte;
		DEBUG_E("Tx Error (TEC) Count Change - 0x%x\n\r", tec);
	}

	byte = CANReadReg(REC);
	if(byte != rec) {
		rec = byte;
		DEBUG_E("Rx Error (REC) Count Change - 0x%x\n\r", rec);
	}

	byte = CANReadReg(EFLG);
	if(byte != eflg) {
		eflg = byte;
		DEBUG_E("EFLG Changed - 0x%x\n\r", eflg);
	}
}
#endif

#ifdef TEST
void test_can()
{
	BYTE byte;

	CAN_Select();
	byte = CANReadReg(CANCTRL);
	CAN_DeSelect();

	DEBUG_D("Test read of CANCTRL - 0x%x\n\r", byte);
}
#endif

static void l2_dispatcher_frame_handler(can_frame *message)
{
	BYTE loop;
	BOOL found = FALSE;

	DEBUG_D("L2_CanDispatcherL2MsgHandler 0x%lx\n\r", message->can_id);

	for (loop = 0; loop < REGISTER_ARRAY_SIZE; loop++) {
		if( (registered[loop].used)
		    &&((message->can_id & registered[loop].target.mask) == (registered[loop].target.filter & registered[loop].target.mask))) {
			registered[loop].target.handler(message);
			found = TRUE;
		}
	}

	if(!found) {
		/*
		 * No handler found so pass the received message to the Application
		 */
		DEBUG_D("No Handler for 0x%lx\n\r", message->can_id);
	}
}

static BYTE l2_can_dispatch_reg_handler(can_target_t *target)
{
	return(FALSE);
}

result_t l2_reg_handler(can_target_t *target)
{
	BYTE loop;
	DEBUG_D("sys_l2_can_dispatch_reg_handler mask %lx, filter %lx Handler %lx\n\r",
		   target->mask,
		   target->filter,
		   target->handler);
    /*
     * clean up the target in case the caller has included spurious bits
     */
	if(target->mask & CAN_EFF_FLAG) {
		target->mask = target->mask & (CAN_EFF_FLAG | CAN_EFF_MASK);
	} else {
		target->mask = target->mask & CAN_SFF_MASK;
	}

	// Find a free slot
	for(loop = 0; loop < REGISTER_ARRAY_SIZE; loop++) {
		if(registered[loop].used == FALSE) {
			DEBUG_D("Target stored at target %d\n\r", loop);
			registered[loop].used = TRUE;
			registered[loop].target.mask = target->mask;
			registered[loop].target.filter = target->filter;
			registered[loop].target.handler = target->handler;
			return(SUCCESS);
		}
	}
	return(ERR_NO_RESOURCES);
}

result_t l2_can_dispatch_unreg_handler(BYTE id)
{
	if(id < REGISTER_ARRAY_SIZE) {
		if (registered[id].used) {
			registered[id].used = FALSE;
			registered[id].target.mask = 0x00;
			registered[id].target.filter = 0x00;
			registered[id].target.handler = (void (*)(can_frame *))NULL;
			return (SUCCESS);
		}
	}
	return(ERR_GENERAL_ERROR);
}
