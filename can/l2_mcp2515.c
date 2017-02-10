/**
 *
 * \file es_lib/can/l2_mcp2515.c
 *
 * CAN L2 Functionality for MCP2515 Chip
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#include <stdlib.h>
#include "system.h"


#define DEBUG_FILE
//#define SYS_LOG_LEVEL LOG_INFO
#include "es_lib/logger/serial_log.h"
#include "es_lib/can/es_can.h"
#include "es_lib/can/dcncp/dcncp_can.h"
#include "es_lib/can/l2_mcp2515.h"
#include "es_lib/timers/timers.h"

#include "es_lib/utils/spi.h"

#define TAG "MCP2515"

typedef struct
{
	uint8_t sidh;
	uint8_t sidl;
	uint8_t eid8;
	uint8_t eid0;
	uint8_t dcl;
	uint8_t data[8];
} canBuffer_t;

#define CAN_RX_CIR_BUFFER_SIZE 5

canBuffer_t buffer[CAN_RX_CIR_BUFFER_SIZE];
uint8_t        buffer_next_read = 0;
uint8_t        buffer_next_write = 0;
uint8_t        buffer_count = 0;

typedef struct
{
	uint8_t used;
	can_l2_target_t target;
} can_register_t;

static can_register_t registered_handlers[CAN_L2_HANDLER_ARRAY_SIZE];
static can_l2_frame_handler_t unhandled_handler;

static uint8_t connecting_errors = 0;

static BOOL mcp2515_isr = FALSE;

/*
 * Byte to store current input values in TXRTSCTRL
 */
//static uint8_t txrtsctrl = 0x00;

#if defined(CAN_PING_PROTOCOL)
/*
 * Idle duration before sending a Ping Message. Initialised to a random value
 * on powerup.
 */
static uint16_t ping_time;
//static result_t send_ping(void);
#endif

static void     service_device(void);
static void     set_can_mode(uint8_t mode);
static void     set_baudrate(can_baud_rate_t baudRate);
#if defined(CAN_BAUD_AUTO_DETECT)
static void     exp_check_network_connection(timer_t timer_id, union sigval);
#endif // SYS_CAN_BAUD_AUTO_DETECT
static void     exp_finalise_baudrate_change(timer_t timer_id, union sigval data);
static void     exp_resend_baudrate_change(timer_t timer_id, union sigval data);
#if defined(CAN_PING_PROTOCOL)
static void     exp_test_ping(timer_t timer_id, union sigval data);
static void     restart_ping_timer(void);
static es_timer ping_timer;
#endif


static void     enable_rx_interrupts(void);
static void     disable_rx_interrupts(void);

static void     reset(void);
static void     set_reg_mask_value(uint8_t reg, u8 mask, u8 value);
static uint8_t       read_reg(u8 reg);
static void     write_reg(uint8_t reg, u8 value);
static void     read_rx_buffer(uint8_t reg, u8 *buffer);
static uint8_t       find_free_tx_buffer(void);

//static uint8_t CheckErrors(void);
//static void checkSubErrors(void);

static void can_l2_dispatcher_frame_handler(can_frame *message);

#if SYS_LOG_LEVEL < NO_LOGGING
void print_error_counts(void);
#endif

/*
 * Global record of CAN Bus error flags.
 */
static can_baud_rate_t connected_baudrate = no_baud;
#if defined(CAN_BAUD_AUTO_DETECT)
static can_baud_rate_t listen_baudrate = no_baud;
#endif
static uint8_t changing_baud_tx_error;
//static uint8_t g_CanErrors = 0x00;
//static UINT32 g_missedMessageCount = 0;
static u32 rx_msg_count = 0;
//static UINT32 messageSentCount = 0;
//static UINT32 wakeUpCount = 0;
static can_frame rx_can_msg;

static can_status_t status;
static can_baud_rate_t status_baud = no_baud;
#if defined(CAN_BAUD_AUTO_DETECT)
static es_timer listen_timer;
#endif
static void (*status_handler)(uint8_t mask, can_status_t status, can_baud_rate_t baud) = NULL;

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
result_t can_l2_init(can_baud_rate_t arg_baud_rate,
                     void (*arg_status_handler)(uint8_t mask, can_status_t status, can_baud_rate_t baud))
{
	uint8_t        loop = 0x00;
	uint8_t        exit_mode = NORMAL_MODE;
	u32       delay;
#ifndef SYS_CAN_PING_PROTOCOL
	can_frame frame;
#endif // SYS_CAN_PING_PROTOCOL
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "l2_init()\n\r");
#endif

#ifndef SYS_CAN_BAUD_AUTO_DETECT
	if(arg_baud_rate >= no_baud) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Bad Baud rate!!!\n\r");
#endif
		return (ERR_BAD_INPUT_PARAMETER);
	}
#endif // SYS_CAN_BAUD_AUTO_DETECT
	mcp2515_isr = FALSE;

	unhandled_handler = (can_l2_frame_handler_t)NULL;

	/*
         * Intialise the status info. and status_baud
         */
        status.byte = 0x00;
        status_baud = no_baud;

