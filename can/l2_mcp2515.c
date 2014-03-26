/**
 *
 * \file es_can/can/l2_mcp2515.c
 *
 * CAN L2 Functionality for MCP2515 for electronicSoup CAN code
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
#include "es_can/core.h"
#include "system.h"

#include "es_can/can/es_can.h"
#include "es_can/can/l2_mcp2515.h"
#include "es_can/timers/timer_sys.h"

#include "es_can/utils/utils.h"

#if LOG_LEVEL < NO_LOGGING
    #include <stdio.h>
    #define TAG "MCP2515"
#endif

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

static can_status_t status = Uninitialised;
static baud_rate_t baud = no_baud;

static es_timer listen_timer;
static es_timer ping_timer;

static can_status_handler status_handler = (can_status_handler)NULL;

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
                 can_status_handler arg_status_handler)
{
	u8 loop = 0x00;
	u8 exitMode = NORMAL_MODE;
	u32 delay;
        result_t result;

#if LOG_LEVEL <= LOG_DEBUG
	//    printErrorCounts();
	serial_log(Debug, TAG, "l2_init()\n\r");
#endif

	listen_timer.status = INACTIVE;
	ping_timer.status = INACTIVE;

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

#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "Set CAN Mode to CONFIG\n\r");
#endif
	set_can_mode(CONFIG_MODE);
#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "Set CAN Mode SET\n\r");
#endif

	CANWriteReg(RXB0CTRL, 0x64);
	CANWriteReg(RXB1CTRL, 0x60);
	CANWriteReg(TXRTSCTRL, 0x00);
	CANWriteReg(BFPCTRL, 0x00);

#if 0
	//
	// Clear Mask 0
	//
	CAN_Select();
	SPIWriteByte(CAN_WRITE_REG);
	SPIWriteByte(RXM0SIDH);

	for(loop = 0; loop < 4; loop++)
		SPIWriteByte(0x00);

	CAN_DeSelect();

	//
	// Clear Mask 1
	//
	CAN_Select();
	SPIWriteByte(CAN_WRITE_REG);
	SPIWriteByte(RXM1SIDH);

	for(loop = 0; loop < 4; loop++)
		SPIWriteByte(0x00);

	CAN_DeSelect();
#endif
	/*
	 * Set up MCP2515 I/O Pins
	 */
#ifdef MCP2515_INPUT_0
	CANSetRegMaskValue(TXRTSCTRL, B0RTSM, 0x00);
#endif
#ifdef MCP2515_INPUT_1
	CANSetRegMaskValue(TXRTSCTRL, B1RTSM, 0x00);
#endif
#ifdef MCP2515_INPUT_2
	CANSetRegMaskValue(TXRTSCTRL, B2RTSM, 0x00);
#endif

#ifdef MCP2515_OUTPUT_0
//    CANWriteReg(BFPCTRL, 0x3C);
//    CANSetRegMaskValue(BFPCTRL, B0BFE | B0BFM | B0BFS, B0BFE);
#if LOG_LEVEL <= LOG_DEBUG
	//    printErrorCounts();
	serial_log(Debug, TAG, "BFPCTRL initialised to 0x%x\n\r", CANReadReg(BFPCTRL));
#endif
#endif  // MCP2515_OUTPUT_0

#ifdef MCP2515_OUTPUT_1
	CANSetRegMaskValue(BFPCTRL, B1BFE | B1BFM | B1BFS, B1BFE);
#endif

	/**
	 * Have to set the baud rate if one has been passed into the function
	 */
	if(arg_baud_rate <= BAUD_MAX) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Valid Baud Rate specified - %s\n\r", baud_rate_strings[arg_baud_rate]);
#endif
		connectedBaudRate = arg_baud_rate;
		setBitRate(arg_baud_rate);
		exitMode = NORMAL_MODE;

		status = Connecting;
		baud = arg_baud_rate;

		if(status_handler)
			status_handler(status, baud);
	} else {
		/*
		 * Have to search for the Networks baud rate. Start at the bottom
		 */
		rxMsgCount = 0;
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Before trying 10K REC - %d, rxCount - %d\n\r", CANReadReg(REC), rxMsgCount);
#endif
		listenBaudRate = baud_10K;
		setBitRate(listenBaudRate);
		exitMode = LISTEN_MODE;

		connectingErrors = 0;
		status = Listening;
		baud = listenBaudRate;

#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Call the status handler\n\r");
#endif
		if(status_handler)
			status_handler(status, baud);
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Return from status handler\n\r");
#endif

		/* Now wait and see if we have errors */
		start_timer(LISTEN_TIME, exp_checkNetworkConnection, (union sigval)(void *)NULL, &listen_timer);
	}

