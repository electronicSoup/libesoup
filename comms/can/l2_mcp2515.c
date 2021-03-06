/**
 * @file libesoup/comms/can/l2_mcp2515.c
 *
 * @author John Whitmore
 * 
 * @brief CAN L2 Functionality for MCP2515 Chip
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

#include <stdlib.h>
#include <stdio.h>
#include "libesoup_config.h"

#ifdef SYS_CAN_BUS
#ifdef BRD_CAN_BUS_MCP2515

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MCP2515";
//#define SYS_LOG_LEVEL LOG_INFO
#include "libesoup/logger/serial_log.h"
#endif // SYS_SERIAL_LOGGING

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/can/can.h"
#include "libesoup/comms/can/l2_mcp2515.h"

#include "libesoup/comms/spi/spi.h"
#include "libesoup/timers/delay.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifdef SYS_SERIAL_LOGGING
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#ifndef SYS_CAN_L2_HANDLER_ARRAY_SIZE
#error libesoup_config.h file should define SYS_CAN_L2_HANDLER_ARRAY_SIZE (see libesoup/examples/libesoup_config.h)
#endif

#ifndef SYS_CAN_RX_CIR_BUFFER_SIZE
#error libesoup_config.h file should define SYS_CAN_RX_CIR_BUFFER_SIZE (see libesoup/examples/libesoup_config.h)
#endif

typedef struct
{
	uint8_t sidh;
	uint8_t sidl;
	uint8_t eid8;
	uint8_t eid0;
	uint8_t dcl;
	uint8_t data[8];
} canBuffer_t;

canBuffer_t buffer[SYS_CAN_RX_CIR_BUFFER_SIZE];
uint8_t        buffer_next_read = 0;
uint8_t        buffer_next_write = 0;
uint8_t        buffer_count = 0;

static uint8_t connecting_errors = 0;

static boolean mcp2515_isr = FALSE;

/*
 * Byte to store current input values in TXRTSCTRL
 */
//static uint8_t txrtsctrl = 0x00;

static void     service_device(void);
static void     set_can_mode(uint8_t mode);
static void     set_baudrate(can_baud_rate_t baudRate);

static void     enable_rx_interrupts(void);
static void     disable_rx_interrupts(void);

static void     reset(void);
static void     set_reg_mask_value(uint8_t reg, uint8_t mask, uint8_t value);
static uint8_t  read_reg(uint8_t reg);
static void     write_reg(uint8_t reg, uint8_t value);
static void     read_rx_buffer(uint8_t reg, uint8_t *buffer);
static uint8_t  find_free_tx_buffer(void);

//static uint8_t CheckErrors(void);
//static void checkSubErrors(void);


#ifdef SYS_SERIAL_LOGGING
void print_error_counts(void);
#endif

/*
 * Global record of CAN Bus error flags.
 */
static can_baud_rate_t     connected_baudrate = no_baud;
static uint8_t             changing_baud_tx_error;
static uint32_t            rx_msg_count = 0;
static can_frame           rx_can_msg;
static union ty_status     status;
static can_status_t        can_status;
static can_baud_rate_t     status_baud = no_baud;
static status_handler_t    status_handler = NULL;

result_t can_l2_init(can_baud_rate_t arg_baud_rate, status_handler_t handler)
{
	result_t       rc;
	uint8_t        exit_mode = NORMAL_MODE;

	LOG_D("can_l2_init()\n\r");

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_SLAVE)
	ping_init();
#endif // SYS_CAN_PING_PROTOCOL
	
#ifndef SYS_CAN_BAUD_AUTO_DETECT
	if(arg_baud_rate >= no_baud) {
		LOG_E("Bad Baud rate!!!\n\r");
		return (-ERR_BAD_INPUT_PARAMETER);
	}