#if defined(CAN_BAUD_AUTO_DETECT)
	TIMER_INIT(listen_timer);
#endif
#if defined(CAN_PING_PROTOCOL)
	TIMER_INIT(ping_timer);
#endif // SYS_CAN_PING_PROTOCOL

        /*
         * Initialise the Handlers table
         */
        for(loop = 0; loop < SYS_CAN_L2_HANDLER_ARRAY_SIZE; loop++) {
		registered_handlers[loop].used = FALSE;
		registered_handlers[loop].target.mask = 0x00;
		registered_handlers[loop].target.filter = 0x00;
		registered_handlers[loop].target.handler = (can_l2_frame_handler_t)NULL;
	}

	status_handler = arg_status_handler;

	CAN_INTERRUPT_PIN_DIRECTION = INPUT_PIN;
	CAN_CS_PIN_DIRECTION = OUTPUT_PIN;
	CAN_DESELECT

	reset();

	for(delay = 0; delay < 0x40000; delay++) {
		asm ("CLRWDT");
		Nop();
	}

	set_can_mode(CONFIG_MODE);

	write_reg(RXB0CTRL, 0x64);
	write_reg(RXB1CTRL, 0x60);
	write_reg(TXRTSCTRL, 0x00);
	write_reg(BFPCTRL, 0x00);

	/**
	 * Have to set the baud rate if one has been passed into the function
	 */
	if(arg_baud_rate < no_baud) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Valid Baud Rate specified - %s\n\r", can_baud_rate_strings[arg_baud_rate]);
#endif
		connected_baudrate = arg_baud_rate;
		set_baudrate(arg_baud_rate);
		exit_mode = NORMAL_MODE;

		status.bit_field.l2_status = L2_Connecting;
		status_baud = arg_baud_rate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);
	} else {
#if defined(CAN_BAUD_AUTO_DETECT)
		/*
		 * Have to search for the Networks baud rate. Start at the bottom
		 */
		rx_msg_count = 0;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Before trying 10K REC - %d, rxCount - %ld\n\r", read_reg(REC), rx_msg_count);
#endif
		listen_baudrate = baud_10K;
		set_baudrate(listen_baudrate);
		exit_mode = LISTEN_MODE;

		connecting_errors = 0;
		status.bit_field.l2_status = L2_Listening;
		status_baud = listen_baudrate;
		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);

		/* Now wait and see if we have errors */
		timer_start(SECONDS_TO_TICKS(CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD), exp_check_network_connection, (union sigval)(void *)NULL, &listen_timer);
#endif // SYS_CAN_BAUD_AUTO_DETECT
	}
	asm ("CLRWDT");

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Set Exit Mode 0x%x\n\r", exit_mode);
#endif
	set_can_mode(exit_mode);

        /*
	 * Turn on Interrupts
	 */
//	write_reg(CANINTE, MERRE | RX1IE | RX0IE);
	write_reg(CANINTE, MERRE | ERRIE | TX2IE | TX1IE | TX0IE | RX1IE | RX0IE);
        INTCON2bits.INT0EP = 1; //Interrupt on Negative edge
        IFS0bits.INT0IF = 0;    // Clear the flag
        IEC0bits.INT0IE = 1;    //Interrupt Enabled

	// Create a random timer for firing the
	// Network Idle Ping message
#if defined(CAN_PING_PROTOCOL)
	ping_time = (uint16_t)((rand() % SECONDS_TO_TICKS(1)) + (SECONDS_TO_TICKS(CAN_PING_PROTOCOL_PERIOD) - MILLI_SECONDS_TO_TICKS(500)));
        restart_ping_timer();
#else
	frame.can_id = CAN_DCNCP_NodePingMessage;
	frame.can_dlc = 0;

	can_l2_tx_frame(&frame);
#endif

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "CAN Layer 2 Initialised\n\r");
#endif
	return(SUCCESS);
}

#if defined(CAN_PING_PROTOCOL)
void exp_test_ping(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        log_d(TAG, "exp_test_ping()\n\r");
#endif
	TIMER_INIT(ping_timer);

	dcncp_send_ping();
        restart_ping_timer();
}
#endif

#if defined(CAN_PING_PROTOCOL)
void restart_ping_timer(void)
{
	if(ping_timer.status == ACTIVE) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
//		log_d(TAG, "Cancel running ping timer\n\r");
#endif
 		if(timer_cancel(&ping_timer) != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Failed to cancel the Ping timer\n\r");
#endif
			return;
		}
	}

	timer_start(ping_time, exp_test_ping, (union sigval)(void *) NULL, &ping_timer);
}
#endif

#if defined(CAN_BAUD_AUTO_DETECT)
void exp_check_network_connection(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
	result_t result;
//	uint8_t rec = read_reg(REC);

	TIMER_INIT(listen_timer);

	if(listen_baudrate < no_baud) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "After trying %s Errors - %d, rxCount - %ld\n\r", can_baud_rate_strings[listen_baudrate], connecting_errors, rx_msg_count);
