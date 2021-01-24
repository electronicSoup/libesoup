/**
 * @file libesoup/devices/looper/looper.c
 *
 * @author John Whitmore
 *
 * @brief File containing API implementation for controlling guitar looper pedals.
 *        Tested with the TC Electronic Looper Pedals.
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
#include "libesoup_config.h"
#ifdef SYS_LOOPER

#ifndef SYS_LOOPER_MAX
#error "configuration must define SYS_LOOPER_MAX, the number of possible pedals."
#endif

#define TAG "LOOPER"
#define DEBUG_FILE
#include "libesoup/logger/serial_log.h"

#include "libesoup/devices/looper/looper.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/timers/delay.h"
#include "libesoup/comms/midi/midi.h"
#include "libesoup/timers/sw_timers.h"

struct looper_data {
	enum gpio_pin     gpio;
	uint8_t           midi_channel;
	uint8_t           midi_pitch;
	void            (*state)(struct looper_data *, struct midi_message *);

};

static struct looper_data loopers[SYS_LOOPER_MAX];

static void uninitialized(struct looper_data *looper, struct midi_message *msg);
static void idle(struct looper_data *looper, struct midi_message *msg);
static void recording(struct looper_data *looper, struct midi_message *msg);
static void stopped(struct looper_data *looper, struct midi_message *msg);
static void playing(struct looper_data *looper, struct midi_message *msg);

#define LOOPER_PULSE  20
static struct timer_req pulse_timer_req;

static void pulse_off(timer_id timer, union sigval data);
static void first_pulse_off(timer_id timer, union sigval data);
static void second_pulse_on(timer_id timer, union sigval data);

static result_t looper_pulse(enum gpio_pin gpio);
static result_t looper_double_pulse(enum gpio_pin gpio);

/*
 * MIDI callback funcitons
 */
void note_on_handler(struct midi_message *msg) {
	uint8_t n;

	for (n = 0; n < SYS_LOOPER_MAX; n++) {
		if (loopers[n].midi_pitch == msg->buffer[1]) {
			loopers[n].state(&loopers[n], msg);
			return;
		}
	}
}

void note_off_handler(struct midi_message *msg) {
	uint8_t n;

	for (n = 0; n < SYS_LOOPER_MAX; n++) {
		if (loopers[n].midi_pitch == msg->buffer[1]) {
			loopers[n].state(&loopers[n], msg);
			return;
		}
	}
}


result_t loppers_init(void)
{
	uint8_t  n;
	LOG_D("looper_init()\n\r");
	for (n = 0; n < SYS_LOOPER_MAX; n++) {
		loopers[n].gpio       = INVALID_GPIO_PIN;
		loopers[n].midi_pitch = 0x00;
		loopers[n].state      = uninitialized;
	}

	pulse_timer_req.period.units    = mSeconds;
	pulse_timer_req.period.duration = LOOPER_PULSE;
	return(SUCCESS);
}

result_t looper_init(enum gpio_pin gpio, uint8_t midi_channel, uint8_t midi_pitch)
{
	result_t            rc;
	uint8_t             looper;
	struct midi_message midi_msg;

	for (looper = 0; looper < SYS_LOOPER_MAX; looper++) {
		if (loopers[looper].gpio == INVALID_GPIO_PIN) {
			loopers[looper].gpio         = gpio;
			loopers[looper].midi_channel = midi_channel;
			loopers[looper].midi_pitch   = midi_pitch;
			loopers[looper].state        = idle;

			midi_msg.buffer[0] = MIDI_STATUS_NOTE_ON | midi_channel;
			midi_msg.buffer[1] = midi_pitch;
			midi_msg.buffer[2] = 0x80;
			midi_msg.len       = 3;

			rc = midi_register_handler(&midi_msg, note_on_handler);
			if (rc < 0) {
				LOG_E("Failed to register MIDI Handler\n\r");
				return(rc);
			}

			midi_msg.buffer[0] = MIDI_STATUS_NOTE_OFF | midi_channel;
			rc = midi_register_handler(&midi_msg, note_off_handler);
			if (rc < 0) {
				LOG_E("Failed to register MIDI Handler\n\r");
				return(rc);
			}
			return(looper);
		}
	}
	return(-ERR_NO_RESOURCES);
}