#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "Set Exit Mode 0x%x\n\r", exitMode);
#endif

	set_can_mode(exitMode);

//        CANWriteReg(CANINTF, 0x00);
//	CANWriteReg(CANINTE, 0x00);
#ifdef L2_CAN_INTERRUPT_DRIVEN
	CANWriteReg(CANINTE, MERRE | ERRIE | TX2IE | TX1IE | TX0IE | RX1IE | RX0IE);
//	CANWriteReg(CANINTE, MERRE | RX1IE | RX0IE);
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
	if(status == Connecting) {
#if LOG_LEVEL <= LOG_DEBUG
        serial_log(Debug, TAG, "Connecting send a test Ping message!\n\r");
#endif
		result = send_ping();
		if (result != SUCCESS) {
#if LOG_LEVEL <= LOG_ERROR
			serial_log(Error, TAG, "Failed to send a test Ping message!\n\r");
#endif
		}
	}

// Create a random timer between 1 and 1.5 seconds for firing the
	// Network Idle Ping message
#if defined(CAN_IDLE_PING)
	ping_time = (u16)((rand() % 500) + 1000);
	start_timer(ping_time, exp_test_ping, (union sigval)(void *)NULL, &ping_timer);
#endif

#if LOG_LEVEL <= LOG_DEBUG
	//    printErrorCounts();
	serial_log(Debug, TAG, "CAN Layer 2 Initialised\n\r");
#endif
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

#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "exp_test_ping()\n\r");
//    serial_log(Debug, TAG, "CANINTF %x\n\r", CANReadReg(CANINTF));
#endif
	flags = CANReadReg(CANINTF);
	ping_timer.status = INACTIVE;

//    if((flags != 0x00) && !PORTAbits.RA14)
	if((flags != 0x00) && !CAN_INTERRUPT) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Error, TAG, "-> Interrupt line Low Flags Set!\n\r");
#endif
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

	listen_timer.status = INACTIVE;

#if LOG_LEVEL <= LOG_DEBUG
	if(listenBaudRate <= BAUD_MAX)
		serial_log(Debug, TAG, "After trying %s Errors - %d, rxCount - %ld\n\r", baud_rate_strings[listenBaudRate], connectingErrors, rxMsgCount);
	else
		serial_log(Debug, TAG, "After trying %s Errors - %d, rxCount - %ld\n\r", "NO BAUD RATE", connectingErrors, rxMsgCount);
#endif

	/*
	 * If we heard valid messages with no errors we've found the baud rate.
	 */
	if(rxMsgCount > 0 && connectingErrors == 0) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "*** Network connected ***\n\r");
#endif
		connectedBaudRate = listenBaudRate;

		set_can_mode(NORMAL_MODE);

		status = Connected;
		baud = connectedBaudRate;

		if(status_handler)
			status_handler(status, baud);

		// Test code to periodically send message
		// message 708 now used by CancelNetLogger message
// start_timer(SECONDS_TO_TICKS(30), exp_test_ping, NULL);
	} else {
		if(listenBaudRate == BAUD_MAX)
			listenBaudRate = baud_10K;
		else
			listenBaudRate++;
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "No joy try Baud Rate - %s\n\r", baud_rate_strings[listenBaudRate]);
#endif
		set_can_mode(CONFIG_MODE);
		setBitRate(listenBaudRate);
		set_can_mode(LISTEN_MODE);

                rxMsgCount = 0;
		connectingErrors = 0;
		status = Listening;
	        baud = listenBaudRate;

		if(status_handler)
			status_handler(status, baud);

