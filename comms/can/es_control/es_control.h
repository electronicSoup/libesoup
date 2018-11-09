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

union es_control_id {
        struct {
                uint8_t   es_type;
                uint8_t   rtr:1;
                uint8_t   priority:2;
                uint8_t   :5;
        } fields;
        uint16_t  word;
};

const uint16_t es_rtr_mask  = 0x0100;
const uint16_t es_type_mask = 0x00ff;

const uint8_t priority_0 = 0x00;   // Highest Priority
const uint8_t priority_1 = 0x01;
const uint8_t priority_2 = 0x02;
const uint8_t priority_3 = 0x03;   // Lowest Priority

const uint8_t bool_431_output = 0x10;
const uint8_t bool_431_input  = 0x11;

union bool_431 {
	struct {
		uint8_t   es_bool:1;
		uint8_t   chan:3;
		uint8_t   node:4;
	} bitfield;
	uint8_t byte;
};