static void uninitialized(struct looper_data *looper, struct midi_message *msg)
{
	LOG_D("uninitialized()\n\r");
	return;
}

static void idle(struct looper_data *looper, struct midi_message *msg)
{
	result_t rc;

	LOG_D("idle()\n\r");

	if ((msg->buffer[0] & 0xf0) == MIDI_STATUS_NOTE_ON) {
		/*
		 * Set recording with single pulse
		 */
		rc = looper_pulse(looper->gpio);
		if (rc < 0) {
			LOG_E("Failed to pulse gpio\n\r");
			return;
		}

		looper->state = recording;
	} else {
		LOG_W("Wasn't expected\n\r");
	}
	return;
}

static void recording(struct looper_data *looper, struct midi_message *msg)
{
	result_t rc;

	LOG_D("recording()\n\r");
	if ((msg->buffer[0] &0xf0) == MIDI_STATUS_NOTE_OFF) {
		/*
		 * Set stopped with double pulse
		 */
		rc = looper_double_pulse(looper->gpio);
		if (rc < 0) {
			LOG_E("Failed to double pulse gpio\n\r");
			return;
		}

		looper->state = stopped;
	} else {
		LOG_W("Wasn't expected\n\r");
	}
	return;
}

static void stopped(struct looper_data *looper, struct midi_message *msg)
{
	result_t rc;

	LOG_D("stopped()\n\r");
	if ((msg->buffer[0] &0xf0) == MIDI_STATUS_NOTE_ON) {
		/*
		 * Set playing with pulse
		 */
		rc = looper_pulse(looper->gpio);
		if (rc < 0) {
			LOG_E("Failed to pulse gpio\n\r");
			return;
		}

		looper->state = playing;
	} else {
		LOG_W("Wasn't expected\n\r");
	}
	return;
}

static void playing(struct looper_data *looper, struct midi_message *msg)
{
	result_t rc;

	LOG_D("playing()\n\r");
	if ((msg->buffer[0] &0xf0) == MIDI_STATUS_NOTE_OFF) {
		/*
		 * Set stopped with double pulse
		 */
		rc = looper_double_pulse(looper->gpio);
		if (rc < 0) {
			LOG_E("Failed to double pulse gpio\n\r");
			return;
		}

		looper->state = stopped;
	} else {
		LOG_W("Wasn't expected\n\r");
	}
	return;
}

static void pulse_off(timer_id timer, union sigval data)
{
	gpio_set(data.sival_int, GPIO_MODE_DIGITAL_OUTPUT, 0);
}

static void first_pulse_off(timer_id timer, union sigval data)
{
	result_t rc;
	gpio_set(data.sival_int, GPIO_MODE_DIGITAL_OUTPUT, 0);

	pulse_timer_req.data.sival_int = data.sival_int;
	pulse_timer_req.exp_fn = second_pulse_on;
	rc = sw_timer_start(&pulse_timer_req);
}

static void second_pulse_on(timer_id timer, union sigval data)
{
	result_t rc;
	gpio_set(data.sival_int, GPIO_MODE_DIGITAL_OUTPUT, 1);

	pulse_timer_req.data.sival_int = data.sival_int;
	pulse_timer_req.exp_fn = pulse_off;
	rc = sw_timer_start(&pulse_timer_req);
}

static result_t looper_pulse(enum gpio_pin gpio)
{
	result_t rc;

	LOG_D("looper_pulse()\n\r");
	rc = gpio_set(gpio, GPIO_MODE_DIGITAL_OUTPUT, 1);

	pulse_timer_req.data.sival_int = gpio;
	pulse_timer_req.exp_fn = pulse_off;
	rc = sw_timer_start(&pulse_timer_req);
	return(rc);
}

static result_t looper_double_pulse(enum gpio_pin gpio)
{
	result_t rc;

	LOG_D("looper_double_pulse()\n\r");
	rc = gpio_set(gpio, GPIO_MODE_DIGITAL_OUTPUT, 1);

	pulse_timer_req.data.sival_int = gpio;
	pulse_timer_req.exp_fn = first_pulse_off;
	rc = sw_timer_start(&pulse_timer_req);
	return(rc);
}

#endif // SYS_LOOPER