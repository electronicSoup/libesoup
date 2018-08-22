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

#define MODBUS_READ_CONFIG     0x03
#define MODBUS_READ_DATA       0x04
#define MODBUS_WRITE_CONFIG    0x06
#define MODBUS_WRITE_MULTIPLE  0x10
#define MODBUS_ID_REQUEST      0x11

struct modbus_channel {
        struct uart_data    *uart;
        timer_id             hw_15_timer;
        timer_id             hw_35_timer;
        timer_id             resp_timer;
        timer_id             turnaround_timer;
        modbus_id            modbus_index;
        uint8_t              address;
        uint8_t              rx_buffer[SYS_MODBUS_RX_BUFFER_SIZE];
        uint16_t             rx_write_index;

        /*
         * function to process unsolicited messages
         */
        modbus_response_function process_unsolicited_msg;

        /*
         * function to process response to sent messages
         */
        modbus_response_function process_response;
//        void                    *response_callback_data;
        void                   (*idle_callback)(modbus_id, uint8_t);
        modbus_response_function slave_frame_handler;
    
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
//    void (*resp_timeout_expiry_fn(timer_t timer_id, union sigval data);
};

extern result_t set_modbus_starting_state(struct modbus_channel *channel);
extern result_t set_modbus_idle_state(struct modbus_channel *channel);
extern result_t set_modbus_transmitting_state(struct modbus_channel *channel);
extern result_t set_modbus_awaiting_response_state(struct modbus_channel *channel);
extern result_t set_modbus_turnaround_state(struct modbus_channel *chan);

extern result_t start_15_timer(struct modbus_channel *channel);
extern result_t start_35_timer(struct modbus_channel *channel);

extern result_t modbus_tx_data(struct modbus_channel *channel, uint8_t *data, uint16_t len);

extern uint16_t crc_calculate(uint8_t *data, uint16_t len);
extern uint8_t crc_check(uint8_t *data, uint16_t len);

#endif //  SYS_MODBUS
#endif //  _MODBUS_PRIVATE_H