#endif // SYS_CAN_BAUD_AUTO_DETECT
	mcp2515_isr = FALSE;

	/*
         * Intialise the status info. and status_baud
         */
        status.sstruct.source = can_bus_status;
	status.sstruct.status = 0x00;
	can_status.byte = 0x00;
        status_baud = no_baud;

	status_handler = handler;

	rc = gpio_set(BRD_CAN_INTERRUPT_PIN, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK
	rc = gpio_set(BRD_CAN_CS_PIN, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK
	BRD_CAN_DESELECT

	reset();

	delay(uSeconds, 500);

	set_can_mode(CONFIG_MODE);

	write_reg(RXB0CTRL, 0x64);
	write_reg(RXB1CTRL, 0x60);
	write_reg(TXRTSCTRL, 0x00);
	write_reg(BFPCTRL, 0x00);

	/*
	 * Have to set the baud rate if one has been passed into the function
	 */
	if(arg_baud_rate < no_baud) {
		LOG_D("Valid Baud Rate specified - %s\n\r", can_baud_rate_strings[arg_baud_rate]);
		connected_baudrate = arg_baud_rate;
		set_baudrate(arg_baud_rate);
		exit_mode = NORMAL_MODE;

		can_status.bit_field.l2_status = L2_Connecting;
		status_baud = arg_baud_rate;
		
		status.sstruct.status = can_status.byte;

		if(status_handler)
			status_handler(status);
	} else {
#if defined(SYS_CAN_BAUD_AUTO_DETECT)
		baud_auto_detect_init();
#endif // SYS_CAN_BAUD_AUTO_DETECT
	}
	asm ("CLRWDT");

	LOG_D("Set Exit Mode 0x%x\n\r", exit_mode);
	set_can_mode(exit_mode);

        /*
	 * Turn on Interrupts
	 */
	write_reg(CANINTE, MERRE | ERRIE | TX2IE | TX1IE | TX0IE | RX1IE | RX0IE);
        INTCON2bits.INT0EP = 1; //Interrupt on Negative edge
        IFS0bits.INT0IF = 0;    // Clear the flag
        IEC0bits.INT0IE = 1;    //Interrupt Enabled

	LOG_D("CAN Layer 2 Initialised\n\r");
	return(0);
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

		LOG_W("*** ISR with zero flags! IOCD %x\n\r", canstat & IOCD);
	} else {
		LOG_D("service_device() flags 0x%x\n\r", flags);
        }

	while(flags != 0x00) {
		canstat = read_reg(CANSTAT);
		LOG_D("service() Flag-%x, IOCD-%x\n\r", flags, (canstat & IOCD));
		if (flags & ERRIE) {
			eflg = read_reg(EFLG);
			LOG_E("*** SYS_CAN ERRIR Flag!!!\n\r");
			LOG_E("*** SYS_CAN EFLG %x\n\r", eflg);
			if(can_status.bit_field.l2_status == L2_Listening) {
				connecting_errors++;
                        } else if(can_status.bit_field.l2_status == L2_Connecting) {
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
			if(can_status.bit_field.l2_status == L2_Listening) {
				connecting_errors++;
                        } else if(can_status.bit_field.l2_status == L2_Connecting) {
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
				if (can_status.bit_field.l2_status == L2_ChangingBaud)
					changing_baud_tx_error++;
			}
			ctrl = ctrl + 0x10;
		}

		if (flags & RX0IE) {
			LOG_D("RX0IE\n\r");
#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
			restart_ping_timer();
#endif
			/*
			 * Increment the rx count in case we're listening for Baud
			 * Rate settings.
			 */
			if (can_status.bit_field.l2_status == L2_Listening) {
				rx_msg_count++;
			} else {
				if (buffer_count < SYS_CAN_RX_CIR_BUFFER_SIZE) {
					read_rx_buffer(RXB0SIDH, (uint8_t *) & buffer[buffer_next_write]);
					buffer_next_write = (buffer_next_write + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
					buffer_count++;
				} else {
					LOG_E("Circular Buffer overflow!\n\r");
				}
			}

			set_reg_mask_value(CANINTF, RX0IE, 0x00);
		}

		if (flags & RX1IE) {
			LOG_D("RX1IE\n\r");
#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
			restart_ping_timer();
#endif

			/*
			 * Incrememnt the rx count incase we're listening for Baud
			 * Rate seettings.
			 */
			if (can_status.bit_field.l2_status == L2_Listening) {
				rx_msg_count++;
			} else {
				if (buffer_count < SYS_CAN_RX_CIR_BUFFER_SIZE) {
					read_rx_buffer(RXB1SIDH, (uint8_t *) & buffer[buffer_next_write]);
					buffer_next_write = (buffer_next_write + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
					buffer_count++;
				} else {
					LOG_E("Circular Buffer overflow!\n\r");
				}
			}

			set_reg_mask_value(CANINTF, RX1IE, 0x00);
		}

		if (flags & TX2IE) {
			LOG_D("TX2IE\n\r");
			if (can_status.bit_field.l2_status == L2_Connecting) {
				can_status.bit_field.l2_status = L2_Connected;
				status.sstruct.status = can_status.byte;

				if (status_handler)
					status_handler(status);
			}

			set_reg_mask_value(CANINTF, TX2IE, 0x00);
		}

		if (flags & TX1IE) {
			LOG_D("TX1IE\n\r");
			if (can_status.bit_field.l2_status == L2_Connecting) {
				can_status.bit_field.l2_status = L2_Connected;
				status.sstruct.status = can_status.byte;

				if (status_handler)
					status_handler(status);
			}
			set_reg_mask_value(CANINTF, TX1IE, 0x00);
		}

		if (flags & TX0IE) {
			LOG_D("TX0IE\n\r");
			if (can_status.bit_field.l2_status == L2_Connecting) {
				can_status.bit_field.l2_status = L2_Connected;
				status.sstruct.status = can_status.byte;

				if (status_handler)
					status_handler(status);
			}

			set_reg_mask_value(CANINTF, TX0IE, 0x00);
		}
		flags = read_reg(CANINTF);
	}
	LOG_D("service_device() finished\n\r");
}

void can_l2_tasks(void)
{
#ifdef TEST
	static uint16_t count = 0;
#endif
	uint8_t loop;

        if(mcp2515_isr)
		service_device();

	while(buffer_count > 0) {
		if (can_status.bit_field.l2_status == L2_Connecting) {
			can_status.bit_field.l2_status = L2_Connected;
			status.sstruct.status = can_status.byte;

			if (status_handler)
				status_handler(status);
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

		buffer_next_read = (buffer_next_read + 1) % SYS_CAN_RX_CIR_BUFFER_SIZE;
		buffer_count--;
		frame_dispatch_handle_frame(&rx_can_msg);
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
	result_t     result = 0;
	canBuffer_t  tx_buffer;
	uint8_t          *buff;
	uint8_t           loop = 0x00;
	uint8_t           ctrl;
	uint8_t           can_buffer = 0x00;

	LOG_D("L2 => Id %lx\n\r", frame->can_id);

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
        restart_ping_timer();
#endif
	if(connected_baudrate == no_baud) {
		LOG_E("Can't Transmit network not connected!\n\r");
		return(-ERR_CAN_ERROR);
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
		return(-ERR_CAN_NO_FREE_BUFFER);
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
	BRD_CAN_SELECT
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
	BRD_CAN_DESELECT

	/*
	 * Right all set for Transmission but check the current network status
	 * and send in One Shot Mode if we're unsure of the Network.
	 */
	if (can_status.bit_field.l2_status == L2_Connecting) {
		LOG_D("Network not good so sending OSM\n\r");
		set_reg_mask_value(CANCTRL, OSM, OSM);
	} else {
		// Clear One Shot Mode
		set_reg_mask_value(CANCTRL, OSM, 0x00);
	}

	// Set the buffer for Transmission
	set_reg_mask_value(ctrl, TXREQ, TXREQ);
	return(result);
}

#if 0
uint8_t CheckErrors(void)
{
	uint8_t flags = 0x00;

	flags = SYS_CANReadReg(CANINTF);

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
		LOG_E("Message Error Flag set! Flasg now 0x%x\n\r", SYS_CANReadReg(CANINTF));
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
	uint8_t error = 0x00;
	uint8_t difference = 0x00;
	uint8_t mask = 0x00;
	uint8_t loop = 0x00;

	/*
	 * Read the error flag
	 */
	error = SYS_CANReadReg(EFLG);

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
		LOG_E("ERRORS Have changed! Difference is %x\n\r", difference);

		/*
		 * Loop through the lower 6 bits we're interested in
		 */
		mask = 0x01;

		for(loop = 0; loop < 6; loop++, mask << 1) {
			if(mask & difference) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
				if (mask & TXBO) {
					/*
					 * TXBO Has changed
					 */
					if(error & TXBO) {
						//TODO Must send an error to Android
						LOG_E("ERROR Bus OFF!\n\r");
					} else {
						LOG_E("Error Cleared SYS_CAN Bus Active again\n\r");
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
#endif // #if (SYS_LOG_LEVEL <= LOG_ERROR)
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
	BRD_CAN_SELECT
	spi_write_byte(CAN_RESET);
	BRD_CAN_DESELECT
}

static void set_reg_mask_value(uint8_t reg, uint8_t mask, uint8_t value)
{
	uint8_t fail;

        //    do {
        BRD_CAN_SELECT
        spi_write_byte(CAN_BIT_MODIFY);
        spi_write_byte(reg);
        spi_write_byte(mask);
        spi_write_byte(value);
        BRD_CAN_DESELECT

        fail = (read_reg(reg) & mask) != value;
        if(fail) {
		LOG_E("Bit Modify Failed!\n\r");
        }
//    } while (fail);
}

static void set_can_mode(uint8_t mode)
{
	unsigned char result = 0x00;

#ifdef TEST
	uint16_t delay;
	uint16_t loop = 0;
#endif // TEST

#ifdef TEST
	LOG_D("set_can_mode(0x%x)\n\r", mode);
#endif // TEST

	set_reg_mask_value(CANCTRL, MODE_MASK, mode);
#ifdef TEST
	result = read_reg(CANCTRL);
#endif // TEST
	while((result & MODE_MASK) != mode) {
		LOG_D("Before write SYS_CANCTRL 0x%x\n\r", result & MODE_MASK);
		set_reg_mask_value(CANCTRL, MODE_MASK, mode);
		result = read_reg(CANCTRL);
#ifdef TEST
		for (delay = 0; delay < 0x100; delay++) {
			Nop();
		}
#endif // TEST
	}
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
	*arg_status = can_status;
	*arg_baud = status_baud;
}

can_baud_rate_t can_l2_get_baudrate(void)
{
	return(connected_baudrate);
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

static uint8_t read_reg(uint8_t reg)
{
	uint8_t value;
	BRD_CAN_SELECT
	spi_write_byte(CAN_READ_REG);
	spi_write_byte(reg);
	value = spi_write_byte(0x00);
	BRD_CAN_DESELECT

	return(value);
}

static void write_reg(uint8_t reg, uint8_t value)
{
	BRD_CAN_SELECT
	spi_write_byte(CAN_WRITE_REG);
	spi_write_byte(reg);
	spi_write_byte(value);
	BRD_CAN_DESELECT
}

static void read_rx_buffer(uint8_t reg, uint8_t *buffer)
{
	uint8_t loop = 0x00;
	uint8_t *ptr;
	uint8_t dataLength = 0x00;

	ptr = buffer;
	BRD_CAN_SELECT
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

	BRD_CAN_DESELECT
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

#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL < NO_LOGGING))
void print_error_counts(void)
{
	uint8_t byte;
	static uint8_t rec = 0x00;
	static uint8_t tec = 0x00;
	static uint8_t eflg = 0x00;

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
	uint8_t byte;

	BRD_CAN_SELECT
	byte = read_reg(CANCTRL);
	BRD_CAN_DESELECT

	LOG_D("Test read of SYS_CANCTRL - 0x%x\n\r", byte);
}
#endif

#endif // SYS_CAN_MCP2515
#endif // SYS_CAN_BUS
