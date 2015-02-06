/**
 *
 * \file es_lib/can/l2_mcp2515.c
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
#include <stdlib.h>
#include "es_lib/core.h"
#include "system.h"


//#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"
#include "es_lib/can/es_can.h"
#include "es_lib/can/l2_mcp2515.h"
#include "es_lib/timers/timers.h"

#include "es_lib/utils/spi.h"

#define TAG "MCP2515"

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

canBuffer_t buffer[CAN_RX_CIR_BUFFER_SIZE];
BYTE        buffer_next_read = 0;
BYTE        buffer_next_write = 0;
BYTE        buffer_count = 0;

typedef struct
{
	BYTE used;
	can_l2_target_t target;
} can_register_t;

static can_register_t registered_handlers[CAN_L2_HANDLER_ARRAY_SIZE];

static u8 connecting_errors = 0;

static BOOL mcp2515_isr = FALSE;

/*
 * Byte to store current input values in TXRTSCTRL
 */
//static u8 txrtsctrl = 0x00;

#if defined(CAN_L2_IDLE_PING)
/*
 * Idle duration before sending a Ping Message
 */
static u16 ping_time;
#endif

static void     service_device(void);
#ifdef CAN_L2_IDLE_PING
static result_t send_ping(void);
#endif
static void     set_can_mode(u8 mode);
static void     set_baudrate(can_baud_rate_t baudRate);
static void     exp_check_network_connection(timer_t timer_id, union sigval);
static void     exp_finalise_baudrate_change(timer_t timer_id, union sigval data);
static void     exp_resend_baudrate_change(timer_t timer_id, union sigval data);
#if defined(CAN_L2_IDLE_PING)
static void     exp_test_ping(timer_t timer_id, union sigval data);
static void     restart_ping_timer(void);
#endif


static void     enable_rx_interrupts(void);
static void     disable_rx_interrupts(void);

static void     reset(void);
static void     set_reg_mask_value(u8 reg, u8 mask, u8 value);
static u8       read_reg(u8 reg);
static void     write_reg(u8 reg, u8 value);
static void     read_rx_buffer(u8 reg, u8 *buffer);
static u8       find_free_tx_buffer(void);

//static u8 CheckErrors(void);
//static void checkSubErrors(void);

static void can_l2_dispatcher_frame_handler(can_frame *message);

#if LOG_LEVEL < NO_LOGGING
void print_error_counts(void);
#endif

/*
 * Global record of CAN Bus error flags.
 */
static can_baud_rate_t connected_baudrate = no_baud;
static can_baud_rate_t listen_baudrate = no_baud;
static BYTE changing_baud_tx_error;
//static BYTE g_CanErrors = 0x00;
//static UINT32 g_missedMessageCount = 0;
static UINT32 rx_msg_count = 0;
//static UINT32 messageSentCount = 0;
//static UINT32 wakeUpCount = 0;
static can_frame rx_can_msg;

static can_status_t status;
static can_baud_rate_t status_baud = no_baud;

