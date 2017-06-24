#include "libesoup_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "l2_pic18f.h"

#include "libesoup/timers/sw_timers.h"

#define DEBUG_FILE TRUE
#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/can/es_can.h"

#define TAG "PIC_CAN"

#undef L2_CAN_INTERRUPT_DRIVEN

#if SYS_DEBUG_LEVEL < NO_LOGGING
char baud_rate_strings[8][10] = {
    "baud_10K",
    "baud_20K",
    "baud_50K",
    "baud_125K",
    "baud_250K",
    "baud_500K",
    "baud_800K",
    "baud_1M"
};
#endif

/**
 * \brief Network Idle functionality
 *
 * We need to have CAN Bus Network traffic so that when devices connect
 * to listen to the Network to establish the CAN Bus Baud Rate. As a result
 * we'll keep a timer and if nothing has been received or transmitted in this
 * time we'll fire a ping message.
 */
static uint16_t     networkIdleDuration = 0;
static uint8_t      networkIdleTimer;
//static ty_CanStatus canStatus = Listening;

static void pingNetwork(timer_t timer, union sigval data);

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

#define     TX_BUFFERS  6
#define     RX_BUFFERS  5

canBuffer_t *tx_buffers[TX_BUFFERS];
canBuffer_t *rx_buffers[RX_BUFFERS];

#endif

#define MASKS 2

can_mask masks[MASKS] =
{
    {(uint8_t *)&RXM0SIDH, (uint8_t *)&RXM0SIDL, (uint8_t *)&RXM0EIDH, (uint8_t *)&RXM0EIDL},
    {(uint8_t *)&RXM1SIDH, (uint8_t *)&RXM1SIDL, (uint8_t *)&RXM1EIDH, (uint8_t *)&RXM1EIDL}
};


static void setMode(uint8_t mode);
static void setBitRate(can_baud_rate_t baudRate);
static void finaliseBaudRateChange(uint8_t *data);

result_t can_l2_init(can_baud_rate_t arg_baud_rate, void (*arg_status_handler)(uint8_t mask, can_status_t status, can_baud_rate_t baud))
{
#if 0
	uint8_t loop;

	if (baudRate <= BAUD_MAX) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "L2_CanInit() Baud Rate %s\n\r", baud_rate_strings[baudRate]);
#endif
	} else {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "L2_CanInit() ToDo!!! No Baud Rate Specified\n\r");
#endif
		return (baudRate);
	}

	l2Handler = handler;

	/*
	 * Set up the SYS_CAN Configuration
	 */

	// IO Settings for SYS_CAN Tx and Rx on Port B
	TRISBbits.TRISB2 = 0;
	TRISBbits.TRISB3 = 1;

	setMode(CONFIG_MODE);

	/*
	 * Set the Baud rate.
	 */
	set_bit_rate(baudRate);

	/*
	 * Set out Functional mode of opperation
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
#else
	// Mode 2 FIFO mode
	ECANCONbits.MDSEL0 = 0;
	ECANCONbits.MDSEL1 = 1;

	/*
	 * Set 6 Additional buffers
	 * 3 (5,4,3) Tx
	 * 3 (2,1,0) Rx
	 */
	BSEL0 = 0xe0;

	/*
	 * Set FIFO to interrupt when one RX Buffer left
	 */
	ECANCONbits.FIFOWM = 1;

	tx_buffers[0] = (canBuffer_t *) & TXB0CON;
	tx_buffers[1] = (canBuffer_t *) & TXB1CON;
	tx_buffers[2] = (canBuffer_t *) & TXB2CON;
	tx_buffers[3] = (canBuffer_t *) & B3CON;
	tx_buffers[4] = (canBuffer_t *) & B4CON;
	tx_buffers[5] = (canBuffer_t *) & B5CON;

	rx_buffers[0] = (canBuffer_t *) & RXB0CON;
	rx_buffers[1] = (canBuffer_t *) & RXB1CON;
	rx_buffers[2] = (canBuffer_t *) & B0CON;
	rx_buffers[3] = (canBuffer_t *) & B1CON;
	rx_buffers[4] = (canBuffer_t *) & B2CON;

	/*
	 * Recieve all valid messages
	 */
	for (loop = 0; loop < RX_BUFFERS; loop++) {
		rx_buffers[loop]->ctrl = 0;
	}
