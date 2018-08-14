/**
 * @file libesoup/comms/can/baud_auto_detect.c
 *
 * @author John Whitmore
 * 
 * @brief Protocol for auto detecting CAN Bus baud rate
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
#include "libesoup_config.h"

#ifdef SYS_CAN_BAUD_AUTO_DETECT

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "CAN_BAD";
#include "libesoup/logger/serial_log.h"
#endif  // SYS_SERIAL_LOGGING

#include "libesoup/errno.h"
#include "libesoup/comms/can/can.h"
#include "libesoup/timers/sw_timers.h"

#ifndef SYS_SW_TIMERS
#error libesoup_config.h should define SYS_SW_TIMERS, required by CAN Bit Rate Detection
#endif

#ifndef SYS_CAN_BAUD_AUTO_DETECT_PERIOD
#error libesoup_config.h should define SYS_CAN_BAUD_AUTO_DETECT_PERIOD, the listening period
#endif

static can_baud_rate_t    current_baud_rate;
static struct timer_req   request;

extern result_t can_l2_bitrate(can_baud_rate_t baud);
extern result_t can_l2_get_rx_count(void);
extern result_t can_l2_baud_found(can_baud_rate_t rate);

result_t can_bad_start_baud_scan();

void expiry(timer_id timer, union sigval data)
{
	result_t rc;

	rc = can_l2_get_rx_count();
	LOG_D("End of Period received %d frames\n\r", rc);
	
	if(rc == 0) {
		current_baud_rate++;
		
		if(current_baud_rate < no_baud) {
			rc = can_l2_bitrate(current_baud_rate);
			LOG_D("Listening on Bit Rate %s\n\r", can_baud_rate_strings[current_baud_rate]);
			rc = sw_timer_start(&request);
			RC_CHECK_PRINT_VOID("Failed to start\n\r")
		} else {
			can_bad_start_baud_scan();
		}
	} else if(rc >0) {
		can_l2_baud_found(current_baud_rate);
	}
}

result_t can_bad_start_baud_scan()
{
	result_t rc;

	LOG_D("can_bad_start_baud_scan()\n\r");
	
	/*
	 * Initialise the timer request structure
	 */
	request.period.units    = Seconds;
	request.period.duration = SYS_CAN_BAUD_AUTO_DETECT_PERIOD;
	request.type            = single_shot;
	request.exp_fn          = expiry;
	request.data.sival_int  = 0x00;
	
	current_baud_rate =  0;
	rc = can_l2_bitrate(current_baud_rate);
	
	while((rc == -ERR_CAN_BITRATE_LOW) && (current_baud_rate < no_baud)) {
		current_baud_rate++;
		rc = can_l2_bitrate(current_baud_rate);
	}
	if(rc < 0) { 
		LOG_E("Set bitrate failed %d\n\r", -rc);
		return(rc);
	}
	
	if(current_baud_rate == no_baud)
		return(-ERR_CAN_INVALID_BAUDRATE);
	
	LOG_D("Listening on Bit Rate %s\n\r", can_baud_rate_strings[current_baud_rate]);

	/*
	 * Start a timer and see if we've received any valid frames.
	 */
	rc = sw_timer_start(&request);
	RC_CHECK
	return(0);
}

#endif // SYS_CAN_BAUD_AUTO_DETECT
