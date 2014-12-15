/**
 *
 * \file es_lib/dcncp/dcncp.h
 *
 * Dynamic CAN Node Configuration Protocol  definitions
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
#ifndef NODE_ADDRESS_H
#define NODE_ADDRESS_H

#include "es_lib/can/es_can.h"

// Node module will use standard CAN frames which are 11 bits for Id
//
// Mask 0x7f0        111 1111 0000
// Filter 0x700

#define NODE_MASK 0x7f0
#define NODE_FILTER 0x700

#define AddressRegisterReq           0x700

#define AddressRegisterReject       0x701
#define NodeAddressError             0x702
#define NodeAddressReportReq     0x703
#define NodeAddressReporting      0x704

#define NodeSetBaudRate      0x705

#define NodePingMessage      0x706

#define NetLogger  0x707
#define CancelNetLogger  0x708

extern void dcncp_init(void (*arg_status_handler)(u8 mask, can_status_t status, baud_rate_t baud));
extern void send_ping_message(void);
#if defined(CAN_LAYER_3)
extern u8 node_get_net_logger_address(void);
extern result_t register_this_node_net_logger(log_level_t level);
extern result_t unregister_this_node_net_logger(void);
#endif

#endif //NODE_H