#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Restart timer\n\r");
#endif
		result = start_timer(LISTEN_TIME,
				     exp_checkNetworkConnection,
				     (union sigval)(void *)NULL,
				     &listen_timer);
		if(result != SUCCESS) {
#if LOG_LEVEL <= LOG_ERROR
			serial_log(Error, TAG, "Failed to start listen timer, result 0x%x\n\r", result);
#endif
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
#if LOG_LEVEL <= LOG_DEBUG
    serial_log(Debug, TAG, "*** ISR *** flags 0x%x\n\r", flags);
#endif

    if(flags == 0x00) {
		canstat = CANReadReg(CANSTAT);
        CANWriteReg(CANINTF, 0x00);

#if LOG_LEVEL <= LOG_WARNING
		serial_log(Warning, TAG, "*** ISR with zero flags! IOCD %x\n\r", canstat & IOCD);
#endif
	}
#endif

	while(flags != 0x00) {
		canstat = CANReadReg(CANSTAT);
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "*** ISR Flag-%x, IOCD-%x\n\r", flags, (canstat & IOCD));
#endif
		if (flags & ERRIE) {
			eflg = CANReadReg(EFLG);
#if LOG_LEVEL <= LOG_ERROR
			serial_log(Error, TAG, "*** CAN ERRIR Flag!!!\n\r");
			serial_log(Error, TAG, "*** CAN EFLG %x\n\r", eflg);
#endif
			if(status == Listening)
				connectingErrors++;

			/*
			 * Clear any rx Frames as there's been an error
			 */
			CANSetRegMaskValue(EFLG, RX1OVR | RX0OVR, 0x00);

			CANSetRegMaskValue(CANINTF, ERRIE, 0x00);
			flags = flags & ~(RX0IE | RX1IE);
		}

		if (flags & MERRE) {
#if LOG_LEVEL <= LOG_WARNING
			serial_log(Warning, TAG, "CAN MERRE Flag\n\r");
#endif
			if(status == Listening)
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
#if LOG_LEVEL <= LOG_ERROR
				serial_log(Error, TAG, "Transmit Buffer Failed to send\n\r");
#endif
				CANSetRegMaskValue(ctrl, TXREQ, 0x00);
				if (status == ChangingBaud)
					ChangingBaudTxError++;
			}
			ctrl = ctrl + 0x10;
		}

		if (flags & RX0IE) {
#if LOG_LEVEL <= LOG_DEBUG
			serial_log(Debug, TAG, "RX0IE\n\r");
#endif
			/*
			 * Increment the rx count in case we're listening for Baud
			 * Rate seettings.
			 */
			if (status == Listening) {
				rxMsgCount++;
			} else {
				if (cirBufferCount < CAN_RX_CIR_BUFFER_SIZE) {
					CANReadRxBuffer(RXB0SIDH, (BYTE *) & cirBuffer[cirBufferNextWrite]);
					cirBufferNextWrite = (cirBufferNextWrite + 1) % CAN_RX_CIR_BUFFER_SIZE;
					cirBufferCount++;
				} else {
#if LOG_LEVEL <= LOG_ERROR
					serial_log(Error, TAG, "Circular Buffer overflow!\n\r");
#endif
				}
			}

			CANSetRegMaskValue(CANINTF, RX0IE, 0x00);
		}

		if (flags & RX1IE) {
#if LOG_LEVEL <= LOG_DEBUG
			serial_log(Debug, TAG, "RX1IE\n\r");
#endif
			/*
			 * Incrememnt the rx count incase we're listening for Baud
			 * Rate seettings.
			 */
			if (status == Listening) {
				rxMsgCount++;
			} else {
				if (cirBufferCount < CAN_RX_CIR_BUFFER_SIZE) {
					CANReadRxBuffer(RXB1SIDH, (BYTE *) & cirBuffer[cirBufferNextWrite]);
					cirBufferNextWrite = (cirBufferNextWrite + 1) % CAN_RX_CIR_BUFFER_SIZE;
					cirBufferCount++;
				} else {
#if LOG_LEVEL <= LOG_ERROR
					serial_log(Error, TAG, "Circular Buffer overflow!\n\r");
#endif
				}
			}

			CANSetRegMaskValue(CANINTF, RX1IE, 0x00);
		}

		if (flags & TX2IE) {
#if LOG_LEVEL <= LOG_DEBUG
			serial_log(Debug, TAG, "TX2IE\n\r");
#endif
			if (status == Connecting) {
				status == Connected;

				if (status_handler)
					status_handler(status, baud);
			}

			CANSetRegMaskValue(CANINTF, TX2IE, 0x00);
		}

		if (flags & TX1IE) {
#if LOG_LEVEL <= LOG_DEBUG
			serial_log(Debug, TAG, "TX1IE\n\r");
#endif
			if (status == Connecting) {
				status == Connected;

				if (status_handler)
					status_handler(status, baud);
			}
			CANSetRegMaskValue(CANINTF, TX1IE, 0x00);
		}

		if (flags & TX0IE) {
#if LOG_LEVEL <= LOG_DEBUG
			serial_log(Debug, TAG, "TX0IE\n\r");
#endif
			if (status == Connecting) {
				status == Connected;

				if (status_handler)
					status_handler(status, baud);
			}

			CANSetRegMaskValue(CANINTF, TX0IE, 0x00);
		}
		flags = CANReadReg(CANINTF);
	}
        IFS0bits.INT0IF = 0;
