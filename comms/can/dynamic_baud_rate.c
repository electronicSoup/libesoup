/*
 *
 * libesoup/comms/can/dynamic_baud_rate.c
 *
 * CAN Bus dynamic Bit Rate protocol
 * 
 * This protocol allows the nodes on the network switch to antoher Bit Rate
 * during normal operation.
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
 *******************************************************************************
 *
 */
#include "libesoup_config.h"

#ifdef SYS_CAN_DYNAMIC_BAUD_RATE

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "DYN_BAUD";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/can/can.h"
#include "libesoup/timers/sw_timers.h"

static void     exp_finalise_baudrate_change(timer_id timer, union sigval data);
static void     exp_resend_baudrate_change(timer_id timer, union sigval data);


void can_db_set_baudrate(can_baud_rate_t baudrate)
{
	timer_id timer;
	struct timer_req timer_request;
	

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("set_can_node_baudrate()\n\r");
#endif
//	status.bit_field.l2_status = L2_ChangingBaud;
//	status_baud = baudrate;

//	if (status_handler)
//		status_handler(L2_STATUS_MASK, status, status_baud);

	set_can_mode(CONFIG_MODE);

	set_baudrate(baudrate);

	/*
	 * The Baud rate is being changed so going to stay in config mode
	 * for 5 Seconds and let the Network settle down.
	 */
	timer_request.units = Seconds;
	timer_request.duration = 5;
	timer_request.type = single_shot;
	timer_request.exp_fn = exp_finalise_baudrate_change;
	timer_request.data.sival_int = 0;
	sw_timer_start(&timer, &timer_request);
}

static void exp_finalise_baudrate_change(timer_id timer __attribute__((unused)), union sigval data __attribute__((unused)))
{
#if (defeind(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("exp_finalise_baudrate_change()\n\r");
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
	timer_id         timer;
	struct timer_req timer_request;
	can_frame        msg;
	result_t         result = SUCCESS;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("initiate_can_baudrate_change()\n\r");
#endif
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

		timer_request.units = mSeconds;
		timer_request.duration = 500;
		timer_request.type = single_shot;
		timer_request.exp_fn = exp_resend_baudrate_change;
		timer_request.data.sival_int = 0;
		sw_timer_start(&timer, &timer_request);
	}
}

static void exp_resend_baudrate_change(timer_id exp_timer __attribute__((unused)), union sigval data __attribute__((unused)))
{
	can_frame        msg;
	timer_id         timer;
	struct timer_req timer_request;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("exp_resend_baudrate_change()\n\r");
#endif
	if(changing_baud_tx_error < 3) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("resending Baud Rate Change Request %d\n\r", changing_baud_tx_error);
#endif
		msg.can_id = 0x705;
		msg.can_dlc = 1;

		msg.data[0] = status_baud;

		if(can_l2_tx_frame(&msg) != ERR_CAN_NO_FREE_BUFFER) {
			timer_request.units = mSeconds;
			timer_request.duration = 500;
			timer_request.type = single_shot;
			timer_request.exp_fn = exp_resend_baudrate_change;
			timer_request.data.sival_int = 0;
			sw_timer_start(&timer, &timer_request);
		} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("No Free Buffers so change the Baud Rate\n\r");
#endif
			set_reg_mask_value(TXB0CTRL, TXREQ, 0x00);
			set_reg_mask_value(TXB1CTRL, TXREQ, 0x00);
			set_reg_mask_value(TXB2CTRL, TXREQ, 0x00);
                        can_l2_set_node_baudrate(status_baud);
		}
	} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("3 Errors so NOT Resending Baud Rate Change Request\n\r");
#endif
                can_l2_set_node_baudrate(status_baud);
	}
}


#endif // SYS_CAN_DYNAMIC_BAUD_RATE