#endif

	/*
	 * Disable all filters for the moment
	 */
	for (loop = 0; loop < MASKS; loop++) {
		*(masks[loop].sidh) = 0;
		*(masks[loop].sidl) = 0;
		*(masks[loop].eidh) = 0;
		*(masks[loop].eidl) = 0;
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
	setMode(NORMAL_MODE);

	canStatus = Connected;

	// Create a random timer between 1 and 1.5 seconds for firing the
	// Network Idle Ping message
	networkIdleDuration = (UINT16) ((rand() % 500) + 1000);

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Network Idle Duration set to %d milliSeconds\n\r", networkIdleDuration);
#endif
	networkIdleTimer = start_timer(networkIdleDuration, pingNetwork, NULL);
	return (baudRate);
#endif
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
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "CAN L2 ISR Flag-%x\n\r", flags);
#endif
//    if(flags & MERRE)
//    {
//#if DEBUG_LEVEL <= LOG_DEBUG
//        serial_log(Debug, TAG, "CAN MERRE Flag\n\r");
//#endif
//        /*
//         * We've got an error condition so dump all received messages
//         */
//        if (flags  & (RX0IE | RX1IE))
//        {
//            if (flags & RX0IE)
//            {
//                SYS_CANSetRegMaskValue(CANINTF, RX0IE, 0x00);
//            }
//
//            if (flags & RX1IE)
//            {
//                SYS_CANSetRegMaskValue(CANINTF, RX1IE, 0x00);
//            }
//        }
//        else
//        {
//            /*
//             * Error Raised and not for RX operaton so stop TX
//             */
//            networkGood = FALSE;
//
//            ctrl = TXB0CTRL;
//
//            for(loop = 0; loop < 3; loop++)
//            {
//                txFlags = SYS_CANReadReg(ctrl);
//
//                if(txFlags & TXERR)
//                {
//                    SYS_CANSetRegMaskValue(ctrl, TXREQ, 0x00);
//                }
//
//                ctrl = ctrl + 0x10;
//            }
//        }
//
//        /*
//         * Clear the Error Flag
//         */
//        SYS_CANSetRegMaskValue(CANINTF, MERRE, 0x00);
//    }
//    else
//    {
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
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Circular Buffer overflow!");
#endif
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
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Circular Buffer overflow!");
#endif
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

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "L2_CanTasks()\n\r");
#endif
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

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        log_d(TAG, "Received a message id - %lx\n\r", rxCanMsg.header.can_id.id);
#endif
//        networkGood = TRUE;
	cirBufferNextRead = (cirBufferNextRead + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
	cirBufferCount--;

        if(l2Handler) {
		l2Handler(&rxCanMsg);
	}
	//        processCanL2MsgFn(&rxCanMsg);
}
}
#else
void can_l2_tasks(void)
{
        union sigval data;
        timer_t      timer;
	uint8_t      i;
	uint8_t      buffer;
	uint8_t     *ptr;

	buffer = CANCON & 0x0f;

	/*
	 * Read all the messages present and process them
	 */
	while (rx_buffers[buffer]->ctrl & CNTL_RXFUL) {
		/*
		 * cancel the timer if running we've received a frame
		 */
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Rx L2 Message so restart Idle Timer\n\r");
#endif
		sw_timer_cancel(networkIdleTimer);
		networkIdleTimer = sw_timer_start(networkIdleDuration, pingNetwork, data, &timer);
		if (rx_buffers[buffer]->sidl & SIDL_EXIDE) {
			/*
			 * Extended message identifer ???
			 */
			rxMsg.  .header.extended_id = TRUE;
			rxMsg.header.can_id.id = rx_buffers[buffer]->sidh;
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 3 | ((rx_buffers[buffer]->sidl >> 5) & 0x07);
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 2 | (rx_buffers[buffer]->sidl & 0x03);
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 8 | rx_buffers[buffer]->eid8;
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 8 | rx_buffers[buffer]->eid0;
		} else {
			rxMsg.header.extended_id = FALSE;

			/*
			 * Pull out the identifier
			 */
			rxMsg.header.can_id.id = rx_buffers[buffer]->sidh;
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 3 | ((rx_buffers[buffer]->sidl >> 5) & 0x07);
		}

		/*
		 * Pull out Data Length
		 */
		rxMsg.header.data_length = rx_buffers[buffer]->dcl & 0x0f;
		rxMsg.header.rnr_frame = rx_buffers[buffer]->dcl & DCL_RNR;

		/*
		 * Pull out the data bytes
		 */
		ptr = rx_buffers[buffer]->data;
		for (i = 0; i < rxMsg.header.data_length; i++, ptr++) {
			rxMsg.data[i] = *ptr;
		}

		/* Clear the received flag */
		rx_buffers[buffer]->ctrl &= ~CNTL_RXFUL;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "rxMsg %lx\n\r", rxMsg.header.can_id.id);
#endif

		if(l2Handler) {
			l2Handler(&rxMsg);
		} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "No Handler so ignoring received message\n\r");
#endif
		}
		buffer = SYS_CANCON & 0x0f;
	}
}
#endif

result_t can_l2_tx_frame(can_frame *frame)
//result_t L2_CanTxMessage(can_msg_t *msg)
{
#if 0
	//    can_message_id_t  *id;
	uint8_t buffer;
	uint8_t i;
	uint8_t *ptr;

	if (canStatus != Connected) {
		return (CAN_ERROR);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "L2_CanTxMessage(0x%lx)\n\r", msg->header.can_id.id);
#endif
	/*
	 * Find a free buffer
	 */
	for (buffer = 0; buffer < TX_BUFFERS; buffer++) {
		if (!(tx_buffers[buffer]->ctrl & TXREQ)) {
			break;
		}
	}

	if (buffer == TX_BUFFERS) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "No empty TX buffer\n\r");
