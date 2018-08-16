/**
 * @file libesoup/comms/modbus/modbus.c
 *
 * @author John Whitmore
 *
 * @brief Functions for using a MODBUS Comms.
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

static void resp_timeout_expiry_fn(timer_id timer, union sigval data);

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
	
	LOG_D("%s\n\r", __func__);

	for (i = 0; i < SYS_MODBUS_NUM_CHANNELS; i++) {
		channels[i].uart         = NULL;
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

	LOG_D("%s\n\r", __func__);

	chan->hw_35_timer = BAD_TIMER_ID;

	if (chan->process_timer_35_expiry) {
		chan->process_timer_35_expiry(chan);
//		jobs_add(chan->process_timer_35_expiry, (void *)chan);
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

result_t start_35_timer(struct modbus_channel *channel)
{
	result_t          rc;
	struct timer_req  request;
	
	LOG_D("%s\n\r", __func__);

	if(channel->hw_35_timer != BAD_TIMER_ID) {
		hw_timer_cancel(channel->hw_35_timer);
	}

	request.period.units    = uSeconds;
	request.period.duration = ((1000000 * 39)/channel->uart->baud);
	request.type            = single_shot;
	request.exp_fn          = hw_35_expiry_function;
	request.data.sival_ptr  = (void *)channel;
	
	rc = hw_timer_start(&request);
	RC_CHECK
		
	channel->hw_35_timer = rc;
	return(SUCCESS);
}

static void modbus_process_rx_character(uint8_t channel_id, uint8_t ch)
{
	LOG_D("%s\n\r", __func__);
#if 0
	if(channels[channel_id].process_rx_character) {
		channels[channel_id].process_rx_character(&channels[channel_id], ch);
	}
#endif
}

/*
 * Called from UART ISR
 */
void modbus_tx_finished(void *data)
{
	LOG_D("%s\n\r", __func__);
#if 0
        struct uart_data *uart = (struct uart_data *)data;

	if(!channels[uart->uindex].uart) {
		LOG_E("Error tx_finished channel %d no UART struct\n\r", uart->uindex);
		return;
	}

	/*
	 * Call the Modbus state machine's Tx finished function
	 */
	if(channels[uart->uindex].modbus_tx_finished) {
		channels[uart->uindex].process_tx_finished(&channels[uart->uindex]);
//		jobs_add(channels[uart->uart].modbus_tx_finished, (void *)&channels[uart->uart]);
	} else {
		LOG_E("Error processing tx_finished\n\r");
	}

	/*
	 * Call the higher Application tx_finished function
	 */
	if(channels[uart->uindex].app_tx_finished) {
		channels[uart->uindex].app_tx_finished(channel_id);
//		jobs_add(channels[uart->uart].app_tx_finished, (void *)uart);
	}
#endif
}

/*
 * Returns the index of the reserved modbus channel on success
 */
//result_t modbus_reserve(struct uart_data *uart, void (*idle_callback)(void *), modbus_response_function unsolicited, void *data)
result_t modbus_reserve(struct uart_data *uart, void (*idle_callback)(modbus_id, uint8_t))
{
	result_t rc;
	uint8_t  i;
	void (*app_tx_finished)(void *data);

	LOG_D("%s\n\r", __func__);
	
	if(!uart) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	
	/*
	 * Find a free modbus channel
	 */
	for (i = 0; i < SYS_MODBUS_NUM_CHANNELS; i++) {
		if(!channels[i].uart) {
			break;
		}
	}
	
	if (i >= SYS_MODBUS_NUM_CHANNELS) {
		return(-ERR_NO_RESOURCES);
	}

	app_tx_finished = uart->tx_finished;

	uart->process_rx_char = modbus_process_rx_character;
	uart->tx_finished = modbus_tx_finished;

	/*
	 * Reserve a UART for the channel
	 */
	rc = uart_reserve(uart);
	RC_CHECK

	LOG_D("modbus_reserve took UART %d\n\r", uart->uindex);
//	channels[uart->uindex].process_unsolicited_msg = unsolicited;
//	channels[uart->uindex].idle_callback_data = data;
	channels[i].idle_callback    = idle_callback;
	channels[i].app_tx_finished  = app_tx_finished;
	channels[i].uart             = uart;
	channels[i].hw_15_timer      = BAD_TIMER_ID;
	channels[i].hw_35_timer      = BAD_TIMER_ID;
	channels[i].resp_timer       = BAD_TIMER_ID;
	channels[i].turnaround_timer = BAD_TIMER_ID;

//	channels[uart->uindex].response_callback_data = NULL;

	/*
	 * Set the starting state.
	 */
	set_modbus_starting_state(&channels[i]);

	return(i);
}