static es_timer listen_timer;
#if defined(CAN_L2_IDLE_PING)
static es_timer ping_timer;
#endif // CAN_L2_IDLE_PING
static void (*status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud) = NULL;

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
                     void (*arg_status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud))
{
	u8 loop = 0x00;
	u8 exit_mode = NORMAL_MODE;
	u32 delay;
//        result_t result;
	LOG_D("l2_init()\n\r");

	mcp2515_isr = FALSE;

	/*
         * Intialise the status info. and status_baud
         */
        status.byte = 0x00;
        status_baud = no_baud;

	TIMER_INIT(listen_timer);
#if defined(CAN_L2_IDLE_PING)
	TIMER_INIT(ping_timer);
#endif // CAN_L2_IDLE_PING

        /*
         * Initialise the Handlers table
         */
        for(loop = 0; loop < CAN_L2_HANDLER_ARRAY_SIZE; loop++) {
		registered_handlers[loop].used = FALSE;
		registered_handlers[loop].target.mask = 0x00;
		registered_handlers[loop].target.filter = 0x00;
		registered_handlers[loop].target.handler = (can_l2_msg_handler_t)NULL;
	}

	status_handler = arg_status_handler;

	CAN_INTERRUPT_PIN_DIRECTION = INPUT_PIN;
	CAN_CS_PIN_DIRECTION = OUTPUT_PIN;
	CAN_DESELECT();

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
	if(arg_baud_rate <= BAUD_MAX) {
		LOG_D("Valid Baud Rate specified - %s\n\r", can_baud_rate_strings[arg_baud_rate]);
		connected_baudrate = arg_baud_rate;
		set_baudrate(arg_baud_rate);
		exit_mode = NORMAL_MODE;

		status.bit_field.l2_status = L2_Connecting;
		status_baud = arg_baud_rate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);
	} else {
		/*
		 * Have to search for the Networks baud rate. Start at the bottom
		 */
		rx_msg_count = 0;
		LOG_D("Before trying 10K REC - %d, rxCount - %ld\n\r", read_reg(REC), rx_msg_count);
		listen_baudrate = baud_10K;
		set_baudrate(listen_baudrate);
		exit_mode = LISTEN_MODE;

		connecting_errors = 0;
		status.bit_field.l2_status = L2_Listening;
		status_baud = listen_baudrate;
		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);

		/* Now wait and see if we have errors */
		timer_start(CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD, exp_check_network_connection, (union sigval)(void *)NULL, &listen_timer);
	}
	asm ("CLRWDT");

	LOG_D("Set Exit Mode 0x%x\n\r", exit_mode);
	set_can_mode(exit_mode);

        /*
	 * Turn on Interrupts
	 */
//	write_reg(CANINTE, MERRE | RX1IE | RX0IE);
	write_reg(CANINTE, MERRE | ERRIE | TX2IE | TX1IE | TX0IE | RX1IE | RX0IE);
        INTCON2bits.INT0EP = 1; //Interrupt on Negative edge
        IFS0bits.INT0IF = 0;    // Clear the flag
        IEC0bits.INT0IE = 1;    //Interrupt Enabled

	// Create a random timer between 1 and 1.5 seconds for firing the
	// Network Idle Ping message
#if defined(CAN_L2_IDLE_PING)
	ping_time = (u16)((rand() % SECONDS_TO_TICKS(1)) + (CAN_L2_IDLE_PING_PERIOD - MILLI_SECONDS_TO_TICKS(500)));
        LOG_D("Ping time set to %d Ticks, Ping Period %d - %d\n\r", ping_time, CAN_L2_IDLE_PING_PERIOD, MILLI_SECONDS_TO_TICKS(500));
        restart_ping_timer();
#endif

	LOG_D("CAN Layer 2 Initialised\n\r");
	return(SUCCESS);
}

#if defined(CAN_L2_IDLE_PING)
result_t send_ping(void)
{
	can_frame msg;

	msg.can_id = CAN_L2_IDLE_PING_FRAME_ID;
	msg.can_dlc = 0;

	return(l2_tx_frame(&msg));
}
#endif

#if defined(CAN_L2_IDLE_PING)
void exp_test_ping(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
//	result_t result = SUCCESS;

        LOG_D("exp_test_ping()\n\r");
	TIMER_INIT(ping_timer);

	if (send_ping() != SUCCESS) {
		LOG_E("Failed to send the PING Message\n\r");
	}
}
#endif

#if defined(CAN_L2_IDLE_PING)
void restart_ping_timer(void)
{
	if(ping_timer.status == ACTIVE) {
		LOG_D("Cancel running ping timer\n\r");
 		if(timer_cancel(&ping_timer) != SUCCESS) {
			LOG_E("Failed to cancel the Ping timer\n\r");
			return;
		}
	}

	timer_start(ping_time, exp_test_ping, (union sigval)(void *) NULL, &ping_timer);
}
#endif