#if defined(L2_CAN_INTERRUPT_DRIVEN)
#if LOG_LEVEL <= LOG_DEBUG
    serial_log(Debug, TAG, "Exit ISR\n\r");
#endif
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
        if(CAN_INTERRUPT) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Call ISR from Tasks\n\r");
#endif
        _INT0Interrupt();
        }
#endif

#if 0
	count++;

	if(count == 0x00) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "L2_CanTasks()\n\r");
#endif
	}
#endif

#if 1
	flags = CANReadReg(CANINTF);
	if(flags != last_flags) {
            last_flags = flags;
        serial_log(Debug, TAG, "!!! - TASKS Flags 0x%x\n\r", flags);
        eflg = CANReadReg(EFLG);
#if LOG_LEVEL <= LOG_DEBUG
        serial_log(Debug, TAG, "*** CAN EFLG %x\n\r", eflg);
#endif
        if(!CAN_INTERRUPT) {
		serial_log(Debug, TAG, "Interrupt line Low Flags Set!\n\r");
//        L2_ISR();
	}
        }
#endif
	while(cirBufferCount > 0) {
		if (status == Connecting) {
			status == Connected;

			if (status_handler)
				status_handler(status, baud);
		}

		/*
		 * make sure the rx msg id is cleared
		 */
		rxCanMsg.can_id = 0x00;

		// Check if it's an extended
		if (cirBuffer[cirBufferNextRead].sidl & SIDL_EXIDE) {
#if 0
			rxCanMsg.header.extended_id = TRUE;
			rxCanMsg.header.rnr_frame = cirBuffer[cirBufferNextRead].dcl & DCL_ERTR;
			rxCanMsg.header.can_id.id = cirBuffer[cirBufferNextRead].sidh;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 3 | (cirBuffer[cirBufferNextRead].sidl >> 5) & 0x07;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 2 | cirBuffer[cirBufferNextRead].sidl & 0x03;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 8 | cirBuffer[cirBufferNextRead].eid8;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 8 | cirBuffer[cirBufferNextRead].eid0;
#endif
			rxCanMsg.can_id = cirBuffer[cirBufferNextRead].sidh;
			rxCanMsg.can_id = rxCanMsg.can_id << 3 | (cirBuffer[cirBufferNextRead].sidl >> 5) & 0x07;
			rxCanMsg.can_id = rxCanMsg.can_id << 2 | cirBuffer[cirBufferNextRead].sidl & 0x03;
			rxCanMsg.can_id = rxCanMsg.can_id << 8 | cirBuffer[cirBufferNextRead].eid8;
			rxCanMsg.can_id = rxCanMsg.can_id << 8 | cirBuffer[cirBufferNextRead].eid0;
			rxCanMsg.can_id |= CAN_EFF_FLAG;

			if(cirBuffer[cirBufferNextRead].dcl & DCL_ERTR)
				rxCanMsg.can_id |= CAN_RTR_FLAG;
		} else {
#if 0
			rxCanMsg.header.extended_id = FALSE;
			rxCanMsg.header.rnr_frame = cirBuffer[cirBufferNextRead].sidl & SIDL_SRTR;
			rxCanMsg.header.can_id.id = cirBuffer[cirBufferNextRead].sidh;
			rxCanMsg.header.can_id.id = rxCanMsg.header.can_id.id << 3 | (cirBuffer[cirBufferNextRead].sidl >> 5) & 0x07;
#endif
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

#if LOG_LEVEL <= LOG_DEBUG
//        serial_log(Debug, TAG, "Received a message id - %lx\n\r", rxCanMsg.header.can_id.id);
#endif
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
#if LOG_LEVEL <= LOG_DEBUG
				serial_log(Debug, TAG, "MCP2515 Input 2 On\n\r");
#endif
			} else {
#if LOG_LEVEL <= LOG_DEBUG
				serial_log(Debug, TAG, "MCP2515 Input 2 Off\n\r");
#endif
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

#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "L2 => Id %lx\n\r", canMsg->can_id);

//    serial_log(Debug, TAG, "Data:\n\r");
//    for (loop = 0; loop < canMsg->header.data_length; loop++)
//    {
//        serial_log(Debug, TAG, "Byte %d - 0x%x\n\r", loop, canMsg->data[loop]);
//    }
#endif

	if(connectedBaudRate == no_baud) {
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "Can't Transmit network not connected!\n\r");
#endif
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
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "ERROR No free Tx Buffers\n\r");
#endif
		return(ERR_CAN_NO_FREE_BUFFER);
	} else if (ctrl == TXB0CTRL) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "TXB0CTRL\n\r");