#endif
		return (CAN_ERROR); //No Empty buffers
	}

	/*
	 * Trasmit buffer with index "buffer" is empty
	 * so fill in the registers with data.
	 */
	if (msg->header.extended_id) {
		//debug("Transmit an extended SYS_CAN message\n\r");

		tx_buffers[buffer]->sidh = (msg->header.can_id.id >> 21) & 0xff;
		tx_buffers[buffer]->sidl = ((msg->header.can_id.id >> 18) & 0x07) << 5;
		tx_buffers[buffer]->sidl |= ((msg->header.can_id.id >> 16) & 0x03);
		tx_buffers[buffer]->sidl |= SIDL_EXIDE;
		tx_buffers[buffer]->eid8 = (msg->header.can_id.id >> 8) & 0xff;
		tx_buffers[buffer]->eid0 = msg->header.can_id.id & 0xff;
	} else {
		//debug("Transmit a standard SYS_CAN message\n\r");
		tx_buffers[buffer]->sidh = (msg->header.can_id.id >> 3) & 0xff;
		tx_buffers[buffer]->sidl = (msg->header.can_id.id & 0x07) << 5;
	}

	tx_buffers[buffer]->dcl = msg->header.data_length & 0x0f;

	if (msg->header.rnr_frame) {
		tx_buffers[buffer]->dcl |= DCL_RNR;
	}

	if (msg->header.data_length > 8) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Copy across the data length %d\n\r", msg->header.data_length);
#endif
	}

	ptr = tx_buffers[buffer]->data;
	for (i = 0; i < msg->header.data_length; i++, ptr++) {
		*ptr = msg->data[i];
	}

	/*
	 * Mark the buffer for transmission
	 */
	tx_buffers[buffer]->ctrl |= TXREQ;

	/*
	 * cancel the timer if running we've received a frame
	 */
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Transmitting L2 Message so restart Idle Timer\n\r");
#endif
	cancel_timer(networkIdleTimer);
	networkIdleTimer = start_timer(networkIdleDuration, pingNetwork, NULL);
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

void L2_SetCanNodeBuadRate(can_baud_rate_t baudRate)
{
	baud_rate_t testRate;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "L2_SetCanNodeBuadRate()\n\r");
#endif

	sys_eeprom_write(NETWORK_BAUD_RATE, (uint8_t) baudRate);

	sys_eeprom_read(NETWORK_BAUD_RATE, (uint8_t *) & testRate);

	if (testRate != baudRate) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Baud Rate NOT Stored!\n\r");
#endif
	} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Baud Rate Stored\n\r");
#endif
	}

//	canStatus = ChangingBaud;
	set_mode(CONFIG_MODE);

	set_bit_rate(baudRate);

	/*
	 * The Baud rate is being changed so going to stay in config mode
	 * for 10 Seconds and let the Network settle down.
	 */
	start_timer(SECONDS_TO_TICKS(10), finalise_baud_rate_change, NULL);
}

static void finaliseBaudRateChange(uint8_t *data)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "finaliseBaudRateChange()\n\r");
#endif
//	canStatus = Connected;
	setMode(NORMAL_MODE);
}

void L2_SetCanNetworkBuadRate(can_baud_rate_t baudRate)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "L2_SetCanNetworkBuadRate()\n\r");
#endif
	setMode(CONFIG_MODE);
	setBitRate(baudRate);
	setMode(NORMAL_MODE);
}

static void set_bit_rate(can_baud_rate_t baudRate)
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
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Invalid Baud Rate Specified\n\r");
#endif
			break;
	}

	BRGCON1 = (((sjw - 1) & 0x03) << 6) | ((brp) & 0x3f);

	BRGCON2 = (((phseg1 - 1) & 0x07) << 3) | ((propseg - 1) & 0x07);
	BRGCON2bits.SAM = 0; //One sampe point
	BRGCON2bits.SEG2PHTS = 1; //Phase Segement 2 programmed

	BRGCON3 = ((phseg2 - 1) & 0x07);
}

/*
 * Function Header
 */
static void setMode(uint8_t mode)
{
	uint8_t value;

	/*
	 * Enter SYS_CAN Configuration mode
	 */
	value = SYS_CANCON;

	value = value & ~MODE_MASK;
	value = value | (mode & MODE_MASK);
	CANCON = value;

	/*
	 * SYS_CAN Mode is a request so we have to wait for confirmation
	 */
	while ((CANSTAT & MODE_MASK) != mode);
}

static void pingNetwork(timer_t timer, union sigval data)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Network Idle Expired so send a ping message and restart\n\r");
#endif
	networkIdleTimer = sw_timer_start(networkIdleDuration, pingNetwork, data, &timer);
	send_ping_message();
}