void exp_check_network_connection(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
	result_t result;
//	u8 rec = read_reg(REC);

	TIMER_INIT(listen_timer);

	if(listen_baudrate <= BAUD_MAX) {
		LOG_D("After trying %s Errors - %d, rxCount - %ld\n\r", can_baud_rate_strings[listen_baudrate], connecting_errors, rx_msg_count);
	} else {
		LOG_D("After trying %s Errors - %d, rxCount - %ld\n\r", "NO BAUD RATE", connecting_errors, rx_msg_count);
	}
	/*
	 * If we heard valid messages with no errors we've found the baud rate.
	 */
	if(rx_msg_count > 0 && connecting_errors == 0) {
		LOG_D("*** Network connected ***\n\r");
		connected_baudrate = listen_baudrate;

		set_can_mode(NORMAL_MODE);

		status.bit_field.l2_status = L2_Connected;
		status_baud = connected_baudrate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);
	} else {
		if(listen_baudrate == BAUD_MAX)
			listen_baudrate = baud_10K;
		else
			listen_baudrate++;
		LOG_D("No joy try Baud Rate - %s\n\r", can_baud_rate_strings[listen_baudrate]);
		set_can_mode(CONFIG_MODE);
		set_baudrate(listen_baudrate);
		set_can_mode(LISTEN_MODE);

                rx_msg_count = 0;
		connecting_errors = 0;
		status.bit_field.l2_status = L2_Listening;
	        status_baud = listen_baudrate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);

		LOG_D("Restart timer\n\r");
		result = timer_start(CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD,
				     exp_check_network_connection,
				     (union sigval)(void *)NULL,
				     &listen_timer);
		if(result != SUCCESS) {
			LOG_E("Failed to start listen timer, result 0x%x\n\r", result);
		}
	}
}

/*
 * CAN L2 ISR
 */
void _ISR __attribute__((__no_auto_psv__)) _INT0Interrupt(void)
{
    if(mcp2515_isr) {
        LOG_E("Overlapping MCP2515 ISR\n\r");
    }
    mcp2515_isr = TRUE;
    IFS0bits.INT0IF = 0;
}

static void service_device(void)
{
	BYTE flags = 0x00;
	BYTE eflg;
	BYTE tx_flags = 0x00;
	BYTE ctrl;
	BYTE loop;
	BYTE canstat;
        BYTE tec = 0x00;

	mcp2515_isr = FALSE;

	/*
	 * Have to work with a snapshot of the Interrupt Flags as they
	 * are volatile.
	 */
	flags = read_reg(CANINTF);

	if(flags == 0x00) {
		canstat = read_reg(CANSTAT);
		write_reg(CANINTF, 0x00);

		LOG_W("*** ISR with zero flags! IOCD %x\n\r", canstat & IOCD);
	} else {
		LOG_D("service_device() flags 0x%x\n\r", flags);
        }

	while(flags != 0x00) {
		canstat = read_reg(CANSTAT);
		LOG_D("service() Flag-%x, IOCD-%x\n\r", flags, (canstat & IOCD));
		if (flags & ERRIE) {
			eflg = read_reg(EFLG);
			LOG_E("*** CAN ERRIR Flag!!!\n\r");
			LOG_E("*** CAN EFLG %x\n\r", eflg);
			if(status.bit_field.l2_status == L2_Listening) {
				connecting_errors++;
                        } else if(status.bit_field.l2_status == L2_Connecting) {
                                tec = read_reg(TEC);
                                LOG_W("Tx Error Count = %d\n\r", tec);
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
			LOG_W("CAN MERRE Flag\n\r");
			if(status.bit_field.l2_status == L2_Listening) {
				connecting_errors++;
                        } else if(status.bit_field.l2_status == L2_Connecting) {
                                tec = read_reg(TEC);
                                LOG_W("Tx Error Count = %d\n\r", tec);
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
				LOG_E("Transmit Buffer Failed to send\n\r");
				set_reg_mask_value(ctrl, TXREQ, 0x00);
				if (status.bit_field.l2_status == L2_ChangingBaud)
					changing_baud_tx_error++;
			}
			ctrl = ctrl + 0x10;
		}

		if (flags & RX0IE) {
			LOG_D("RX0IE\n\r");
#if defined(CAN_L2_IDLE_PING)
			restart_ping_timer();
#endif
			/*
			 * Increment the rx count in case we're listening for Baud
			 * Rate settings.
			 */
			if (status.bit_field.l2_status == L2_Listening) {
				rx_msg_count++;
			} else {
				if (buffer_count < CAN_RX_CIR_BUFFER_SIZE) {
					read_rx_buffer(RXB0SIDH, (BYTE *) & buffer[buffer_next_write]);
					buffer_next_write = (buffer_next_write + 1) % CAN_RX_CIR_BUFFER_SIZE;
					buffer_count++;
				} else {
					LOG_E("Circular Buffer overflow!\n\r");
				}
			}

			set_reg_mask_value(CANINTF, RX0IE, 0x00);
		}

		if (flags & RX1IE) {
			LOG_D("RX1IE\n\r");
#if defined(CAN_L2_IDLE_PING)
			restart_ping_timer();
#endif

			/*
			 * Incrememnt the rx count incase we're listening for Baud
			 * Rate seettings.
			 */
			if (status.bit_field.l2_status == L2_Listening) {
				rx_msg_count++;
			} else {
				if (buffer_count < CAN_RX_CIR_BUFFER_SIZE) {
					read_rx_buffer(RXB1SIDH, (BYTE *) & buffer[buffer_next_write]);
					buffer_next_write = (buffer_next_write + 1) % CAN_RX_CIR_BUFFER_SIZE;
					buffer_count++;
				} else {
					LOG_E("Circular Buffer overflow!\n\r");
				}
			}

			set_reg_mask_value(CANINTF, RX1IE, 0x00);
		}

		if (flags & TX2IE) {
			LOG_D("TX2IE\n\r");
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, status_baud);
			}

			set_reg_mask_value(CANINTF, TX2IE, 0x00);
		}

		if (flags & TX1IE) {
			LOG_D("TX1IE\n\r");
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, status_baud);
			}
			set_reg_mask_value(CANINTF, TX1IE, 0x00);
		}

		if (flags & TX0IE) {
			LOG_D("TX0IE\n\r");
			if (status.bit_field.l2_status == L2_Connecting) {
				status.bit_field.l2_status = L2_Connected;

				if (status_handler)
					status_handler(L2_STATUS_MASK, status, status_baud);
			}

			set_reg_mask_value(CANINTF, TX0IE, 0x00);
		}
		flags = read_reg(CANINTF);
	}
}

