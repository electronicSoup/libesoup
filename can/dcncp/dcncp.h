/**
 *
 * \file es_lib/can/dcncp/dcncp.h
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

/*
 * Node module will use standard CAN frames which are 11 bits for Id
 */
#define CAN_DCNCP_MASK                          0x7f0
#define CAN_DCNCP_FILTER                        0x700

#define CAN_DCNCP_AddressRegisterReq            0x700

#define CAN_DCNCP_AddressRegisterReject         0x701
#define CAN_DCNCP_NodeAddressError              0x702
#define CAN_DCNCP_NodeAddressReportReq          0x703
#define CAN_DCNCP_NodeAddressReporting          0x704

#define CAN_DCNCP_NetworkChangeBaudRateReq      0x705
#define CAN_DCNCP_NodePingMessage               0x706

#define CAN_DCNCP_NetLogger                     0x707
#define CAN_DCNCP_CancelNetLogger               0x708

extern void dcncp_init(void (*arg_status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud));
extern void dcncp_request_network_baud_change(can_baud_rate_t baud);
extern void dcncp_send_ping(void);
#if defined(CAN_LAYER_3)
extern u8 dcncp_get_can_l3_address(void);
extern u8 node_get_net_logger_address(void);
extern result_t register_this_node_net_logger(log_level_t level);
extern result_t unregister_this_node_net_logger(void);
#endif

#endif //NODE_H
