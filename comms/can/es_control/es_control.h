/**
 * @file libesoup/comms/can/es_control/es_control.h
 *
 * @author John Whitmore
 * 
 * @brief Definitions required to use the es_control protocol
 *
 * Copyright 2018 electronicSoup
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 3 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ES_CONTROL_H
#define __ES_CONTROL_H

union es_control_id {
        struct {
                uint8_t   es_type;
                uint8_t   rtr:1;
                uint8_t   priority:2;
                uint8_t   :5;
        } fields;
        uint16_t  word;
};

#define  ESC_RTR_MASK                 0x0100
#define  ESC_TYPE_MASK                0x00ff

#define  ESC_PRIORITY_0               0x00   // Highest Priority
#define  ESC_PRIORITY_1               0x01
#define  ESC_PRIORITY_2               0x02
#define  ESC_PRIORITY_3               0x03   // Lowest Priority

#define  ESC_BOOL_431_OUTPUT          0x10
#define  ESC_BOOL_431_INPUT           0x11
#define  ESC_PING_PROTOCOL            0xff
/*
 * Hardcode the ping frame ID to be lowest priority, no RTR and the lowest 
 * priority es_type 0xff
 */
#define  ESC_PING_PROTOCOL_ID         0x6ff

union bool_431 {
	struct {
		uint8_t   es_bool:1;
		uint8_t   chan:3;
		uint8_t   node:4;
	} bitfield;
	uint8_t byte;
};

#endif // __ES_CONTROL_H