void can_l2_tasks(void)
{
#ifdef TEST
	static UINT16 count = 0;
#endif
	BYTE loop;

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
			rx_can_msg.can_id |= CAN_EFF_FLAG;

			if(buffer[buffer_next_read].dcl & DCL_ERTR)
				rx_can_msg.can_id |= CAN_RTR_FLAG;
		} else {
			rx_can_msg.can_id = buffer[buffer_next_read].sidh;
			rx_can_msg.can_id = (rx_can_msg.can_id << 3) | ((buffer[buffer_next_read].sidl >> 5) & 0x07);

			if(buffer[buffer_next_read].sidl & SIDL_SRTR)
				rx_can_msg.can_id |= CAN_RTR_FLAG;
		}

		/*
		 * Fill out the Data Length
		 */
		rx_can_msg.can_dlc = buffer[buffer_next_read].dcl & 0x0f;

		for (loop = 0; loop < rx_can_msg.can_dlc; loop++) {
			rx_can_msg.data[loop] = buffer[buffer_next_read].data[loop];
		}

		buffer_next_read = (buffer_next_read + 1) % CAN_RX_CIR_BUFFER_SIZE;
		buffer_count--;
		can_l2_dispatcher_frame_handler(&rx_can_msg);
	}

#ifdef TEST
	if(count == 0x00) {

		CAN_SELECT();
		byte = read_reg(TXRTSCTRL);
		CAN_DESELECT();

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
	u8          *buff;
	u8           loop = 0x00;
	u8           ctrl;
	u8           can_buffer;

	LOG_D("L2 => Id %lx\n\r", frame->can_id);

#if defined(CAN_L2_IDLE_PING)
        restart_ping_timer();
#endif
	if(connected_baudrate == no_baud) {
		LOG_E("Can't Transmit network not connected!\n\r");
		return(ERR_GENERAL_CAN_ERROR);
	}

	if(frame->can_id & CAN_EFF_FLAG) {
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
	tx_buffer.dcl = (frame->can_id & CAN_RTR_FLAG) ? 0x40 : 0x00;

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
		LOG_E("ERROR No free Tx Buffers\n\r");
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
	CAN_SELECT();
	spi_write_byte(CAN_WRITE_REG);
	spi_write_byte(can_buffer);

	/*
	 * First pull out the five header bytes in the message
	 */
	buff = (BYTE *)&tx_buffer;
	for(loop = 0; loop < 5; loop++, buff++) {
		spi_write_byte(*buff);
	}

	for(loop = 0; loop < frame->can_dlc; loop++, buff++) {
		spi_write_byte(*buff);
	}
	CAN_DESELECT();

	/*
	 * Right all set for Transmission but check the current network status
	 * and send in One Shot Mode if we're unsure of the Network.
	 */
	if (status.bit_field.l2_status == L2_Connecting) {
		LOG_D("Network not good so sending OSM\n\r");
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
			LOG_E("Wait for transmission to complete\n\r");
		}
		status.bit_field.l2_status = L2_Connected;
		if (status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);
        }
#endif
	return(result);
}

