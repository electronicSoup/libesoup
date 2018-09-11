/**
 * @file libesoup/comms/modbus/modbus.h
 *
 * @author John Whitmore
 *
 * Function prototypes for using modbus Comms.
 * 
 * Specs are at http://www.modbus.org/specs.php
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
 * This implementation does not fully implement the standard.
 * 
 * - Only RTU Mode is implemented.
 * - T15 not implemented
 * - No processing of response received from the wrong slave.
 * - Incorrect implementation of Response timeout, which currently gets
 *   cancelled on the first response byte received.
 * - No retry counters.
 * - No diagnostics counters
 * - Turnaround timer not correctly started on broadcast message transmission
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

/*
 * MODBUS Layer 7 Function Codes
 */
#define MODBUS_READ_COILS                      0x01
#define MODBUS_READ_DISCRETE_INPUT             0x02
#define MODBUS_READ_HOLDING_REGISTERS          0x03
#define MODBUS_READ_INPUT_REGISTER             0x04
#define MODBUS_WRITE_SINGLE_COIL               0x05
#define MODBUS_WRITE_SINGLE_REGISTER           0x06
#define MODBUS_READ_EXCEPTION_STATUS           0x07
#define MODBUS_DIAGNOSTICS                     0x08
#define MODBUS_GET_COMM_EVENT_COUNTER          0x0B
#define MODBUS_GET_COMM_EVENT_LOG              0x0C
#define MODBUS_WRITE_MULTIPLE_COILS            0x0F
#define MODBUS_WRITE_MULTIPLE_REGISTERS        0x10
#define MODBUS_REPORT_SERVER_ID                0x11
#define MODBUS_READ_FILE_RECORD                0x14
#define MODBUS_WRITE_FILE_RECORD               0x15
#define MODBUS_MASK_WRITE_REGISTER             0x16
#define MODBUS_READ_WRITE_MULTIPLE_REGISTERS   0x17
#define MODBUS_READ_FIFO_QUEUE                 0x18

#define MODBUS_CANOPEN_REQUEST                 0x2B
#define MODBUS_CANOPEN_RESPONSE                0x0D

#define MODBUS_READ_DEVICE_IDENTIFICATION_0E   0x0E
#define MODBUS_READ_DEVICE_IDENTIFICATION_2B   0x2B

/*
 * MODBUS Layer 7 Exception codes
 */
#define MODBUS_FUNCTION_CODE_EXCEPTION         0x01
#define MODBUS_ADDRESS_EXCEPTION               0x02
#define MODBUS_DATA_EXCEPTION                  0x03

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
 * @brief   MODBUS Function to release channel, no longer needed.
 *
 * @param   struct modbus_app_data    *app_data
 * 
 *          - Application data of the MODBUS channel to release.
 * 
 * @return  result_t        SUCCESS
 *                          -ERR_BAD_INPUT_PARAMETER
 */
extern result_t  modbus_release(struct modbus_app_data *app_data);

/**
 * @ingroup MODBUS
 * @fn      modbus_read_coils_req
 * 
 * @brief   MODBUS Master Function to read coil status.
 *          MODBUS Function code 0x01
 *
 * @param   modbus_id                  chan
 * 
 *          - Identifier of the MODBUS channel to use.
 * 
 * @param   uint8_t                    modbus_address
 * 
 *          - MODBUS Address of the slave to read the coil status of.
 * 
 * @param   uint16_t                   coil_address
 * 
 *          - Address of the first coil to read status of.
 * 
 * @param   uint16_t                   number_of_coils
 * 
 *          - Number of coils to read the status of
 * 
 * @param   modbus_response_function   callback
 * 
 *          - Callback function to be called with response from the Slave.
 *
 * @return  result_t        SUCCESS
 *                          -ERR_BAD_INPUT_PARAMETER
 */
#if defined(SYS_MODBUS_MASTER)
extern result_t  modbus_read_coils_req(modbus_id                chan,
			   	       uint8_t                  modbus_address,
                                       uint16_t                 coil_address,
                                       uint16_t                 number_of_coils,
				       modbus_response_function callback);
#endif // SYS_MODBUS_MASTER

/**
 * @ingroup MODBUS
 * @fn      modbus_read_holding_regs_req
 * 
 * @brief   MODBUS Master Function to read holding registers.
 *          MODBUS Function Code 0x03
 *
 * @param   modbus_id                  chan
 * 
 *          - Identifier of the MODBUS channel to use.
 * 
 * @param   uint8_t                    modbus_address
 * 
 *          - MODBUS Address of the slave to read the coil status of.
 * 
 * @param   uint16_t                   coil_address
 * 
 *          - Address of the first coil to read status of.
 * 
 * @param   uint16_t                   number_of_coils
 * 
 *          - Number of coils to read the status of
 * 
 * @param   modbus_response_function   callback
 * 
 *          - Callback function to be called with response from the Slave.
 *
 * @return  result_t        SUCCESS
 *                          -ERR_BAD_INPUT_PARAMETER
 */
#if defined(SYS_MODBUS_MASTER)
extern result_t  modbus_read_holding_regs_req(modbus_id                chan,
                                              uint8_t                  modbus_address,
                                              uint16_t                 coil_address,
                                              uint16_t                 number_of_coils,
				              modbus_response_function callback);
#endif // SYS_MODBUS_MASTER

/**
 * @ingroup MODBUS
 * @fn      modbus_error_response
 * 
 * @brief   MODBUS Slave Function to return an error to the Master
 *
 * @param   modbus_id                  chan
 * 
 *          - Identifier of the MODBUS channel to use.
 * 
 * @param   uint8_t                    modbus_function
 *
 *          - The MODBUS Function which was received by the Slave
 * 
 * @param   uint8_t                    exception
 * 
 *          - The error code for the exception found
 *
 * @return  result_t        SUCCESS
 *                          -ERR_BAD_INPUT_PARAMETER
 */
#if defined(SYS_MODBUS_SLAVE)
extern result_t  modbus_error_resp(modbus_id  chan,
                                   uint8_t    modbus_function,
                                   uint8_t    exception);
#endif // SYS_MODBUS_SLAVE

/**
 * @ingroup MODBUS
 * @fn      modbus_read_coils_resp
 * 
 * @brief   MODBUS Slave Function to return requested coil status to Master
 *
 * @param   modbus_id                  chan
 * 
 *          - Identifier of the MODBUS channel to use.
 * 
 * @param   uint8_t                   *buffer
 * 
 *          - Address of the first byte to be returned to the MODBUS Master
 * 
 * @param   uint8_t                    len
 * 
 *          - Number of coil status bytes in the buffer to be retruned to the
 *            Modbus Master.
 *
 * @return  result_t        SUCCESS
 *                          -ERR_BAD_INPUT_PARAMETER
 */
#if defined(SYS_MODBUS_SLAVE)
extern result_t  modbus_read_coils_resp(modbus_id   chan,
                                        uint8_t    *buffer,
                                        uint8_t     len);
#endif // SYS_MODBUS_SLAVE

#if defined(SYS_MODBUS_SLAVE)
extern result_t  modbus_read_registers_resp(modbus_id   chan,
	                                    uint8_t    *buffer,
                                            uint8_t     len);
#endif // SYS_MODBUS_SLAVE

#endif //  SYS_MODBUS
#endif //  _MODBUS_H
