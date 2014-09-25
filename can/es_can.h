/**
 *
 * \file es_lib/can/es_can.h
 *
 * Core CAN Bus definitions
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#ifndef ES_CAN_H
#define ES_CAN_H

/*
 * Layer 3 Protocols
 */
#if defined(CAN_LAYER_3)
#define NODE_MANAGEMENT_L3_ID   0xc8  //0x01
#define NET_LOG_L3_ID   0xc9  // 0x02
#endif

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
 *             DCNCP_L3_Address_Not_Final 0x00,
 *             DCNCP_L3_Address_Finalised 0x10,
 *
 * Layer 3 - 2 values Initialised or not. 1 bit
 *
 *         L3_STATUS_INITIALISED_MASK 0x20
 *
 *             L3_Uninitialised 0x00
 *             L3_Inititialised 0x20
 */

#define L2_STATUS_MASK 0x07

#define L2_Uninitialised 0x00
#define L2_Listening     0x01
#define L2_Connecting    0x02
#define L2_Connected     0x03
#define L2_ChangingBaud  0x04

#define DCNCP_STATUS_MASK 0x18

#define DCNCP_INITIALISED_MASK 0x08

#define DCNCP_Uninitilised 0x00
#define DCNCP_Initialised  0x08

#define DCNCP_L3_ADDRESS_STATUS_MASK 0x10

#define DCNCP_L3_Address_Not_Final 0x00
#define DCNCP_L3_Address_Finalised 0x10

#define L3_STATUS_MASK 0x20

#define L3_Uninitialised 0x00
#define L3_Inititialised 0x20

typedef struct {
    union {
        struct {
            u8 l2_status : 3;
            u8 dcncp_status : 2;
            u8 l3_status : 1;
        } bit_field;
        u8 byte;
    };
} can_status_t;
#if 0
typedef enum {
	Uninitialised,
	Listening,
	Connecting,
	Connected,
	ChangingBaud
} can_status_t;
#endif //0

typedef enum {
	baud_10K = 0,
	baud_20K = 1,
	baud_50K = 2,
	baud_125K = 3,
	baud_250K = 4,
	baud_500K = 5,
	baud_800K  = 6,
	baud_1M = 7,
	no_baud = 0xff
} baud_rate_t;

typedef void (*can_status_handler)(can_status_t, baud_rate_t);

#if LOG_LEVEL < NO_LOGGING
extern char baud_rate_strings[8][10];
#endif

#define BAUD_MAX baud_1M

extern result_t can_init(baud_rate_t      baud,
	can_status_handler   status_default_handler);

//extern bool can_initialised(void);



extern result_t l2_init(baud_rate_t arg_baud_rate,
                 void (*arg_status_handler)(u8 mask, can_status_t status, baud_rate_t baud));

extern result_t l2_reg_handler(can_target_t *target);

#ifdef L2_CAN_INTERRUPT_DRIVEN
extern void L2_ISR(void);
#endif
extern void L2_CanTasks(void);

extern result_t l2_tx_frame(can_frame *message);
extern void L2_CanTxError(u8 node_type, u8 node_number, u32 errorCode);

extern baud_rate_t L2_GetCanBuadRate(void);
extern void l2_set_can_node_buadrate(baud_rate_t baudrate);
extern void L2_SetCanNetworkBuadRate(baud_rate_t);

extern void L2_getStatus(can_status_t *, baud_rate_t *);

#ifdef MCP
extern void canTasks(void);
#endif


#if defined(CAN_LAYER_3)
#define NET_LOG_L3_ID   0xc9 

extern result_t l3_init(void (*arg_status_handler)(u8 mask, can_status_t status, baud_rate_t baud));
//extern void l3_finalise_address(u8 source);
extern BOOL l3_initialised(void);
//extern result_t l3_get_ddress(u8 * address);
extern result_t l3_tx_msg(l3_can_msg_t *msg);

extern result_t l3_register_handler(u8 protocol, l3_msg_handler_t handler);
#endif

#endif // CAN_H