#if 0
BYTE CheckErrors(void)
{
	BYTE flags = 0x00;

	flags = CANReadReg(CANINTF);

	if(flags != 0x00) {
		LOG_D("CheckErrors() Flag set 0x%x\n\r", flags);
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
		LOG_E("Message Error Flag set! Flasg now 0x%x\n\r", CANReadReg(CANINTF));
	}

	if (flags & WAKIE) {
		CANSetRegMaskValue(CANINTF, WAKIE, 0x00);
		wakeUpCount++;
		LOG_D("Wake Up Count - now %ld\n\r", wakeUpCount);
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
	BYTE error = 0x00;
	BYTE difference = 0x00;
	BYTE mask = 0x00;
	BYTE loop = 0x00;

	/*
	 * Read the error flag
	 */
	error = CANReadReg(EFLG);

	if(error != 0x00) {
		LOG_E("checkSubErrors() errors - %x\n\r", error);
	}

	/*
	 * Process the two RX error flags out of the way and clear them
	 */
	if (error & RX1OVR) {
		g_missedMessageCount++;
		LOG_D("Missed Message Count - %ld\n\r", g_missedMessageCount);

		/*
		 * Clear this flag
		 */
		CANSetRegMaskValue(EFLG, RX1OVR, 0x00);
	}

	if (error & RX0OVR) {
		g_missedMessageCount++;
		LOG_D("Missed Message Count - %ld\n\r", g_missedMessageCount);

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
		LOG_E("ERRORS Have changed! Difference is %x\n\r", difference);

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
						LOG_E("ERROR Bus OFF!\n\r");
					} else {
						LOG_E("Error Cleared CAN Bus Active again\n\r");
					}
				}

				if (mask & TXEP) {
					if (error & TXEP) {
						//ToDo Must send an error to Android
						LOG_E("Transmitter ERROR PASSIVE Error count > 128!\n\r");
					} else {
						LOG_E("Tx Error count < 128 ;-)\n\r");
					}
				}

				if (mask & RXEP) {
					if (error & RXEP) {
						//TODO Must send an error to Android
						LOG_E("Receiver ERROR PASSIVE Error count > 128!\n\r");
					} else {
						LOG_E("Rx Error < 128 :-)\n\r");
					}
				}

				if (mask & TXWAR) {
					if (error & TXWAR) {
						//TODO Must send an error to Android
						LOG_E("Transmitter WARNING Error count > 96!\n\r");
					} else {
						LOG_E("Tx Warning cleared Error Count < 96\n\r");
					}
				}

				if (mask & TXBO) {
					if (error & RXWAR) {
						//TODO Must send an error to Android
						LOG_E("Receiver WARNING Error count > 96!\n\r");
					} else {
						LOG_E("Rx Warning Cleared Error Count < 96\n\r");
					}
				}
			}
		}
		g_CanErrors = error;
	} else {
		LOG_D("Errors Have not changed\n\r");
	}
}
#endif

static void reset(void)
{
	/* Reset the Can Chip */
	CAN_SELECT();
	spi_write_byte(CAN_RESET);
	CAN_DESELECT();
}

static void set_reg_mask_value(BYTE reg, BYTE mask, BYTE value)
{
	BYTE fail;

        //    do {
        CAN_SELECT();
        spi_write_byte(CAN_BIT_MODIFY);
        spi_write_byte(reg);
        spi_write_byte(mask);
        spi_write_byte(value);
        CAN_DESELECT();
       
        fail = (read_reg(reg) & mask) != value;
        if(fail) {
		LOG_E("Bit Modify Failed!\n\r");
        }
//    } while (fail);
}

