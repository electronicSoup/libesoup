/**
 *
 * \file es_can/can/es_can.h
 *
 * Core CAN Bus definitions required by electronicSoup CAN code
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

typedef enum {
    Uninitialised,
    Listening,
    Connecting,
    Connected,
    ChangingBaud
} can_status_t;

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
 #if defined(CAN_LAYER_3)
       u8 l3_address,
#endif
	can_status_handler   status_default_handler);

//extern bool can_initialised(void);




extern result_t l2_init(baud_rate_t          baudRate,
		        can_status_handler   status_default_handler);

//extern bool l2_initialised(void);

extern result_t l2_reg_handler(can_target_t *target);

#ifdef L2_CAN_INTERRUPT_DRIVEN
extern void L2_ISR(void);
#endif
extern void L2_CanTasks(void);

extern result_t l2_tx_frame(can_frame *message);
extern void L2_CanTxError(u8 node_type, u8 node_number, u32 errorCode);

extern baud_rate_t L2_GetCanBuadRate(void);
extern void L2_SetCanNodeBuadRate(baud_rate_t);
extern void L2_SetCanNetworkBuadRate(baud_rate_t);

extern void L2_getStatus(can_status_t *, baud_rate_t *);

#ifdef MCP2515_OUTPUT_0
extern void set_output_0(u8);
#endif

#ifdef MCP2515_OUTPUT_1
extern void set_output_1(u8);
#endif

#ifdef MCP
extern void canTasks(void);
#endif


#if defined(CAN_LAYER_3)
#define NET_LOG_L3_ID   0xc9 

extern result_t l3_init(u8 address);
extern void l3_finalise_address(u8 source);
extern bool l3_initialised(void);
//extern result_t l3_get_ddress(u8 * address);
extern result_t l3_tx_msg(l3_can_msg_t *msg);

extern result_t l3_register_handler(u8 protocol, l3_msg_handler_t handler);
#endif

#endif // CAN_H