#endif
	} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "After trying %s Errors - %d, rxCount - %ld\n\r", "NO BAUD RATE", connecting_errors, rx_msg_count);
#endif
	}
	/*
	 * If we heard valid messages with no errors we've found the baud rate.
	 */
	if(rx_msg_count > 0 && connecting_errors == 0) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "*** Network connected ***\n\r");
#endif
		connected_baudrate = listen_baudrate;

		set_can_mode(NORMAL_MODE);

		status.bit_field.l2_status = L2_Connected;
		status_baud = connected_baudrate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);
	} else {
		listen_baudrate++;
		if(listen_baudrate == no_baud)
			listen_baudrate = baud_10K;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "No joy try Baud Rate - %s\n\r", can_baud_rate_strings[listen_baudrate]);
#endif
		set_can_mode(CONFIG_MODE);
		set_baudrate(listen_baudrate);
		set_can_mode(LISTEN_MODE);

                rx_msg_count = 0;
		connecting_errors = 0;
		status.bit_field.l2_status = L2_Listening;
	        status_baud = listen_baudrate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Restart timer\n\r");
#endif
		result = timer_start(SECONDS_TO_TICKS(CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD),
				     exp_check_network_connection,
				     (union sigval)(void *)NULL,
				     &listen_timer);
		if(result != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Failed to start listen timer, result 0x%x\n\r", result);
#endif
		}
	}
}
#endif // SYS_CAN_BAUD_AUTO_DETECT

/*
 * CAN L2 ISR
 */
void _ISR __attribute__((__no_auto_psv__)) _INT0Interrupt(void)
{
    if(mcp2515_isr) {
#if (LOG_LEVEL <= LOG_ERROR)
        log_e(TAG, "Overlapping MCP2515 ISR\n\r");
#endif
    }
    mcp2515_isr = TRUE;
    IFS0bits.INT0IF = 0;
}

static void service_device(void)
{
	uint8_t flags = 0x00;
	uint8_t eflg;
	uint8_t tx_flags = 0x00;
	uint8_t ctrl;
	uint8_t loop;
	uint8_t canstat;
        uint8_t tec = 0x00;

	mcp2515_isr = FALSE;

	/*
	 * Have to work with a snapshot of the Interrupt Flags as they
	 * are volatile.
	 */
	flags = read_reg(CANINTF);

	if(flags == 0x00) {
		canstat = read_reg(CANSTAT);
		write_reg(CANINTF, 0x00);

#if (DEBUG_FILE && (LOG_LEVEL <= LOG_WARNING))
		log_w(TAG, "*** ISR with zero flags! IOCD %x\n\r", canstat & IOCD);
#endif
	} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "service_device() flags 0x%x\n\r", flags);
#endif
        }

	while(flags != 0x00) {
		canstat = read_reg(CANSTAT);
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "service() Flag-%x, IOCD-%x\n\r", flags, (canstat & IOCD));
#endif
		if (flags & ERRIE) {
			eflg = read_reg(EFLG);
#if (LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "*** SYS_CAN ERRIR Flag!!!\n\r");
#endif
#if (LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "*** SYS_CAN EFLG %x\n\r", eflg);
#endif
			if(status.bit_field.l2_status == L2_Listening) {
				connecting_errors++;
                        } else if(status.bit_field.l2_status == L2_Connecting) {
                                tec = read_reg(TEC);
#if (DEBUG_FILE && (LOG_LEVEL <= LOG_WARNING))
                                log_w(TAG, "Tx Error Count = %d\n\r", tec);
#endif
                                if(eflg & TXWAR) {
					set_can_mode(CONFIG_MODE);
					set_can_mode(NORMAL_MODE);
                                }
                        }

			/*
			 * Clear any rx Frames as there's been an error
			 */
			set_reg_mask_value(EFLG, RX1OVR | RX0OVR, 0x00);

			set_reg_mask_value(CANINTF, ERRIE, 0x00);
			flags = flags & ~(RX0IE | RX1IE);
		}

		if (flags & MERRE) {
#if (DEBUG_FILE && (LOG_LEVEL <= LOG_WARNING))
			log_w(TAG, "CAN MERRE Flag\n\r");
#endif
			if(status.bit_field.l2_status == L2_Listening) {
				connecting_errors++;
                        } else if(status.bit_field.l2_status == L2_Connecting) {
                                tec = read_reg(TEC);
#if (DEBUG_FILE && (LOG_LEVEL <= LOG_WARNING))
                                log_w(TAG, "Tx Error Count = %d\n\r", tec);
#endif
                        }
			/*
			 * We've got an error condition so dump all received messages
			 */
			set_reg_mask_value(CANINTF, RX0IE | RX1IE | MERRE, 0x00);
			flags = flags & ~(RX0IE | RX1IE | MERRE);
		}

		ctrl = TXB0CTRL;

		for (loop = 0; loop < 3; loop++) {
			tx_flags = read_reg(ctrl);

			if((tx_flags & TXREQ) && (tx_flags & TXERR)) {
#if (LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Transmit Buffer Failed to send\n\r");
#endif
				set_reg_mask_value(ctrl, TXREQ, 0x00);
				if (status.bit_field.l2_status == L2_ChangingBaud)
					changing_baud_tx_error++;
			}
			ctrl = ctrl + 0x10;
		}

		if (flags & RX0IE) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "RX0IE\n\r");
