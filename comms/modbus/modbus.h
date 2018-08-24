/**
 * @file libesoup/comms/modbus/modbus.h
 *
 * @author John Whitmore
 *
 * Function prototypes for using modbus Comms.
 * 
 * Layer 7 http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf
 * Layers 1 & 2 http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
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
#ifndef _MODBUS_H
#define _MODBUS_H

#include "libesoup_config.h"

#ifdef SYS_MODBUS

#include "libesoup/comms/uart/uart.h"

#define MODBUS_BROADCAST_ADDRESS  0x00

/*
 * MODBUS addresses above 247 are reserved and should not be used.
 */
#define MODBUS_MAX_ADDRESS         247

/**
 * @typedef  modbus_id
 * @brief    This is an overlay type to the result_t return code type.
 *
 * Modbus channel identifier is returned by the 
 */
typedef int16_t modbus_id;

/**
 * @ingroup MODBUS
 * @typedef modbus_response_function - callback function to process received
 *                                     MODBUS frames.
 * 
 * @input   modbus_id chan  MODBUS Channel identifier, of the channel over which
 *                          the frame was received.
 * @input   uint8_t  *frame MODBUS Frame to be processed.
 * @input   uint8_t   len   Length of the received MODBUS frame.
 */
typedef void (*modbus_response_function)(modbus_id chan,
					 uint8_t *frame,
					 uint8_t len);

struct modbus_app_data {
        modbus_id                 channel_id;
        struct uart_data          uart_data;
        uint8_t                   address;
        void                    (*idle_state_callback)(modbus_id, uint8_t);
        modbus_response_function  unsolicited_frame_handler;
        modbus_response_function  broadcast_frame_handler;
};

/**
 * @ingroup MODBUS
 * @fn      modbus_reserve
 * 
 * @brief   Function to reserve a MODBUS channel for Master communications.
 *          a modbus address of 0 indicates master functionality.
 *
 * @return  Modbus Channel Identifier, or error on fail.
 */
extern modbus_id modbus_reserve(struct modbus_app_data *app_data);

/**
 * @ingroup MODBUS
 * @fn      modbus_release
 * 
 * @brief   API Function to relase a MODBUS Channel, which is no longer required
 *          by the application.
 *
 * @param   modbus_id       Identifer of the MODBUS channel to be released.
 *
 * @return  result_t        SUCCESS
 *                          -ERR_BAD_INPUT_PARAMETER
 */
extern result_t  modbus_release(struct modbus_app_data *app_data);

extern result_t  modbus_read_config_req(modbus_id                chan,
			   	        uint8_t                  modbus_address,
                                        uint16_t                 mem_address,
				        modbus_response_function callback);

extern result_t  modbus_read_config_resp(modbus_id                chan,
                                         uint16_t                 mem_address);
#endif //  SYS_MODBUS
#endif //  _MODBUS_H