#endif
		canBuffer = TXB0SIDH;
	} else if (ctrl == TXB1CTRL) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "TXB1CTRL\n\r");
#endif
		canBuffer = TXB1SIDH;
	} else if (ctrl == TXB2CTRL) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "TXB2CTRL\n\r");
#endif
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
	if (status == Connecting) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Network not good so sending OSM\n\r");
#endif
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
		serial_log(Error, TAG, "Wait for transmission to complete\n\r");
#endif
	}

	if(CANReadReg(CANINTF) & MERRE) {
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "MERRE\n\r");
#endif
		// Clear the error flag
		CANSetRegMaskValue(CANINTF, MERRE, 0x00);
		errorCode = CAN_ERROR;
	}

	if(CANReadReg(CANINTF) & ERRIE) {
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "ERRIE\n\r");
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

#if LOG_LEVEL <= LOG_DEBUG
//    serial_log(Debug, TAG, "CheckErrors()\n\r");
	//printErrorCounts();
#endif

	flags = CANReadReg(CANINTF);

	if(flags != 0x00) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "CheckErrors() Flag set 0x%x\n\r", flags);
#endif
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
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "Message Error Flag set! Flasg now 0x%x\n\r", CANReadReg(CANINTF));
#endif
	}

	if (flags & WAKIE) {
		CANSetRegMaskValue(CANINTF, WAKIE, 0x00);
		wakeUpCount++;
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Wake Up Count - now %ld\n\r", wakeUpCount);
#endif
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
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "checkSubErrors() errors - %x\n\r", error);
#endif
	}

	/*
	 * Process the two RX error flags out of the way and clear them
	 */
	if (error & RX1OVR) {
		g_missedMessageCount++;
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Missed Message Count - %ld\n\r", g_missedMessageCount);
#endif

		/*
		 * Clear this flag
		 */
		CANSetRegMaskValue(EFLG, RX1OVR, 0x00);
	}

	if (error & RX0OVR) {
		g_missedMessageCount++;
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Missed Message Count - %ld\n\r", g_missedMessageCount);
#endif

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
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "ERRORS Have changed! Difference is %x\n\r", difference);
#endif

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
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "ERROR Bus OFF!\n\r");
#endif
					} else {
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Error Cleared CAN Bus Active again\n\r");
#endif
					}
				}

				if (mask & TXEP) {
					if (error & TXEP) {
						//ToDo Must send an error to Android
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Transmitter ERROR PASSIVE Error count > 128!\n\r");
#endif
					} else {
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Tx Error count < 128 ;-)\n\r");
#endif
					}
				}

				if (mask & RXEP) {
					if (error & RXEP) {
						//TODO Must send an error to Android
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Receiver ERROR PASSIVE Error count > 128!\n\r");
#endif
					} else {
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Rx Error < 128 :-)\n\r");
#endif
					}
				}

				if (mask & TXWAR) {
					if (error & TXWAR) {
						//TODO Must send an error to Android
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Transmitter WARNING Error count > 96!\n\r");
#endif
					} else {
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Tx Warning cleared Error Count < 96\n\r");
#endif
					}
				}

				if (mask & TXBO) {
					if (error & RXWAR) {
						//TODO Must send an error to Android
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Receiver WARNING Error count > 96!\n\r");
#endif
					} else {
#if LOG_LEVEL <= LOG_ERROR
						serial_log(Error, TAG, "Rx Warning Cleared Error Count < 96\n\r");
#endif
					}
				}
			}
		}
		g_CanErrors = error;
	} else {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Errors Have not changed\n\r");
#endif
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
#ifdef TEST
#if LOG_LEVEL <= LOG_DEBUG
	if(reg == CANCTRL) {
//        serial_log(Debug, TAG, "Writing to CANTCTRL mask 0x%x\n\r", mask);
	}
#endif
#endif
//    do {
        CAN_Select();
        SPIWriteByte(CAN_BIT_MODIFY);
        SPIWriteByte(reg);
        SPIWriteByte(mask);
        SPIWriteByte(value);
        CAN_DeSelect();
       
        fail = (CANReadReg(reg) & mask) != value; 
        if(fail) {
#if LOG_LEVEL <= LOG_ERROR
		printf("-");
//            serial_log(ERROR, TAG, "Bit Modify Failed!\n\r");
#endif            
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
#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "set_can_mode(0x%x)\n\r", mode);
#endif
#endif

	CANSetRegMaskValue(CANCTRL, MODE_MASK, mode);
#ifdef TEST
	result = CANReadReg(CANCTRL);

    while((result & MODE_MASK) != mode) {
#if LOG_LEVEL <= LOG_DEBUG
	    printf(".");
//        serial_log(Debug, TAG, "Before write CANCTRL 0x%x\n\r", result & MODE_MASK);
#endif
	    CANSetRegMaskValue(CANCTRL, MODE_MASK, mode);
	    result = CANReadReg(CANCTRL);
#if LOG_LEVEL <= LOG_DEBUG
//        serial_log(Debug, TAG, "After Write CANCTRL 0x%x\n\r", result & MODE_MASK);
#endif

#ifdef TEST
	    for (delay = 0; delay < 0x100; delay++) {
		    Nop();
	    }
#endif
    }
#endif

    do {
#ifdef TEST
#if LOG_LEVEL <= LOG_DEBUG
            printf("*");
#endif
	    loop++;
	    for (delay = 0; delay < 0x100; delay++) {
		    Nop();
	    }

	    if(loop == 0) {
#if LOG_LEVEL <= LOG_ERROR
//            serial_log(Error, TAG, "Error Failing to set CAN Mode\n\r");
//            serial_log(Error, TAG, "CANCTRL 0x%x\n\r", result & MODE_MASK);
//            result = CANReadReg(CANSTAT);
//            serial_log(Error, TAG, "CANSTAT 0x%x\n\r", result & MODE_MASK);
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
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG,  "Invalid Baud Rate Specified\n\r");
#endif
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
#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "L2_SetCanNodeBuadRate()\n\r");
#endif
	timer.status = INACTIVE;

	status = ChangingBaud;
	baud = baudRate;

	if (status_handler)
		status_handler(status, baud);

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
#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "exp_finaliseBaudRateChange()\n\r");
#endif
	set_can_mode(NORMAL_MODE);

	status = Connected;

	if (status_handler)
		status_handler(status, baud);
}

/*
 * TODO Change name to initiate
 */
void L2_SetCanNetworkBuadRate(baud_rate_t rate)
{
	es_timer timer;
	can_frame msg;
	result_t result = SUCCESS;

#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "L2_SetCanNetworkBuadRate()\n\r");
#endif
	timer.status = INACTIVE;

	msg.can_id = 0x705;
	msg.can_dlc = 1;

	msg.data[0] = rate;

	result = l2_tx_frame(&msg);

	if (result == SUCCESS) {
		status = ChangingBaud;
		baud = rate;
		ChangingBaudTxError = 0;

		if (status_handler)
			status_handler(status, baud);

        //ToDo
		start_timer(MILLI_SECONDS_TO_TICKS(500), exp_resendBaudRateChange, (union sigval)(void *)NULL, &timer);
	}
}

static void exp_resendBaudRateChange(union sigval data __attribute__((unused)))
{
	can_frame msg;
	es_timer timer;

	timer.status = INACTIVE;

#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "exp_resendBaudRateChange()\n\r");
#endif

	if(ChangingBaudTxError < 3) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "resending Baud Rate Change Request %d\n\r", ChangingBaudTxError);
#endif
		msg.can_id = 0x705;
		msg.can_dlc = 1;

		msg.data[0] = baud;

		if(l2_tx_frame(&msg) != ERR_CAN_NO_FREE_BUFFER)
			start_timer(MILLI_SECONDS_TO_TICKS(500), exp_resendBaudRateChange, (union sigval)(void *)NULL, &timer);
		else {
#if LOG_LEVEL <= LOG_DEBUG
			serial_log(Debug, TAG, "No Free Buffers so change the Baud Rate\n\r");
#endif
			CANSetRegMaskValue(TXB0CTRL, TXREQ, 0x00);
			CANSetRegMaskValue(TXB1CTRL, TXREQ, 0x00);
			CANSetRegMaskValue(TXB2CTRL, TXREQ, 0x00);
			L2_SetCanNodeBuadRate(baud);
		}
	} else {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "3 Errors so NOT Resending Baud Rate Change Request\n\r");
#endif
		L2_SetCanNodeBuadRate(baud);
	}
}

