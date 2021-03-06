/**
 *
 * @file libesoup/comms/can/dcncp/dcncp_can.h
 *
 * @author John Whitmore
 *
 * @brief Dynamic CAN Node Configuration Protocol definitions.
 * 
 * Included in the build with SYS_CAN_DCNCP
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
#ifndef _NODE_ADDRESS_H
#define _NODE_ADDRESS_H

#include "libesoup_config.h"

#ifdef SYS_CAN_DCNCP

#include "libesoup/comms/can/can.h"

/*
 * Node module will use standard CAN frames which are 11 bits for Id
 *
 * All these definitions are used in the ES Node Management Applicaiton! If
 * changed update there.
 */
#define CAN_DCNCP_MASK                          0x7f0
#define CAN_DCNCP_FILTER                        0x700

//#define CAN_DCNCP_NodePingMessage               0x6ff

#define CAN_DCNCP_AddressRegisterReq            0x700

#define CAN_DCNCP_AddressRegisterReject         0x701
#define CAN_DCNCP_NodeAddressError              0x702
#define CAN_DCNCP_NodeAddressReportReq          0x703
#define CAN_DCNCP_NodeAddressReporting          0x704

#define CAN_DCNCP_NetworkChangeBaudRateReq      0x705

#define CAN_DCNCP_RegisterNetLogger             0x706
#define CAN_DCNCP_UnRegisterNetLogger           0x707

/*
 * CAN Bus DCNCP Protocol Status numbers
 */
enum can_dcncp_status {
    can_dcncp_l3_address_registered,
};

extern result_t dcncp_init(status_handler_t arg_status_handler, uint8_t l3_address);
extern void dcncp_request_network_baud_change(can_baud_rate_t baud);
extern void dcncp_send_ping(void);
#if defined(ISO15765) || defined(ISO11783)
extern uint8_t dcncp_get_node_address(void);
#endif //ISO15765 || SYS_ISO11783

#if defined(ISO15765)
extern uint8_t node_get_net_logger_address(void);
#ifdef SYS_ISO15765_LOGGER
extern result_t dcncp_register_this_node_net_logger(log_level_t level);
extern result_t dcncp_unregister_this_node_net_logger(void);
#endif // SYS_ISO15765_LOGGER
#endif

#endif // #ifdef SYS_CAN_DCNCP

#endif // _NODE_ADDRESS_H
