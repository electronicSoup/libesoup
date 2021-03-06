/**
 * @file libesoup/comms/modbus/modbus.c
 *
 * @author John Whitmore
 *
 * @brief Functions for using a MODBUS Comms.
 *
 * Copyright 2017-2019 electronicSoup Limited
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
 */
#include "libesoup_config.h"

#ifdef SYS_MODBUS

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MODBUS";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/comms/modbus/modbus_private.h"
#include "libesoup/comms/uart/uart.h"
#include "libesoup/jobs/jobs.h"

struct modbus_channel channels[SYS_MODBUS_NUM_CHANNELS];

/*
 *  Table of CRC values for high?order byte
 */
static uint8_t crc_high_bytes[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;

/* Table of CRC values for low?order byte
*/
static uint8_t crc_low_bytes[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
	0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
	0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
	0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
	0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
	0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
	0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
	0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
	0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
	0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
	0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
	0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,
	0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
	0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
	0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
	0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,
	0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
	0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
	0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
	0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
	0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
	0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;

result_t start_15_timer(struct modbus_channel *channel);
result_t start_35_timer(struct modbus_channel *channel);

uint16_t crc_calculate(uint8_t *data, uint16_t len)
{
	uint8_t *ptr = data;
	uint8_t  crc_high = 0xFF; /* high byte of CRC initialised */
	uint8_t  crc_low  = 0xFF; /* low byte of CRC initialised */
	uint16_t index;           /* will index into CRC lookup table */

	while (len--) {      /* pass through message buffer */
		             /* calculate the CRC */
		asm ("CLRWDT");
		index = crc_high ^ *ptr++;
		crc_high = crc_low ^ crc_high_bytes[index];
		crc_low  = crc_low_bytes[index];
	}
	return (crc_high << 8 | crc_low);
}

uint8_t crc_check(uint8_t *data, uint16_t len)
{
        uint16_t crc;

	crc = crc_calculate(data, len - 2);

	if (  (((crc >> 8) & 0xff) == data[len - 2])
	    &&((crc & 0xff) == data[len - 1]) ) {
		return (TRUE);
	} else {
		return (FALSE);
	}
}

result_t modbus_init(void)
{
	uint8_t i;

	for (i = 0; i < SYS_MODBUS_NUM_CHANNELS; i++) {
		channels[i].app_data     = NULL;
		channels[i].hw_15_timer  = BAD_TIMER_ID;
		channels[i].hw_35_timer  = BAD_TIMER_ID;
		channels[i].resp_timer   = BAD_TIMER_ID;
		channels[i].modbus_index = i;
	}

	return(0);
}

void hw_35_expiry_function(timer_id timer, union sigval data)
{
	struct modbus_channel *chan = (struct modbus_channel *)data.sival_ptr;

	chan->hw_35_timer = BAD_TIMER_ID;

	if (chan->process_timer_35_expiry) {
		chan->process_timer_35_expiry(chan);
	} else {
		LOG_E("T35 in unknown state\n\r");
	}
}

void hw_15_expiry_function(timer_id timer, union sigval data)
{
	struct modbus_channel *chan = (struct modbus_channel *)data.sival_ptr;

	LOG_D("%s\n\r", __func__);

	if (chan->process_timer_15_expiry) {
		chan->process_timer_15_expiry(chan);
//		jobs_add(chan->process_timer_15_expiry, (void *)chan);
	} else {
		LOG_E("T15 in unknown state\n\r");
	}
}

result_t start_15_timer(struct modbus_channel *channel)
{
//	struct period period;
	
	LOG_D("%s\n\r", __func__);
#if 0
	period.units    = uSeconds;
	period.duration = ((1000000 * 17)/channel->uart->baud);
//	channel->hw_15_timer = hw_timer_start(uSeconds, ((1000000 * 17)/channel->uart->baud), FALSE, hw_15_expiry_function, (void *)channel);
#endif
	return(SUCCESS);
}

/*
 * In RTU mode, message frames are separated by a silent interval of at
 * least 3.5 character times.
 * 
 * Character time is 1 start bit, 8 data bits and say 2 stop bits for arguments
 * sake, that's 11 * 1 / baud rate for a character.
 * 
 * For 3.5 char times it's (11 * 3.5) / baudrate
 * 
 * multiply but 1,000,000 for uSeconds 
 * 
 * (1,000,000 * 11 * 3.5) / baudrate
 * Approx 1,000,000 * 39 / baudrate
 * 
 * I'm getting a time out on a baudrate of 9600 from a linux based machine so 
 * up the timer to 
 * 
 * 
 */
result_t start_35_timer(struct modbus_channel *channel)
{
	result_t          rc;
	struct timer_req  request;

	if(channel->hw_35_timer != BAD_TIMER_ID) {
		hw_timer_cancel(&channel->hw_35_timer);
	}

	request.period.units    = uSeconds;
	request.period.duration = ((1000000 * 50)/channel->app_data->uart_data.baud);
	request.type            = single_shot_expiry;
	request.exp_fn          = hw_35_expiry_function;
	request.data.sival_ptr  = channel;

	rc = hw_timer_start(&request);
	RC_CHECK
		
	channel->hw_35_timer = rc;
	return(SUCCESS);
}

static void modbus_process_rx_character(uint8_t uindex, uint8_t ch)
{
	uint8_t i;

	for (i = 0; i < SYS_MODBUS_NUM_CHANNELS; i++) {
		if (channels[i].app_data && channels[i].app_data->uart_data.uindex == uindex) {
			if(channels[i].process_rx_character) {
				channels[i].process_rx_character(&channels[i], ch);
			} else {
				serial_printf("-");
			}
			return;
		}
	}
}

/*
 * Called from UART ISR
 */
void modbus_tx_finished(struct uart_data *uart)
{
	uint8_t           i;

	/*
	 * Find what modbus channel is using this uart
	 */

	for (i = 0; i < SYS_MODBUS_NUM_CHANNELS; i++) {
		if (channels[i].app_data && channels[i].app_data->uart_data.uindex == uart->uindex) {
			break;
		}
	}
	
	if (i >= SYS_MODBUS_NUM_CHANNELS) {
		LOG_E("Unknown uart!\n\r");
		return;
	}

	/*
	 * Call the higher Application tx_finished function
	 */
	if(channels[i].app_tx_finished) {
		channels[i].app_tx_finished(uart);
	}

	/*
	 * Call the Modbus state machine's Tx finished function
	 */
	if(channels[i].modbus_tx_finished) {
		channels[i].modbus_tx_finished(&channels[i]);
	} else {
		LOG_E("Error processing tx_finished\n\r");
	}
}

/*
 * Returns the index of the reserved modbus channel on success
 */
modbus_id modbus_reserve(struct modbus_app_data *app_data)
{
	result_t rc;
	uint8_t  i;
	void (*app_tx_finished)(struct uart_data *);

	if(!app_data || app_data->address > MODBUS_MAX_ADDRESS) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	
	/*
	 * Find a free modbus channel
	 */
	for (i = 0; i < SYS_MODBUS_NUM_CHANNELS; i++) {
		if(!channels[i].app_data) {
			break;
		}
	}
	
	if (i >= SYS_MODBUS_NUM_CHANNELS) {
		return(-ERR_NO_RESOURCES);
	}

	app_tx_finished = app_data->uart_data.tx_finished;

	app_data->uart_data.process_rx_char = modbus_process_rx_character;
	app_data->uart_data.tx_finished     = modbus_tx_finished;

	/*
	 * Reserve a UART for the channel
	 */
	rc = uart_reserve(&app_data->uart_data);
	RC_CHECK

	app_data->channel_id         = i;
	channels[i].app_data         = app_data;
	channels[i].app_tx_finished  = app_tx_finished;
	channels[i].hw_15_timer      = BAD_TIMER_ID;
	channels[i].hw_35_timer      = BAD_TIMER_ID;
	channels[i].resp_timer       = BAD_TIMER_ID;
	channels[i].turnaround_timer = BAD_TIMER_ID;

	/*
	 * Set the starting state.
	 */
#if defined(SYS_MODBUS_MASTER) && !defined(SYS_MODBUS_SLAVE)
	if (app_data->address == 0) {
		set_master_starting_state(&channels[i]);
	} else {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
#endif
	
#if defined(SYS_MODBUS_SLAVE) && !defined(SYS_MODBUS_MASTER)
	if (app_data->address > 0) {
		set_slave_idle_state(&channels[i]);
	} else {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
#endif

#if defined(SYS_MODBUS_MASTER) && defined(SYS_MODBUS_SLAVE)
	if (app_data->address == 0) {
		set_master_starting_state(&channels[i]);
	} else {
		set_slave_idle_state(&channels[i]);
	}
#endif
	return(i);
}

result_t modbus_release(struct modbus_app_data *app_data)
{
	modbus_id index;
	
	if (!app_data) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	index = app_data->channel_id;
	
	if(channels[index].hw_35_timer != BAD_TIMER_ID) {
		hw_timer_cancel(&channels[index].hw_35_timer);
	}
	if(channels[index].hw_15_timer != BAD_TIMER_ID) {
		hw_timer_cancel(&channels[index].hw_15_timer);
	}
	if(channels[index].resp_timer != BAD_TIMER_ID) {
		hw_timer_cancel(&channels[index].resp_timer);
	}
	if(channels[index].turnaround_timer != BAD_TIMER_ID) {
		hw_timer_cancel(&channels[index].turnaround_timer);
	}

        /*
         * put back the tx_finished function
         */
        app_data->uart_data.tx_finished = channels[index].app_tx_finished;

        channels[index].app_data                 = NULL;
	channels[index].app_tx_finished          = NULL;
	channels[index].process_timer_15_expiry  = NULL;
	channels[index].process_timer_35_expiry  = NULL;
	channels[index].transmit                 = NULL;
	channels[index].process_rx_character     = NULL;
	channels[index].process_response_timeout = NULL;

	/*
	 * Release our UART
	 */
	return(uart_release(&app_data->uart_data));
}

#if defined(SYS_MODBUS_MASTER)
result_t  modbus_read_coils_req(modbus_id                chan,
	                        uint8_t                  modbus_address,
                                uint16_t                 coil_address,
                                uint16_t                 number_of_coils,
				modbus_response_function callback)
{
	uint8_t   tx_buffer[6];
	
	if (chan >= SYS_MODBUS_NUM_CHANNELS || !channels[chan].app_data || !callback) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	if (!channels[chan].transmit || channels[chan].state != mb_m_idle) {
		return(-ERR_BUSY);
	}
	if (modbus_address == 0 || modbus_address > 247) {
		return(-ERR_ADDRESS_RANGE);
	}

	/*
	 * Can only send a request from a Modbus Master so check App data for
	 * address of this channel. (0 = Master)
	 */
	if (channels[chan].app_data->address != 0) {
		return(-ERR_NOT_MASTER);
	}
	
	tx_buffer[0] = modbus_address;
	tx_buffer[1] = MODBUS_READ_COILS;
	tx_buffer[2] = (uint8_t)((coil_address >> 8) & 0xff);
	tx_buffer[3] = (uint8_t)(coil_address & 0xff);
	tx_buffer[4] = (uint8_t)((number_of_coils >> 8) & 0xff);
	tx_buffer[5] = (uint8_t)(number_of_coils & 0xff);
	
	return(channels[chan].transmit(&channels[chan], tx_buffer, 6, callback));
}
#endif

#if defined(SYS_MODBUS_MASTER)
extern result_t  modbus_read_holding_regs_req(modbus_id                chan,
                                              uint8_t                  modbus_address,
                                              uint16_t                 reg_address,
                                              uint16_t                 number_of_regs,
				              modbus_response_function callback)
{
	uint8_t   tx_buffer[6];

	if (chan >= SYS_MODBUS_NUM_CHANNELS || !channels[chan].app_data || !callback) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	if (!channels[chan].transmit || channels[chan].state != mb_m_idle) {
		return(-ERR_BUSY);
	}
	if (modbus_address == 0 || modbus_address > 247) {
		return(-ERR_ADDRESS_RANGE);
	}

	/*
	 * Can only send a request from a Modbus Master so check App data for
	 * address of this channel. (0 = Master)
	 */
	if (channels[chan].app_data->address != 0) {
		return(-ERR_NOT_MASTER);
	}

	tx_buffer[0] = modbus_address;
	tx_buffer[1] = MODBUS_READ_HOLDING_REGISTERS;
	tx_buffer[2] = (uint8_t)((reg_address >> 8) & 0xff);
	tx_buffer[3] = (uint8_t)(reg_address & 0xff);
	tx_buffer[4] = (uint8_t)((number_of_regs >> 8) & 0xff);
	tx_buffer[5] = (uint8_t)(number_of_regs & 0xff);

	return(channels[chan].transmit(&channels[chan], tx_buffer, 6, callback));
	
}
#endif // SYS_MODBUS_MASTER

#if defined(SYS_MODBUS_SLAVE)
result_t  modbus_error_resp(modbus_id  chan,
                            uint8_t    modbus_function,
                            uint8_t    exception)
{
	uint8_t  response[3];
	
	if (chan >= SYS_MODBUS_NUM_CHANNELS || !channels[chan].app_data) {
		LOG_E("Bad chan\n\r");
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	if (!channels[chan].transmit || channels[chan].state != mb_s_processing_request) {
		LOG_E("Bad state\n\r");
		return(-ERR_BUSY);
	}

	/*
	 * Can only send a response from a Modbus Slave so check App data for
	 * address of this channel. (0 = Master)
	 */
	if (channels[chan].app_data->address == 0) {
		LOG_E("Not Slave\n\r");
		return(-ERR_NOT_SLAVE);
	}

	response[0] = channels[chan].app_data->address;
	response[1] = modbus_function | 0x80;   // Set MSBit for Error Code
	response[2] = exception;
	
	return(channels[chan].transmit(&channels[chan], response, 3, NULL));
}
#endif

#if defined(SYS_MODBUS_SLAVE)
result_t  modbus_read_coils_resp(modbus_id   chan,
                                 uint8_t    *buffer,
                                 uint8_t     len)
{
	uint8_t   i;
	uint8_t   tx_buffer[254];

	/*
	 * Maximum frame size if 256 bytes which includes an address byte and
	 * two CRC bytes so 253. The response has to include a byte for 
	 * function code and a byte for byte count so len as to be less then
	 * 251
	 */
	if (len > 251 || chan >= SYS_MODBUS_NUM_CHANNELS || !channels[chan].app_data) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	if (!channels[chan].transmit || channels[chan].state != mb_s_processing_request) {
		return(-ERR_BUSY);
	}

	/*
	 * Can only send a response from a Modbus Slave so check App data for
	 * address of this channel. (0 = Master)
	 */
	if (channels[chan].app_data->address == 0) {
		return(-ERR_NOT_SLAVE);
	}
	
	tx_buffer[0] = channels[chan].app_data->address;
	tx_buffer[1] = MODBUS_READ_COILS;
	tx_buffer[2] = len;
	
	for (i = 0; i < len; i++) {
		tx_buffer[3 + i] = buffer[i];
	}
	
	return(channels[chan].transmit(&channels[chan], tx_buffer, (i + 3), NULL));
}
#endif

#if defined(SYS_MODBUS_SLAVE)
result_t  modbus_read_registers_resp(modbus_id   chan,
	                             uint8_t    *buffer,
                                     uint8_t     len)
{
	uint8_t   i;
	uint8_t   tx_buffer[254];

	/*
	 * Maximum frame size if 256 bytes which includes an address byte and
	 * two CRC bytes so 253. The response has to include a byte for 
	 * function code and a byte for byte count so len as to be less then
	 * 251
	 */
	if (len > 251 || chan >= SYS_MODBUS_NUM_CHANNELS || !channels[chan].app_data) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	if (!channels[chan].transmit || channels[chan].state != mb_s_processing_request) {
		return(-ERR_BUSY);
	}

	/*
	 * Can only send a response from a Modbus Slave so check App data for
	 * address of this channel. (0 = Master)
	 */
	if (channels[chan].app_data->address == 0) {
		return(-ERR_NOT_SLAVE);
	}
	
	tx_buffer[0] = channels[chan].app_data->address;
	tx_buffer[1] = MODBUS_READ_HOLDING_REGISTERS;
	tx_buffer[2] = len;
	
	for (i = 0; i < len; i++) {
		tx_buffer[3 + i] = buffer[i];
	}
	
	return(channels[chan].transmit(&channels[chan], tx_buffer, (i + 3), NULL));
}
#endif

result_t modbus_tx_data(struct modbus_channel *chan, uint8_t *data, uint16_t len)
{
	uint16_t      crc;
	uint8_t      *ptr;
	uint16_t      loop;
	uint8_t       buffer[SYS_UART_TX_BUFFER_SIZE];

	ptr = data;

	crc = crc_calculate(data, len);

	for(loop = 0; loop < len; loop++) {
		buffer[loop] = *ptr++;
	}

	buffer[loop++] = (crc >> 8) & 0xff;
	buffer[loop++] = crc & 0xff;

	return(uart_tx_buffer(&chan->app_data->uart_data, buffer, loop));
}

#endif // SYS_MODBUS