#endif
#if defined(CAN_PING_PROTOCOL)
			restart_ping_timer();
#endif
			/*
			 * Increment the rx count in case we're listening for Baud
			 * Rate settings.
			 */
			if (status.bit_field.l2_status == L2_Listening) {
				rx_msg_count++;
			} else {
				if (buffer_count < SYS_CAN_RX_CIR_BUFFER_SIZE) {
					read_rx_buffer(RXB0SIDH, (uint8_t *) & buffer[buffer_next_write]);
					buffer_next_write = (buffer_next_write + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
					buffer_count++;
				} else {
#if (LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "Circular Buffer overflow!\n\r");
#endif
				}
			}

			set_reg_mask_value(CANINTF, RX0IE, 0x00);
		}

		if (flags & RX1IE) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "RX1IE\n\r");
#endif
#if defined(CAN_PING_PROTOCOL)
			restart_ping_timer();
#endif

			/*
			 * Incrememnt the rx count incase we're listening for Baud
			 * Rate seettings.
			 */
			if (status.bit_field.l2_status == L2_Listening) {
				rx_msg_count++;
			} else {
				if (buffer_count < SYS_CAN_RX_CIR_BUFFER_SIZE) {
					read_rx_buffer(RXB1SIDH, (uint8_t *) & buffer[buffer_next_write]);
					buffer_next_write = (buffer_next_write + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
					buffer_count++;
				} else {
#if (LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "Circular Buffer overflow!\n\r");
#endif
				}
			}

			set_reg_mask_value(CANINTF, RX1IE, 0x00);
		}

		if (flags & TX2IE) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "TX2IE\n\r");
#endif
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, status_baud);
			}

			set_reg_mask_value(CANINTF, TX2IE, 0x00);
		}

		if (flags & TX1IE) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "TX1IE\n\r");
#endif
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, status_baud);
			}
			set_reg_mask_value(CANINTF, TX1IE, 0x00);
		}

		if (flags & TX0IE) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "TX0IE\n\r");
#endif
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, status_baud);
			}

			set_reg_mask_value(CANINTF, TX0IE, 0x00);
		}
		flags = read_reg(CANINTF);
	}
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "service_device() finished\n\r");
#endif
}