result_t modbus_release(struct uart_data *uart)
{
//	result_t rc;

	LOG_D("%s UART %d\n\r", __func__, uart->uindex);
#if 0
	if(channels[uart->uart].hw_35_timer != BAD_TIMER) {
		hw_timer_cancel(channels[uart->uart].hw_35_timer);
	}

        /*
         * put back the tx_finished function
         */
        uart->tx_finished = channels[uart->uart].app_tx_finished;

        channels[uart->uart].uart = NULL;
	channels[uart->uart].process_unsolicited_msg = NULL;
	channels[uart->uart].idle_callback = NULL;
	channels[uart->uart].idle_callback_data = NULL;
	channels[uart->uart].app_tx_finished = NULL;
	channels[uart->uart].process_timer_15_expiry = NULL;
	channels[uart->uart].process_timer_35_expiry = NULL;
	channels[uart->uart].transmit = NULL;
	channels[uart->uart].process_rx_character = NULL;
	channels[uart->uart].process_response_timeout = NULL;
	channels[uart->uart].hw_35_timer = BAD_TIMER;

	/*
	 * Release our UART
	 */
	return(uart_release(uart));
#endif
	return(0);
}

void modbus_tx_data(struct modbus_channel *channel, uint8_t *data, uint16_t len)
{
	LOG_D("%s\n\r", __func__);
#if 0
	uint16_t      crc;
	uint8_t      *ptr;
	uint16_t      loop;
	uint8_t       buffer[SYS_UART_TX_BUFFER_SIZE];
	result_t rc;

	ptr = data;

	crc = crc_calculate(data, len);
	LOG_D("tx_data crc %x\n\r", crc);
	for(loop = 0; loop < len; loop++) {
		buffer[loop] = *ptr++;
	}

	buffer[loop++] = (crc >> 8) & 0xff;
	buffer[loop++] = crc & 0xff;

	rc = uart_tx_buffer(channel->uart, buffer, loop);

	if(rc != 0) {
		LOG_E("Failed to transmit modbus data\n\r");
	}
#endif
}

result_t modbus_attempt_transmission(struct uart_data *uart, uint8_t *data, uint16_t len, modbus_response_function fn, void *callback_data)
{
	LOG_D("%s\n\r", __func__);
#if 0
	if (channels[uart->uart].transmit) {
		channels[uart->uart].transmit(&channels[uart->uart], data, len, fn, callback_data);
		return(0);
	} else {
		LOG_E("Tx Attempted in unknown state\n\r");
		return(-ERR_NOT_READY);
	}
#endif
	return(0);
}

result_t start_response_timer(struct modbus_channel *chan)
{
	result_t          rc;
	struct timer_req  request;

	LOG_D("%s\n\r", __func__);

	if (chan->resp_timer != BAD_TIMER_ID) {
		return(-ERR_GENERAL_ERROR);
	}
	request.period.units    = SYS_MODBUS_RESPONSE_TIMEOUT_UNITS;
	request.period.duration = SYS_MODBUS_RESPONSE_TIMEOUT_DURATION;
	request.type            = single_shot;
	request.exp_fn          = resp_timeout_expiry_fn;
	request.data.sival_int  = chan->modbus_index;

//	if (channel->address == 0) {
//		ticks = SYS_MODBUS_RESPONSE_BROADCAST_TIMEOUT;
//	} else {
//		ticks = SYS_MODBUS_RESPONSE_TIMEOUT;
//	}

	rc = sw_timer_start(&request);
	RC_CHECK

	chan->resp_timer = rc;

	return(SUCCESS);
}

result_t cancel_response_timer(struct modbus_channel *channel)
{
	LOG_D("%s\n\r", __func__);
#if 0
	return(sw_timer_cancel(&(channel->resp_timer)));
#endif
	return(0);
}

static void resp_timeout_expiry_fn(timer_id timer, union sigval data)
{
	LOG_D("%s\n\r", __func__);
	channels[data.sival_int].resp_timer = BAD_TIMER_ID;

	if (channels[data.sival_int].process_response_timeout) {
		channels[data.sival_int].process_response_timeout(&channels[data.sival_int]);
	} else {
		LOG_E("Response Timout in unknown state\n\r");
	}
}

#endif // SYS_MODBUS
