/**
 *
 * \file es_lib/utils/modbus.h
 *
 * Function prototypes for using modbus Comms.
 *
 * The first uart port is used by the logger. See es_lib/logger
 *
 * Copyright 2015 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "es_lib/comms/uart.h"

typedef void (*modbus_response_function)(u8 *msg, u8 size, void *data);

struct modbus_channel {
    struct uart_data        *uart;
    u8                       hw_15_timer;
    u8                       hw_35_timer;
    es_timer                 resp_timer;
    u8                       address;
    u8                       rx_buffer[MODBUS_RX_BUFFER_SIZE];
    u16                      rx_write_index;

    /*
     * function to process unsolicited messages
     */
    modbus_response_function process_unsolicited_msg;

    /*
     * function to process response to sent messages
     */
    modbus_response_function process_response;
    void                    *response_callback_data;
    void                   (*idle_callback)(void*);
    void                    *idle_callback_data;
    
    /*
     * The higher layer applicaton code will pass in a tx_finished() in the 
     * uart structure. Modbus code will hijack that function and call it from 
     * the modbus finished function.
     */
    void                   (*app_tx_finished)(void *);
    void                   (*modbus_tx_finished)(void *);
    void                   (*process_timer_15_expiry)(void *);
    void                   (*process_timer_35_expiry)(void *);
    void                   (*transmit)(struct modbus_channel *channel, u8 *data, u16 len, modbus_response_function fn, void *callback_data);
    void                   (*process_rx_character)(struct modbus_channel *channel, u8 ch);
    void                   (*process_response_timeout)();
//    void (*resp_timeout_expiry_fn(timer_t timer_id, union sigval data);

};


#define MODBUS_READ_CONFIG     0x03
#define MODBUS_READ_DATA       0x04
#define MODBUS_WRITE_CONFIG    0x06
#define MODBUS_WRITE_MULTIPLE  0x10
#define MODBUS_ID_REQUEST      0x11

extern void set_modbus_starting_state(struct modbus_channel *channel);
extern void set_modbus_idle_state(struct modbus_channel *channel);
extern void set_modbus_transmitting_state(struct modbus_channel *channel);
extern void set_modbus_awaiting_response_state(struct modbus_channel *channel);

extern void start_15_timer(struct modbus_channel *channel);
extern void start_35_timer(struct modbus_channel *channel);

extern result_t modbus_reserve(struct uart_data *uart, void (*idle_callback)(void *), modbus_response_function, void *);
extern result_t modbus_release(struct uart_data *uart);
extern void modbus_tx_data(struct modbus_channel *channel, u8 *data, u16 len);
extern result_t modbus_attempt_transmission(struct uart_data *uart, u8 *data, u16 len, modbus_response_function fn, void *callback_data);

extern u16 crc_calculate(u8 *data, u16 len);
extern u8 crc_check(u8 *data, u16 len);


extern result_t start_response_timer(struct modbus_channel *channel);
extern result_t cancel_response_timer(struct modbus_channel *channel);