void can_l2_tasks(void)
{
#ifdef TEST
	static UINT16 count = 0;
#endif
	uint8_t loop;

        if(mcp2515_isr)
		service_device();

	while(buffer_count > 0) {
		if (status.bit_field.l2_status == L2_Connecting) {
			status.bit_field.l2_status = L2_Connected;

			if (status_handler)
				status_handler(L2_STATUS_MASK, status, status_baud);
		}

		/*
		 * make sure the rx msg id is cleared
		 */
		rx_can_msg.can_id = 0x00;

		// Check if it's an extended
		if (buffer[buffer_next_read].sidl & SIDL_EXIDE) {
			rx_can_msg.can_id = buffer[buffer_next_read].sidh;
			rx_can_msg.can_id = (rx_can_msg.can_id << 3) | ((buffer[buffer_next_read].sidl >> 5) & 0x07);
			rx_can_msg.can_id = (rx_can_msg.can_id << 2) | (buffer[buffer_next_read].sidl & 0x03);
			rx_can_msg.can_id = (rx_can_msg.can_id << 8) | buffer[buffer_next_read].eid8;
			rx_can_msg.can_id = (rx_can_msg.can_id << 8) | buffer[buffer_next_read].eid0;
			rx_can_msg.can_id |= SYS_CAN_EFF_FLAG;

			if(buffer[buffer_next_read].dcl & DCL_ERTR)
				rx_can_msg.can_id |= SYS_CAN_RTR_FLAG;
		} else {
			rx_can_msg.can_id = buffer[buffer_next_read].sidh;
			rx_can_msg.can_id = (rx_can_msg.can_id << 3) | ((buffer[buffer_next_read].sidl >> 5) & 0x07);

			if(buffer[buffer_next_read].sidl & SIDL_SRTR)
				rx_can_msg.can_id |= SYS_CAN_RTR_FLAG;
		}

		/*
		 * Fill out the Data Length
		 */
		rx_can_msg.can_dlc = buffer[buffer_next_read].dcl & 0x0f;

		for (loop = 0; loop < rx_can_msg.can_dlc; loop++) {
			rx_can_msg.data[loop] = buffer[buffer_next_read].data[loop];
		}

		buffer_next_read = (buffer_next_read + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
		buffer_count--;
		can_l2_dispatcher_frame_handler(&rx_can_msg);
	}

#ifdef TEST
	if(count == 0x00) {

		CAN_SELECT
		byte = read_reg(TXRTSCTRL);
		CAN_DESELECT

		txrtsctrl = byte;
	}
#endif
}

void activate(void)
{
	set_can_mode(NORMAL_MODE);
	enable_rx_interrupts();
}

void deactivate(void)
{
	set_can_mode(CONFIG_MODE);
	disable_rx_interrupts();
}

result_t can_l2_tx_frame(can_frame  *frame)
{
	result_t     result = SUCCESS;
	canBuffer_t  tx_buffer;
	uint8_t          *buff;
	uint8_t           loop = 0x00;
	uint8_t           ctrl;
	uint8_t           can_buffer = 0x00;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "L2 => Id %lx\n\r", frame->can_id);
#endif

#if defined(CAN_PING_PROTOCOL)
        restart_ping_timer();
#endif
	if(connected_baudrate == no_baud) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Can't Transmit network not connected!\n\r");
#endif
		return(ERR_GENERAL_CAN_ERROR);
	}

	if(frame->can_id & SYS_CAN_EFF_FLAG) {
		tx_buffer.sidh = (frame->can_id >> 21) & 0xff;
		tx_buffer.sidl = ((frame->can_id >> 18) & 0x07) << 5;
		tx_buffer.sidl |= ((frame->can_id >> 16) & 0x03);
		tx_buffer.sidl |= SIDL_EXIDE;
		tx_buffer.eid8 = (frame->can_id >> 8) & 0xff;
		tx_buffer.eid0 = frame->can_id & 0xff;
	} else {
		// Fill in the ID
		tx_buffer.sidh = (frame->can_id >> 3) & 0xff;
		tx_buffer.sidl = (frame->can_id & 0x07) << 5;
	}
	// Remote Transmission Request
	tx_buffer.dcl = (frame->can_id & SYS_CAN_RTR_FLAG) ? 0x40 : 0x00;

	// Data Length
	tx_buffer.dcl = tx_buffer.dcl | (frame->can_dlc & 0x0f);

	for(loop = 0; loop < tx_buffer.dcl; loop++) {
		tx_buffer.data[loop] = frame->data[loop];
	}

	/*
	 * Find an empty txBuffer
	 */
	ctrl = find_free_tx_buffer();

	if(ctrl == 0xff) {
		// Shipment of fail has arrived
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "ERROR No free Tx Buffers\n\r");
#endif
		return(ERR_CAN_NO_FREE_BUFFER);
	} else if (ctrl == TXB0CTRL) {
		can_buffer = TXB0SIDH;
	} else if (ctrl == TXB1CTRL) {
		can_buffer = TXB1SIDH;
	} else if (ctrl == TXB2CTRL) {
		can_buffer = TXB2SIDH;
	}

	/*
	 * Load up the transmit buffer
	 */
	CAN_SELECT
	spi_write_byte(CAN_WRITE_REG);
	spi_write_byte(can_buffer);

	/*
	 * First pull out the five header bytes in the message
	 */
	buff = (uint8_t *)&tx_buffer;
	for(loop = 0; loop < 5; loop++, buff++) {
		spi_write_byte(*buff);
	}

	for(loop = 0; loop < frame->can_dlc; loop++, buff++) {
		spi_write_byte(*buff);
	}
	CAN_DESELECT

	/*
	 * Right all set for Transmission but check the current network status
	 * and send in One Shot Mode if we're unsure of the Network.
	 */
	if (status.bit_field.l2_status == L2_Connecting) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Network not good so sending OSM\n\r");
#endif
		set_reg_mask_value(CANCTRL, OSM, OSM);
	} else {
		// Clear One Shot Mode
		set_reg_mask_value(CANCTRL, OSM, 0x00);
	}

	// Set the buffer for Transmission
	set_reg_mask_value(ctrl, TXREQ, TXREQ);
#if 0
	if (status.bit_field.l2_status == L2_Connecting) {
		while (read_reg(ctrl) & 0x08) {
#if (LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Wait for transmission to complete\n\r");
#endif
		}
		status.bit_field.l2_status = L2_Connected;
		if (status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);
        }
#endif
	return(result);
}

#if 0
uint8_t CheckErrors(void)
{
	uint8_t flags = 0x00;

	flags = SYS_CANReadReg(CANINTF);

	if(flags != 0x00) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "CheckErrors() Flag set 0x%x\n\r", flags);
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
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Message Error Flag set! Flasg now 0x%x\n\r", SYS_CANReadReg(CANINTF));
#endif
	}

	if (flags & WAKIE) {
		CANSetRegMaskValue(CANINTF, WAKIE, 0x00);
		wakeUpCount++;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Wake Up Count - now %ld\n\r", wakeUpCount);
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
        // TODO check return value or make fn void
        return(0x00);
}
#endif //0

#if 0
static void checkSubErrors(void)
{
	uint8_t error = 0x00;
	uint8_t difference = 0x00;
	uint8_t mask = 0x00;
	uint8_t loop = 0x00;

	/*
	 * Read the error flag
	 */
	error = SYS_CANReadReg(EFLG);

	if(error != 0x00) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "checkSubErrors() errors - %x\n\r", error);
