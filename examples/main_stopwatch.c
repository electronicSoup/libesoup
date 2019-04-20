/**
 * @file libesoup/examples/main_sw_timers.c
 *
 * @author John Whitmore
 *
 * Copyright 2019 electronicSoup Limited
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
/*
 * This example main is used in YouTube episode: https://youtu.be/sT5RIbYrS0s
 * where an Infra Red Remote Control is decoded so that we can replace the
 * control board.
 *
 * The input signal is received on Pin D0 and we're timing the pulses to
 * determine which button on the remote has been pressed.
 *
 * The associated project is located in examples/projects/microchip/IR_Remote.X
 */

#include "libesoup_config.h"

//#define DEBUG

#define DEBUG_FILE
static const char *TAG = "Main";

#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/change_notification.h"
#include "libesoup/timers/time.h"
#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/timers/delay.h"
#include "libesoup/timers/stop_watch.h"

#define START_THRESHOLD     1
#define START_PULSE         2
#define END_THRESHOLD       1
#define END_PULSE           2
#define BIT_THRESHOLD       2
#define ZERO_PULSE         17
#define ONE_PULSE          22

static timer_id            stopwatch_timer    = BAD_TIMER_ID;
static timer_id            eob_timer          = BAD_TIMER_ID;
static struct timer_req    stopwatch_request;
static struct period       period;
static uint8_t             atomic;
static uint8_t             num_bits;
static uint8_t             rx_data;

static void idle_state(void);
static void awaiting_end_of_initial_start_pulse(void);
static void awaiting_end_of_start_pulse(void);
static void awaiting_start_of_end_pulse(void);
static void awaiting_end_of_end_pulse(void);

static void (*current_state)(void) = (void (*)(void))NULL;

void change_notification(enum gpio_pin pin)
{
	result_t rc;

	if (stopwatch_timer != BAD_TIMER_ID) {
		rc = hw_timer_stop(stopwatch_timer, &period);
	}
	stopwatch_timer = hw_timer_start(&stopwatch_request);
	if (stopwatch_timer < 0) {
		LOG_E("Timer Fail\n\r");
		return;
	}
	atomic++;
}

void end_of_byte_expiry(timer_id timer, union sigval data)
{
	result_t rc;

	eob_timer     = BAD_TIMER_ID;
	current_state = idle_state;

	LOG_D("Data - 0x%x\n\r", rx_data);
	/*
	 * Cancel the stop watch timer if it's running
	 */
	if (stopwatch_timer != BAD_TIMER_ID) {
		rc = hw_timer_stop(stopwatch_timer, &period);
		stopwatch_timer = BAD_TIMER_ID;
	}
	LOG_D("Waiting...\n\r");
}

static void idle_state(void)
{
	if (!PORTDbits.RD0) {
		/*
		 * Expect the start of a start pulse
		 */
		rx_data = 0x00;
		current_state   = awaiting_end_of_initial_start_pulse;
	}
}

static void awaiting_end_of_initial_start_pulse(void)
{
	struct timer_req  sw_timer_req;
	/*
	 * Assume we're going back to the idle state unless we get a 
	 * good start pulse
	 */
	current_state = idle_state;

	if (PORTDbits.RD0) {
		if (period.duration >= (START_PULSE - START_THRESHOLD) && period.duration <= (START_PULSE + START_THRESHOLD)) {
			current_state = awaiting_start_of_end_pulse;

			sw_timer_req.period.units         = mSeconds;
			sw_timer_req.period.duration      = 50;
			sw_timer_req.type                 = single_shot_expiry;
			sw_timer_req.exp_fn               = end_of_byte_expiry;
			sw_timer_req.data.sival_int       = 0;
			eob_timer                         = sw_timer_start(&sw_timer_req);
			
			num_bits                          = 0;
		} else {
			LOG_E("Start bit %d?\n\r", period.duration);
		}
	}
}

static void awaiting_start_of_start_pulse(void)
{
	current_state = NULL;

	if (!PORTDbits.RD0) {
		current_state = awaiting_end_of_start_pulse;
	}
}

static void awaiting_end_of_start_pulse(void)
{
	current_state = NULL;

	if (PORTDbits.RD0) {
		if (period.duration >= (START_PULSE - START_THRESHOLD) && period.duration <= (START_PULSE + START_THRESHOLD)) {
			current_state = awaiting_start_of_end_pulse;
		}
	}
}

static void awaiting_start_of_end_pulse(void)
{
	current_state = NULL;
	if (!PORTDbits.RD0) {
		if (period.duration >= (ZERO_PULSE - BIT_THRESHOLD) && period.duration <= (ZERO_PULSE + BIT_THRESHOLD)) {
			rx_data = rx_data << 1;
			num_bits++;
		} else if (period.duration >= (ONE_PULSE - BIT_THRESHOLD) && period.duration <= (ONE_PULSE + BIT_THRESHOLD)) {
			rx_data = rx_data << 1;
			rx_data |= 0x01;
			num_bits++;
		}

		if ( num_bits > 8 ) {
			LOG_E("Received %d bits\n\r", num_bits)			
		}
		current_state = awaiting_end_of_end_pulse;
	}
}

static void awaiting_end_of_end_pulse(void)
{
	current_state = NULL;

	if (PORTDbits.RD0) {
		if (period.duration >= (END_PULSE - END_THRESHOLD) && period.duration <= (END_PULSE + END_THRESHOLD)) {
			current_state                   = awaiting_start_of_start_pulse;
		}
	}
}

int main(void)
{
        result_t         rc;

	period.units                      = Tenths_mSeconds;
	stopwatch_request.period.units    = Tenths_mSeconds;
	stopwatch_request.type            = stopwatch;
	stopwatch_request.data.sival_int  = 0;
	current_state                     = idle_state;
	stopwatch_timer                   = BAD_TIMER_ID;
	atomic                            = 0;

	rc = libesoup_init();

	rc = gpio_set(RD3, GPIO_MODE_DIGITAL_OUTPUT, 1);
	if (rc < 0) {
		LOG_E("Failed to set GPIO Pin mode\n\r");
	}

	rc = gpio_set(RD0, GPIO_MODE_DIGITAL_INPUT, 0);
	if (rc < 0) {
		LOG_E("Failed to set GPIO Pin mode\n\r");
	}
	rc = change_notifier_register(RD0, change_notification);
	if(rc < 0) {
		LOG_E("change_notifier_register()\n\r");
	}

        LOG_D("Starting %ld Hz\n\r", sys_clock_freq);

        while(1) {
		if (atomic > 1) {
			LOG_E("Atomic Overflow\n\r");			
		}
		while (atomic) {
			if (current_state) {
				current_state();
			}
			period.duration = 0;
			atomic--;
		}
                libesoup_tasks();
        }
        return 0;
}
