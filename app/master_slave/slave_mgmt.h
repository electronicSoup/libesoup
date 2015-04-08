/**
 *
 * \file es_can/app/master_slave/slave_mgmt.h
 *
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
#ifndef SLAVE_MGMT_H
#define SLAVE_MGMT_H

#define SLAVE_MGMT_CAN_BASE_ID   0x710

typedef enum slave_mgmt_ids {
	SlavePing = 0,
	SlaveAck
} slave_mgmt_ids;


#define DIGITAL_OUTPUT_SLAVE        0x01
#define DIGITAL_INPUT_SLAVE         0x02
#define PERCENTAGE_OUTPUT_SLAVE     0x04
#define PERCENTAGE_INPUT_SLAVE      0x08

typedef struct
#if defined(__C30__) || defined(__XC16__)
__attribute__ ((packed))
#endif
{
    u16 node_type;
    u8  node_number;
} can_node_id_t;

typedef struct node_status_t
{
    can_node_id_t node_id;
    u8            idleCount;
} node_status_t;

#endif // SLAVE_MGMT_H