#endif
	}

	/*
	 * Process the two RX error flags out of the way and clear them
	 */
	if (error & RX1OVR) {
		g_missedMessageCount++;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Missed Message Count - %ld\n\r", g_missedMessageCount);
#endif

		/*
		 * Clear this flag
		 */
		CANSetRegMaskValue(EFLG, RX1OVR, 0x00);
	}

	if (error & RX0OVR) {
		g_missedMessageCount++;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Missed Message Count - %ld\n\r", g_missedMessageCount);
#endif

		/*
		 * Clear this flag
		 */
		CANSetRegMaskValue(EFLG, RX0OVR, 0x00);
	}

	/*
	 * The remaining error Flags are permanent in nature and will
	 * stay active until cleared by the SYS_CAN Module. We only want to
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
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "ERRORS Have changed! Difference is %x\n\r", difference);
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
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "ERROR Bus OFF!\n\r");
#endif
					} else {
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Error Cleared SYS_CAN Bus Active again\n\r");
#endif
					}
				}

				if (mask & TXEP) {
					if (error & TXEP) {
						//ToDo Must send an error to Android
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Transmitter ERROR PASSIVE Error count > 128!\n\r");
#endif
					} else {
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Tx Error count < 128 ;-)\n\r");
#endif
					}
				}

				if (mask & RXEP) {
					if (error & RXEP) {
						//TODO Must send an error to Android
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Receiver ERROR PASSIVE Error count > 128!\n\r");
#endif
					} else {
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Rx Error < 128 :-)\n\r");
#endif
					}
				}

				if (mask & TXWAR) {
					if (error & TXWAR) {
						//TODO Must send an error to Android
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Transmitter WARNING Error count > 96!\n\r");
#endif
					} else {
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Tx Warning cleared Error Count < 96\n\r");
#endif
					}
				}

				if (mask & TXBO) {
					if (error & RXWAR) {
						//TODO Must send an error to Android
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Receiver WARNING Error count > 96!\n\r");
#endif
					} else {
#if (LOG_LEVEL <= LOG_ERROR)
						log_e(TAG, "Rx Warning Cleared Error Count < 96\n\r");
#endif
					}
				}
			}
		}
		g_CanErrors = error;
	} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Errors Have not changed\n\r");
#endif
	}
}
#endif

static void reset(void)
{
	/* Reset the Can Chip */
	CAN_SELECT
	spi_write_byte(CAN_RESET);
	CAN_DESELECT
}

static void set_reg_mask_value(uint8_t reg, u8 mask, u8 value)
{
	uint8_t fail;

        //    do {
        SYS_CAN_SELECT
        spi_write_byte(CAN_BIT_MODIFY);
        spi_write_byte(reg);
        spi_write_byte(mask);
        spi_write_byte(value);
        SYS_CAN_DESELECT

        fail = (read_reg(reg) & mask) != value;
        if(fail) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Bit Modify Failed!\n\r");
#endif
        }
//    } while (fail);
}

static void set_can_mode(uint8_t mode)
{
//	unsigned char result;
#ifdef TEST
	UINT16 delay;
	UINT16 loop = 0;
#endif

#ifdef TEST
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "set_can_mode(0x%x)\n\r", mode);
#endif
#endif

	set_reg_mask_value(CANCTRL, MODE_MASK, mode);
#ifdef TEST
	result = read_reg(CANCTRL);

	while((result & MODE_MASK) != mode) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Before write SYS_CANCTRL 0x%x\n\r", result & MODE_MASK);
#endif
		set_reg_mask_value(CANCTRL, MODE_MASK, mode);
		result = read_reg(CANCTRL);
#ifdef TEST
		for (delay = 0; delay < 0x100; delay++) {
			Nop();
		}
#endif
	}
#endif

}

/**
 * \brief Set the Baud rate of the SYS_CAN Connection to the BUS
 *
 * TQ = 2BRP/Fosc
 *
 * \param baudRate baud rate to set
 */
static void set_baudrate(can_baud_rate_t baudrate)
{
	uint8_t sjw = 0;
	uint8_t brp = 0;
	uint8_t phseg1 = 0;
	uint8_t phseg2 = 0;
	uint8_t propseg = 0;

	switch(baudrate)
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
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Invalid Baud Rate Specified\n\r");
#endif
		break;
	}

	set_can_mode(CONFIG_MODE);

	set_reg_mask_value(CNF1_REG, SJW_MASK, (sjw - 1) << 6 );
	set_reg_mask_value(CNF1_REG, BRP_MASK, (brp) );

	set_reg_mask_value(CNF2_REG, BTLMODE_MASK, 0x80 );
	set_reg_mask_value(CNF2_REG, SAM_MASK, 0x00 );
	set_reg_mask_value(CNF2_REG, PSEG1_MASK, (phseg1 - 1) << 3 );
	set_reg_mask_value(CNF2_REG, PROPSEG_MASK, (propseg - 1) );

	set_reg_mask_value(CNF3_REG, WAKFIL_MASK, 0x00 );
	set_reg_mask_value(CNF3_REG, PSEG2_MASK, (phseg2 - 1) );
}