static void CANEnableRXInterrupts(void)
{
#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "CANEnableRXInterrupts\n\r");
#endif
	CANSetRegMaskValue(CANINTE, RX1IE, RX1IE);
	CANSetRegMaskValue(CANINTE, RX0IE, RX0IE);
}

static void CANDisableRXInterrupts(void)
{
#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "CANDisableRXInterrupts\n\r");
#endif
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
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "Invalid %x & 0x0f = %x\n\r", buffer[4], buffer[4] & 0x0f);
#endif
	}

	for(loop = 0; loop < dataLength; loop++, ptr++) {
		*ptr = SPIWriteByte(0x00);
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
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "Tx Error (TEC) Count Change - 0x%x\n\r", tec);
#endif
	}

	byte = CANReadReg(REC);
	if(byte != rec) {
		rec = byte;
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "Rx Error (REC) Count Change - 0x%x\n\r", rec);
#endif
	}

	byte = CANReadReg(EFLG);
	if(byte != eflg) {
		eflg = byte;
#if LOG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "EFLG Changed - 0x%x\n\r", eflg);
#endif
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

#if LOG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "Test read of CANCTRL - 0x%x\n\r", byte);
#endif
}
#endif

#ifdef MCP2515_OUTPUT_0
void set_output_0(BYTE value)
{
	if(value) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Set Output 0\n\r");
#endif
		CANSetRegMaskValue(BFPCTRL, B0BFS, B0BFS);
	} else {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Clear Output 0\n\r");
#endif
		CANSetRegMaskValue(BFPCTRL, B0BFS, 0x00);
	}
}
#endif

