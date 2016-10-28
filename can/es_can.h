/**
 *
 * \file es_lib/can/es_can.h
 *
 * Core CAN Bus definitions
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#ifndef ES_CAN_H
#define ES_CAN_H

#include "system.h"

/*
 * ISO15765 Protocols
 */
#if defined(ISO15765)
#define ISO15765_LOGGER_PROTOCOL_ID    0x01
#define ISO15765_DCNCP_PROTOCOL_ID     0x02
#endif

/*
 * A Node address of 0xff is a Broadcast Network node address in ISO11783 so
 * we'll not use it.
 */
#define BROADCAST_NODE_ADDRESS         0xff

/*
 * Have to change this status to cater for L2, DCNCP and L3
 *
 * Layer 2 - 5 values so 3 bits L2_STATUS_MASK 0x07
 *
 *	L2_Uninitialised 0x00,
 *	L2_Listening     0x01,
 *	L2_Connecting    0x02,
 *	L2_Connected     0x03,
 *	L2_ChangingBaud  0x04
 *
 * DCNCP - 3 Values for status so 2 bits
 *         Actually use a bit filed for this
 *
 *         DCNCP_STATUS_INITIALISED_MASK 0x08
 *
 *             DCNCP_Uninitilised 0x00,
 *             DCNCP_Initialised  0x08,
 *
 *         DCNCP_L3_ADDRESS_STATUS_MASK 0x10
 *
 *             DCNCP_NODE_Address_Not_Final 0x00,
 *             DCNCP_NODE_Address_Finalised 0x10,
 *
 * Layer 3 - 2 values Initialised or not. 1 bit
 *
 *         L3_STATUS_INITIALISED_MASK 0x20
 *
 *             L3_Uninitialised 0x00
 *             L3_Inititialised 0x20
 */

#define L2_STATUS_MASK                 0x07

#define L2_Uninitialised               0x00
#define L2_Listening                   0x01
#define L2_Connecting                  0x02
#define L2_Connected                   0x03
#define L2_ChangingBaud                0x04

#if LOG_LEVEL < NO_LOGGING
extern char can_l2_status_strings[5][17];
#endif

#ifdef DCNCP_CAN
#define DCNCP_INIT_STATUS_MASK         0x08
#define DCNCP_NODE_ADDRESS_STATUS_MASK 0x10
#endif // DCNCP_CAN

typedef struct {
    union {
        struct {
            u8 l2_status : 3;
            u8 dcncp_initialised : 1;
            u8 dcncp_node_address_valid :1;
        } bit_field;
        u8 byte;
    };
} can_status_t;

typedef enum {
	baud_10K   = 0x00,
	baud_20K   = 0x01,
	baud_50K   = 0x02,
	baud_125K  = 0x03,
	baud_250K  = 0x04,
	baud_500K  = 0x05,
	baud_800K  = 0x06,
	baud_1M    = 0x07,
	no_baud    = 0x08
} can_baud_rate_t;

typedef void (*can_status_handler_t)(can_status_t, can_baud_rate_t);

#if LOG_LEVEL < NO_LOGGING
extern char can_baud_rate_strings[8][10];
#endif

//#define BAUD_MAX baud_1M

extern result_t can_init(can_baud_rate_t      baud,
	can_status_handler_t   status_default_handler);

//extern bool can_initialised(void);

extern result_t can_l2_init(can_baud_rate_t arg_baud_rate,
                 void (*arg_status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud));
extern void can_l2_tasks(void);

extern result_t can_l2_tx_frame(can_frame *frame);
extern result_t can_l2_dispatch_reg_handler(can_l2_target_t *target);
extern result_t can_l2_dispatch_unreg_handler(u8 id);
extern result_t can_l2_dispatch_set_unhandled_handler(can_l2_frame_handler_t handler);

//extern void can_l2_ISR(void);

//extern void can_l2_tx_error(u8 node_type, u8 node_number, u32 errorCode);

extern can_baud_rate_t can_l2_get_baudrate(void);
extern void can_l2_set_node_baudrate(can_baud_rate_t baudrate);
extern void can_l2_get_status(can_status_t *, can_baud_rate_t *);

#ifdef MCP
extern void can_tasks(void);
#endif


#if defined(ISO15765)

extern u8 node_get_address(void);

extern result_t iso15765_init(u8 address);
extern u8 iso15765_initialised(void);

extern result_t iso15765_tx_msg(iso15765_msg_t *msg);
extern result_t iso15765_dispatch_reg_handler(iso15765_target_t *target);
extern result_t iso15765_dispatch_unreg_handler(u8 id);
extern result_t iso15765_dispatch_set_unhandled_handler(iso15765_msg_handler_t handler);

#endif

#if defined(ISO11783)


typedef enum {
    ack            = 0x00,
    nack           = 0x01,
    denied         = 0x02,
    cannot_respond = 0x03
} iso11783_ack_t;


extern result_t  iso11783_init(u8);

extern result_t iso11783_tx_msg(iso11783_msg_t *msg);
extern result_t iso11783_dispatch_reg_handler(iso11783_target_t *target);
extern result_t iso11783_dispatch_unreg_handler(u8 id);
extern result_t iso11783_dispatch_set_unhandled_handler(iso11783_msg_handler_t handler);
#endif // ISO11783

#endif // CAN_H