void get_status(can_status_t *arg_status, can_baud_rate_t *arg_baud)
{
	*arg_status = status;
	*arg_baud = status_baud;
}

can_baud_rate_t can_l2_get_baudrate(void)
{
	return(connected_baudrate);
}

void can_l2_set_node_baudrate(can_baud_rate_t baudrate)
{
	es_timer timer;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "set_can_node_baudrate()\n\r");
#endif
	TIMER_INIT(timer);

	status.bit_field.l2_status = L2_ChangingBaud;
	status_baud = baudrate;

	if (status_handler)
		status_handler(L2_STATUS_MASK, status, status_baud);

	set_can_mode(CONFIG_MODE);

	set_baudrate(baudrate);

	/*
	 * The Baud rate is being changed so going to stay in config mode
	 * for 5 Seconds and let the Network settle down.
	 */
	timer_start(SECONDS_TO_TICKS(5), exp_finalise_baudrate_change, (union sigval)(void *)NULL, &timer);
}

static void exp_finalise_baudrate_change(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "exp_finalise_baudrate_change()\n\r");
#endif

        set_can_mode(NORMAL_MODE);

	status.bit_field.l2_status = L2_Connected;

	if (status_handler)
		status_handler(L2_STATUS_MASK, status, status_baud);
}

/*
 * TODO Change name to initiate
 */
void can_l2_initiate_baudrate_change(can_baud_rate_t rate)
{
	es_timer timer;
	can_frame msg;
	result_t result = SUCCESS;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "initiate_can_baudrate_change()\n\r");
#endif
	TIMER_INIT(timer);

	msg.can_id = 0x705;
	msg.can_dlc = 1;

	msg.data[0] = rate;

	result = can_l2_tx_frame(&msg);

	if (result == SUCCESS) {
		status.bit_field.l2_status = L2_ChangingBaud;
		status_baud = rate;
		changing_baud_tx_error = 0;

		if (status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);

		timer_start(MILLI_SECONDS_TO_TICKS(500), exp_resend_baudrate_change, (union sigval)(void *)NULL, &timer);
	}
}

static void exp_resend_baudrate_change(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
	can_frame msg;
	es_timer timer;

        TIMER_INIT(timer);

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "exp_resend_baudrate_change()\n\r");
#endif

	if(changing_baud_tx_error < 3) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "resending Baud Rate Change Request %d\n\r", changing_baud_tx_error);
#endif
		msg.can_id = 0x705;
		msg.can_dlc = 1;

		msg.data[0] = status_baud;

		if(can_l2_tx_frame(&msg) != ERR_CAN_NO_FREE_BUFFER)
			timer_start(MILLI_SECONDS_TO_TICKS(500), exp_resend_baudrate_change, (union sigval)(void *)NULL, &timer);
		else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "No Free Buffers so change the Baud Rate\n\r");
#endif
			set_reg_mask_value(TXB0CTRL, TXREQ, 0x00);
			set_reg_mask_value(TXB1CTRL, TXREQ, 0x00);
			set_reg_mask_value(TXB2CTRL, TXREQ, 0x00);
                        can_l2_set_node_baudrate(status_baud);
		}
	} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "3 Errors so NOT Resending Baud Rate Change Request\n\r");
#endif
                can_l2_set_node_baudrate(status_baud);
	}
}

static void enable_rx_interrupts(void)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "CANEnableRXInterrupts\n\r");
#endif
	set_reg_mask_value(CANINTE, RX1IE, RX1IE);
	set_reg_mask_value(CANINTE, RX0IE, RX0IE);
}

static void disable_rx_interrupts(void)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "CANDisableRXInterrupts\n\r");
#endif
	set_reg_mask_value(CANINTE, RX1IE, 0x00);
	set_reg_mask_value(CANINTE, RX0IE, 0x00);
}

static uint8_t read_reg(u8 reg)
{
	uint8_t value;
	CAN_SELECT
	spi_write_byte(CAN_READ_REG);
	spi_write_byte(reg);
	value = spi_write_byte(0x00);
	CAN_DESELECT

	return(value);
}

static void write_reg(uint8_t reg, u8 value)
{
	CAN_SELECT
	spi_write_byte(CAN_WRITE_REG);
	spi_write_byte(reg);
	spi_write_byte(value);
	CAN_DESELECT
}

static void read_rx_buffer(uint8_t reg, u8 *buffer)
{
	uint8_t loop = 0x00;
	uint8_t *ptr;
	uint8_t dataLength = 0x00;

	ptr = buffer;
	CAN_SELECT
	spi_write_byte(CAN_READ_REG);
	spi_write_byte(reg);

	/*
	 * First pull out the five header bytes in the message
	 */
	for(loop = 0; loop < 5; loop++, ptr++) {
		*ptr = spi_write_byte(0x00);
	}

	dataLength = buffer[4] & 0x0f;
	if(dataLength > SYS_CAN_DATA_LENGTH) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Invalid Data Length %x & 0x0f = %x\n\r", buffer[4], buffer[4] & 0x0f);
#endif
	} else {
		for (loop = 0; loop < dataLength; loop++, ptr++) {
			*ptr = spi_write_byte(0x00);
		}
        }

	CAN_DESELECT
}

