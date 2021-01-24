/**
 * @file libesoup/comms/midi/midi.h
 *
 * @author John Whitmore
 *
 * @brief File containing MIDI protocol implementation
 *
 * Copyright 2021 electronicSoup Limited
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
 *******************************************************************************
 *
 */

/*
 * Set up the configuration words of the processor:
 */

#include "libesoup_config.h"

#include "libesoup/timers/delay.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/comms/uart/uart.h"
#include "libesoup/comms/midi/midi.h"

#define DEBUG_FILE
static const char *TAG = "MIDI";
#include "libesoup/logger/serial_log.h"

#if defined(SYS_MIDI_TX) || defined(SYS_MIDI_RX)
static struct uart_data midi_uart;

#ifdef MIDI_TX
static void expiry(timer_id timer_id, union sigval);
#endif // MIDI_TX

#ifdef SYS_MIDI_RX

/*
 * Structure to hold the MIDI message handlers in the system
 */
struct midi_handler {
	struct midi_message msg;
	void   (*handler)(struct midi_message *);
};

static struct midi_message rx_msg;
static uint8_t rx_await_count = 0;

static struct midi_handler handlers[SYS_MIDI_NUM_HANDLERS];

/*
 * The current MIDI state
 */
static void (*midi_state)(uint8_t uart_id, uint8_t ch) = NULL;
static void awaiting_data(uint8_t uart_id, uint8_t ch);
static void awaiting_status(uint8_t uart_id, uint8_t ch);

static void process_midi(uint8_t uart_id, uint8_t ch)
{
	if (midi_state) {
		midi_state(uart_id, ch);
	}
}
#endif  // MIDI_RX

static void awaiting_status(uint8_t uart_id, uint8_t ch)
{
	result_t rc;
	uint8_t  status;
	uint8_t  chan;
	uint8_t handler;

	if (ch & 0x80) {
		for (handler = 0; handler < SYS_MIDI_NUM_HANDLERS; handler++) {
			if (handlers[handler].handler) {
				if (handlers[handler].msg.buffer[0] == ch) {
					/*
					 * There is a handler defined for this MIDI status so process it.
					 */
					status = ch & 0xf0;
					chan   = ch & 0x0f;

					rx_msg.buffer[0] = ch;
					rx_msg.len       = 1;

					switch(status) {
					case MIDI_STATUS_NOTE_ON:
					case MIDI_STATUS_NOTE_OFF:
						midi_state     = awaiting_data;
						rx_await_count = 3;
						break;
					default:
						LOG_E("Unprogrammed Status\n\r");
					}
					return;
				}
			}
		}
	}
}

static void awaiting_data(uint8_t uart_id, uint8_t ch)
{
	uint8_t handler;
	uint8_t data_byte;
	uint8_t negative;

	if (ch & 0x80) {
		LOG_E("Received a status message\n\r");
	} else {
		rx_msg.buffer[rx_msg.len++] = ch;

		if (rx_msg.len == rx_await_count) {
			/*
			 * Find any matching handlers
			 */
			for (handler = 0; handler < SYS_MIDI_NUM_HANDLERS; handler++) {
				if (handlers[handler].handler) {
					if (handlers[handler].msg.buffer[0] == rx_msg.buffer[0]) {
						data_byte = 1;
						negative  = FALSE;
						while (data_byte < rx_msg.len && !negative) {
							if ((handlers[handler].msg.buffer[data_byte] != rx_msg.buffer[data_byte]) && !(handlers[handler].msg.buffer[data_byte] & 0x80)) {
								negative = TRUE;
							}
							data_byte++;
						}
						if (!negative) {
							handlers[handler].handler(&rx_msg);
						}
					}
				}
			}
			midi_state = awaiting_status;
		}
	}
}

void midi_init(void)
{
	uint8_t n;

	for (n = 0; n < SYS_MIDI_NUM_HANDLERS; n++) {
		handlers[n].handler = NULL;
		handlers[n].msg.len = 0;
	}
}
result_t midi_reserve(struct midi_data *data)
{
	result_t         rc;

	midi_uart.rx_pin = data->rx_pin;
#ifndef SYS_MIDI_TX
	midi_uart.tx_pin = INVALID_GPIO_PIN;
#else
	midi_uart.tx_pin = data->tx_pin;
#endif
	rc = uart_calculate_mode(&midi_uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	if (rc < 0) {
		LOG_E("Failed to calculate\n\r")
		return(rc);
	}
	midi_uart.baud             = 31250;
	midi_uart.tx_finished      = NULL;
#ifdef SYS_MIDI_RX
	midi_uart.process_rx_char  = process_midi;
	midi_state                 = awaiting_status;
#else
	midi_port.process_rx_char  = NULL;
#endif
	rc = uart_reserve(&midi_uart);
	if (rc < 0) {
		LOG_E("Failed to reserver a UART (%d)\n\r", rc);
	}
	return(rc);
}

result_t midi_register_handler(struct midi_message *p_msg, void (*p_handler)(struct midi_message *))
{
	uint8_t n;
	uint8_t x;

	for (n = 0; n < SYS_MIDI_NUM_HANDLERS; n++) {
		if (handlers[n].handler == NULL && handlers[n].msg.len == 0) {
			handlers[n].handler = p_handler;
			handlers[n].msg.len = p_msg->len;

			for (x = 0; x < p_msg->len; x++) {
				handlers[n].msg.buffer[x] = p_msg->buffer[x];
			}
			return(n);
		}
	}
	return(-ERR_NO_RESOURCES);
}
#endif