#ifdef MCP2515_OUTPUT_1
void set_output_1(BYTE value)
{
#if LOG_LEVEL <= LOG_DEBUG
        serial_log(Debug, TAG, "set_output_1(0x%x)\n\r", value);
#endif
	if(value) {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Set Output 1\n\r");
#endif
		CANSetRegMaskValue(BFPCTRL, B1BFS, B1BFS);
	} else {
#if LOG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Clear Output 0\n\r");
#endif
		CANSetRegMaskValue(BFPCTRL, B1BFS, 0x00);
	}
}
#endif

static void l2_dispatcher_frame_handler(can_frame *message)
{
	BYTE loop;
	BOOL found = FALSE;

#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "L2_CanDispatcherL2MsgHandler 0x%lx\n\r", message->can_id);
#endif

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
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "No Handler for 0x%lx\n\r", message->can_id);
#endif
	}
}

static BYTE l2_can_dispatch_reg_handler(can_target_t *target)
{
	return(FALSE);
}

result_t l2_reg_handler(can_target_t *target)
{
	BYTE loop;
#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "sys_l2_can_dispatch_reg_handler mask %lx, filter %lx Handler %lx\n\r",
		   target->mask,
		   target->filter,
		   target->handler);
#endif
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
#if DEBUG_LEVEL <= LOG_DEBUG
			serial_log(Debug, TAG, "Target stored at target %d\n\r", loop);
#endif
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

