/**
 * @file libesoup/comms/modbus/master_states/transmitting.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus transmission
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

#if defined(SYS_MODBUS) && defined(SYS_MODBUS_MASTER)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MB_M_Tx";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

//#define TEST_MODBUS_LOOPBACK

#if defined(SYS_TEST_BUILD) && defined(TEST_MODBUS_LOOPBACK)
void test_rx(timer_id timer, union sigval data)
{
	struct modbus_channel *chan = (struct modbus_channel *)data.sival_ptr;
	uint8_t   buffer[6] = {0x01, 0x02, 0x03, 0x04};

	buffer[4] = 0xa1;
	buffer[5] = 0x2b;

	uart_test_rx_buffer(&chan->app_data->uart_data, (uint8_t *)&buffer, 6);
}
#endif // SYS_TEST_BUILD

static void tx_finished(struct modbus_channel *chan)
{
#if defined(SYS_TEST_BUILD) && defined(TEST_MODBUS_LOOPBACK)
	result_t          rc;
	struct timer_req  request;
#endif
        LOG_D("tx_finished()\n\r");
	set_master_awaiting_response_state(chan);

#if defined(SYS_TEST_BUILD) && defined(TEST_MODBUS_LOOPBACK)
	request.period.units    = mSeconds;
	request.period.duration = 50;
	request.type            = single_shot;
	request.data.sival_ptr  = chan;
	request.exp_fn          = test_rx;
	
	rc = sw_timer_start(&request);
#endif
}

result_t set_master_transmitting_state(struct modbus_channel *chan)
{
	LOG_D("set_master_transmitting_state()\n\r");
	chan->state                    = mb_m_transmitting;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = NULL;
	chan->transmit                 = NULL;
	chan->modbus_tx_finished       = tx_finished;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;

	if(chan->app_data->idle_state_callback) {
		chan->app_data->idle_state_callback(chan->app_data->channel_id, FALSE);
	}
	
	return(SUCCESS);
}

#endif // SYS_MODBUS