static void set_can_mode(BYTE mode)
{
//	unsigned char result;
#ifdef TEST
	UINT16 delay;
	UINT16 loop = 0;
#endif

#ifdef TEST
	LOG_D("set_can_mode(0x%x)\n\r", mode);
#endif

	set_reg_mask_value(CANCTRL, MODE_MASK, mode);
#ifdef TEST
	result = read_reg(CANCTRL);

	while((result & MODE_MASK) != mode) {
		LOG_D("Before write CANCTRL 0x%x\n\r", result & MODE_MASK);
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
 * \brief Set the Baud rate of the CAN Connection to the BUS
 *
 * TQ = 2BRP/Fosc
 *
 * \param baudRate baud rate to set
 */
static void set_baudrate(can_baud_rate_t baudrate)
{
	BYTE sjw = 0;
	BYTE brp = 0;
	BYTE phseg1 = 0;
	BYTE phseg2 = 0;
	BYTE propseg = 0;

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
		LOG_E("Invalid Baud Rate Specified\n\r");
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

can_baud_rate_t get_baudrate(void)
{
	return(connected_baudrate);
}

void can_l2_set_node_baudrate(can_baud_rate_t baudrate)
{
	es_timer timer;

	LOG_D("set_can_node_baudrate()\n\r");
	TIMER_INIT(timer);

	status.bit_field.l2_status = L2_ChangingBaud;
	status_baud = baudrate;

	if (status_handler)
		status_handler(L2_STATUS_MASK, status, status_baud);

	set_can_mode(CONFIG_MODE);

	set_baudrate(baudrate);

	/*
	 * The Baud rate is being changed so going to stay in config mode
	 * for 10 Seconds and let the Network settle down.
	 */
	timer_start(SECONDS_TO_TICKS(5), exp_finalise_baudrate_change, (union sigval)(void *)NULL, &timer);
}

static void exp_finalise_baudrate_change(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
	LOG_D("exp_finalise_baudrate_change()\n\r");

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

	LOG_D("initiate_can_baudrate_change()\n\r");
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

	LOG_D("exp_resend_baudrate_change()\n\r");

	if(changing_baud_tx_error < 3) {
		LOG_D("resending Baud Rate Change Request %d\n\r", changing_baud_tx_error);
		msg.can_id = 0x705;
		msg.can_dlc = 1;

		msg.data[0] = status_baud;

		if(can_l2_tx_frame(&msg) != ERR_CAN_NO_FREE_BUFFER)
			timer_start(MILLI_SECONDS_TO_TICKS(500), exp_resend_baudrate_change, (union sigval)(void *)NULL, &timer);
		else {
			LOG_D("No Free Buffers so change the Baud Rate\n\r");
			set_reg_mask_value(TXB0CTRL, TXREQ, 0x00);
			set_reg_mask_value(TXB1CTRL, TXREQ, 0x00);
			set_reg_mask_value(TXB2CTRL, TXREQ, 0x00);
                        can_l2_set_node_baudrate(status_baud);
		}
	} else {
		LOG_D("3 Errors so NOT Resending Baud Rate Change Request\n\r");
                can_l2_set_node_baudrate(status_baud);
	}
}

static void enable_rx_interrupts(void)
{
	LOG_D("CANEnableRXInterrupts\n\r");
	set_reg_mask_value(CANINTE, RX1IE, RX1IE);
	set_reg_mask_value(CANINTE, RX0IE, RX0IE);
}

static void disable_rx_interrupts(void)
{
	LOG_D("CANDisableRXInterrupts\n\r");
	set_reg_mask_value(CANINTE, RX1IE, 0x00);
	set_reg_mask_value(CANINTE, RX0IE, 0x00);
}

static u8 read_reg(BYTE reg)
{
	u8 value;
	CAN_SELECT();
	spi_write_byte(CAN_READ_REG);
	spi_write_byte(reg);
	value = spi_write_byte(0x00);
	CAN_DESELECT();

	return(value);
}

static void write_reg(BYTE reg, BYTE value)
{
	CAN_SELECT();
	spi_write_byte(CAN_WRITE_REG);
	spi_write_byte(reg);
	spi_write_byte(value);
	CAN_DESELECT();
}

static void read_rx_buffer(BYTE reg, BYTE *buffer)
{
	BYTE loop = 0x00;
	BYTE *ptr;
	BYTE dataLength = 0x00;

	ptr = buffer;
	CAN_SELECT();
	spi_write_byte(CAN_READ_REG);
	spi_write_byte(reg);

	/*
	 * First pull out the five header bytes in the message
	 */
	for(loop = 0; loop < 5; loop++, ptr++) {
		*ptr = spi_write_byte(0x00);
	}

	dataLength = buffer[4] & 0x0f;
	if(dataLength > CAN_DATA_LENGTH) {
		LOG_E("Invalid Data Length %x & 0x0f = %x\n\r", buffer[4], buffer[4] & 0x0f);
	} else {
		for (loop = 0; loop < dataLength; loop++, ptr++) {
			*ptr = spi_write_byte(0x00);
		}
        }

	CAN_DESELECT();
}

BYTE find_free_tx_buffer(void)
{
	BYTE ctrl_value = 0x00;

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

#if LOG_LEVEL < NO_LOGGING
void print_error_counts(void)
{
	BYTE byte;
	static BYTE rec = 0x00;
	static BYTE tec = 0x00;
	static BYTE eflg = 0x00;

	byte = read_reg(TEC);
	if(byte != tec) {
		tec = byte;
		LOG_E("Tx Error (TEC) Count Change - 0x%x\n\r", tec);
	}

	byte = read_reg(REC);
	if(byte != rec) {
		rec = byte;
		LOG_E("Rx Error (REC) Count Change - 0x%x\n\r", rec);
	}

	byte = read_reg(EFLG);
	if(byte != eflg) {
		eflg = byte;
		LOG_E("EFLG Changed - 0x%x\n\r", eflg);
	}
}
#endif

#ifdef TEST
void test_can()
{
	BYTE byte;

	CAN_SELECT();
	byte = read_reg(CANCTRL);
	CAN_DESELECT();

	LOG_D("Test read of CANCTRL - 0x%x\n\r", byte);
}
#endif

static void can_l2_dispatcher_frame_handler(can_frame *message)
{
	BYTE loop;
	BOOL found = FALSE;

	LOG_D("L2_CanDispatcherL2MsgHandler 0x%lx\n\r", message->can_id);

	for (loop = 0; loop < CAN_L2_HANDLER_ARRAY_SIZE; loop++) {

		if(registered_handlers[loop].used) {
			if ((message->can_id & registered_handlers[loop].target.mask) == (registered_handlers[loop].target.filter & registered_handlers[loop].target.mask)) {
				registered_handlers[loop].target.handler(message);
				found = TRUE;
			}
		}
	}

	if(!found) {
		/*
		 * No handler found so pass the received message to the Application
		 */
		LOG_D("No Handler for 0x%lx\n\r", message->can_id);
	}
}

result_t can_l2_reg_handler(can_l2_target_t *target)
{
	BYTE loop;
	LOG_D("sys_l2_can_dispatch_reg_handler mask 0x%lx, filter 0x%lx\n\r",
		   target->mask,
		   target->filter);
	/*
	 * clean up the target in case the caller has included spurious bits
	 */
	if(target->mask & CAN_EFF_FLAG) {
		target->mask = target->mask & (CAN_EFF_FLAG | CAN_EFF_MASK);
	} else {
		target->mask = target->mask & CAN_SFF_MASK;
	}

	// Find a free slot
	for(loop = 0; loop < CAN_L2_HANDLER_ARRAY_SIZE; loop++) {
		if(registered_handlers[loop].used == FALSE) {
			LOG_D("Target stored at target %d\n\r", loop);
			registered_handlers[loop].used = TRUE;
			registered_handlers[loop].target.mask = target->mask;
			registered_handlers[loop].target.filter = target->filter;
			registered_handlers[loop].target.handler = target->handler;
			return(SUCCESS);
		}
	}
	return(ERR_NO_RESOURCES);
}

result_t can_l2_dispatch_unreg_handler(BYTE id)
{
	if(id < CAN_L2_HANDLER_ARRAY_SIZE) {
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