uint8_t find_free_tx_buffer(void)
{
	uint8_t ctrl_value = 0x00;

	ctrl_value = read_reg(TXB0CTRL);
	if(!(ctrl_value & 0x08))
		return(TXB0CTRL);

	ctrl_value = read_reg(TXB1CTRL);
	if(!(ctrl_value & 0x08))
		return(TXB1CTRL);

	ctrl_value = read_reg(TXB2CTRL);
	if(!(ctrl_value & 0x08))
		return(TXB2CTRL);
	else
		return(0xff);
}

#if SYS_LOG_LEVEL < NO_LOGGING
void print_error_counts(void)
{
	uint8_t byte;
	static uint8_t rec = 0x00;
	static uint8_t tec = 0x00;
	static uint8_t eflg = 0x00;

	byte = read_reg(TEC);
	if(byte != tec) {
		tec = byte;
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Tx Error (TEC) Count Change - 0x%x\n\r", tec);
#endif
	}

	byte = read_reg(REC);
	if(byte != rec) {
		rec = byte;
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Rx Error (REC) Count Change - 0x%x\n\r", rec);
#endif
	}

	byte = read_reg(EFLG);
	if(byte != eflg) {
		eflg = byte;
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "EFLG Changed - 0x%x\n\r", eflg);
#endif
	}
}
#endif

#ifdef TEST
void test_can()
{
	uint8_t byte;

	CAN_SELECT
	byte = read_reg(CANCTRL);
	CAN_DESELECT

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Test read of SYS_CANCTRL - 0x%x\n\r", byte);
#endif
}
#endif

static void can_l2_dispatcher_frame_handler(can_frame *frame)
{
	uint8_t loop;
	BOOL found = FALSE;

	printf("L2_CanDispatcherL2MsgHandler 0x%lx [", frame->can_id);
	for(loop = 0; loop < frame->can_dlc; loop++) {
		printf("0x%2x,", frame->data[loop]);
	}
	printf("]\n\r");

	for (loop = 0; loop < SYS_CAN_L2_HANDLER_ARRAY_SIZE; loop++) {

		if(registered_handlers[loop].used) {
			if ((frame->can_id & registered_handlers[loop].target.mask) == (registered_handlers[loop].target.filter & registered_handlers[loop].target.mask)) {
				registered_handlers[loop].target.handler(frame);
				found = TRUE;
			}
		}
	}

	if(!found) {
		/*
		 * No handler found so pass the received message to the Application
		 */
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "No Handler for 0x%lx\n\r", frame->can_id);
#endif
	}
}

result_t can_l2_dispatch_reg_handler(can_l2_target_t *target)
{
	uint8_t loop;
#if (DEBUG_FILE && (LOG_LEVEL <= LOG_INFO))
	log_i(TAG, "sys_l2_can_dispatch_reg_handler mask 0x%lx, filter 0x%lx\n\r",
#endif
		   target->mask,
		   target->filter);
	/*
	 * clean up the target in case the caller has included spurious bits
	 */
	if(target->mask & SYS_CAN_EFF_FLAG) {
		target->mask = target->mask & (CAN_EFF_FLAG | SYS_CAN_EFF_MASK);
	} else {
		target->mask = target->mask & SYS_CAN_SFF_MASK;
	}

	// Find a free slot
	for(loop = 0; loop < SYS_CAN_L2_HANDLER_ARRAY_SIZE; loop++) {
		if(registered_handlers[loop].used == FALSE) {
#if (DEBUG_FILE && (LOG_LEVEL <= LOG_INFO))
			log_i(TAG, "Target stored at target %d\n\r", loop);
#endif
			registered_handlers[loop].used = TRUE;
			registered_handlers[loop].target.mask = target->mask;
			registered_handlers[loop].target.filter = target->filter;
			registered_handlers[loop].target.handler = target->handler;
			target->handler_id = loop;
			return(SUCCESS);
		}
	}
	return(ERR_NO_RESOURCES);
}

result_t can_l2_dispatch_unreg_handler(uint8_t id)
{
	if(id < SYS_CAN_L2_HANDLER_ARRAY_SIZE) {
		if (registered_handlers[id].used) {
			registered_handlers[id].used = FALSE;
			registered_handlers[id].target.mask = 0x00;
			registered_handlers[id].target.filter = 0x00;
			registered_handlers[id].target.handler = (void (*)(can_frame *))NULL;
			return (SUCCESS);
		}
	}
	return(ERR_GENERAL_CAN_ERROR);
}

result_t can_l2_dispatch_set_unhandled_handler(can_l2_frame_handler_t handler)
{
	unhandled_handler = (can_l2_frame_handler_t)handler;
	return(SUCCESS);
}
