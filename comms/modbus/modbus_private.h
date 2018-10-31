/**
 * @file libesoup/comms/modbus/modbus_private.h
 *
 * @author John Whitmore
 *
 * Function prototypes for using modbus Comms.
 * 
 * http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
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
#ifndef _MODBUS_PRIVATE_H
#define _MODBUS_PRIVATE_H

#include "libesoup_config.h"

#ifdef SYS_MODBUS

#include "libesoup/comms/uart/uart.h"
#include "libesoup/comms/modbus/modbus.h"
#include "libesoup/timers/sw_timers.h"

enum modbus_state {
        mb_m_starting,
        mb_m_idle,
        mb_m_transmitting,
        mb_m_awaiting_response,
        mb_m_turnaround,

        mb_s_idle,
        mb_s_receiving,
        mb_s_processing_request,
        mb_s_transmitting,
};

struct modbus_channel {
        uint8_t                  modbus_index;
        enum modbus_state        state;
        struct modbus_app_data  *app_data;
        timer_id                 hw_15_timer;
        timer_id                 hw_35_timer;
        timer_id                 resp_timer;
        timer_id                 turnaround_timer;
        uint8_t                  rx_buffer[SYS_MODBUS_RX_BUFFER_SIZE];
        uint16_t                 rx_write_index;
        uint8_t                  tx_modbus_address;

        /*
         * function to process response to sent messages
         */
        modbus_response_function process_response;
    
        /*
         * The higher layer application code will pass in a tx_finished() in the 
         * uart structure. Modbus code will hijack that function and call it from 
         * the modbus finished function.
         */
        void                   (*app_tx_finished)(struct uart_data *);
        void                   (*modbus_tx_finished)(struct modbus_channel *chan);
        void                   (*process_timer_15_expiry)(struct modbus_channel *chan);
        void                   (*process_timer_35_expiry)(struct modbus_channel *chan);
        result_t               (*transmit)(struct modbus_channel *chan, uint8_t *data, uint16_t len, modbus_response_function callback);
        void                   (*process_rx_character)(struct modbus_channel *channel, uint8_t ch);
        void                   (*process_response_timeout)();
};

#if defined(SYS_MODBUS_MASTER)
extern result_t set_master_starting_state(struct modbus_channel *chan);
extern result_t set_master_idle_state(struct modbus_channel *channel);
extern result_t set_master_transmitting_state(struct modbus_channel *channel);
extern result_t set_master_awaiting_response_state(struct modbus_channel *channel);
extern result_t set_master_turnaround_state(struct modbus_channel *chan);
#endif

#if defined(SYS_MODBUS_SLAVE)
extern result_t set_slave_idle_state(struct modbus_channel *chan);
extern result_t set_slave_receiving_state(struct modbus_channel *chan);
extern result_t set_slave_processing_request_state(struct modbus_channel *chan);
extern result_t set_slave_transmitting_state(struct modbus_channel *channel);
#endif

extern result_t start_15_timer(struct modbus_channel *channel);
extern result_t start_35_timer(struct modbus_channel *channel);

extern result_t modbus_tx_data(struct modbus_channel *channel, uint8_t *data, uint16_t len);

extern uint16_t crc_calculate(uint8_t *data, uint16_t len);
extern uint8_t crc_check(uint8_t *data, uint16_t len);

#endif //  SYS_MODBUS
#endif //  _MODBUS_PRIVATE_H
