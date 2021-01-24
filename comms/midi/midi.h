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
#ifndef _MIDI_H
#define _MIDI_H

#include "libesoup_config.h"

#if defined(SYS_MIDI_TX) || defined(SYS_MIDI_RX)

struct midi_data {
#if defined(SYS_MIDI_TX)
	enum gpio_pin        tx_pin;
#endif
#if defined(SYS_MIDI_RX)
	enum gpio_pin    rx_pin;
	void           (*process_midi_rx)(void);
#endif
};

#define MIDI_STATUS_NOTE_ON  0x90
#define MIDI_STATUS_NOTE_OFF 0x80

/*
 * Stricture to contain a MIDI Message
 */
#define MIDI_MAX_MSG_LEN 4
struct midi_message {
	uint8_t  buffer[MIDI_MAX_MSG_LEN];
	uint8_t  len;
};

extern result_t midi_reserve(struct midi_data *data);

/*
 * API Function to register a handler for a MIDI Status Message
 */
extern result_t midi_register_handler(struct midi_message *, void (*handler)(struct midi_message *));

#endif // defined(SYS_MIDI_TX) || defined(SYS_MIDI_RX)

#endif // _MIDI_H
