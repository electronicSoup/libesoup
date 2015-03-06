/**
 *
 * \file es_lib/can/l3_can.c
 *
 * Implementation of ISO 11783-3
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
#include "system.h"
#include "es_lib/can/es_can.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "ISO11783"

/*
 * The CAN ID as used by the 11783-3 Protocol
 *
 *   28..26    25   24  |   23..16   |     15..8    |      7..0      |
 *   Priority  EDP  DP  | PDU Format | PDU Specific | Source Address |
 *
 *              0   0  => ISO-11783 page 0 PGN
 *              0   1  => ISO-11783 page 1 PGN
 *            [ 1   1  => ISO-15765-3 ]
 *
 *  PDU Format | PF (PDU Format) |  PS (PDU Specific)
 * ----------------------------------------------------
 *    PDU_1    |      0 - 239    |   DA Destination Address
 *    PDU_2    |    240 - 255    |   GE Group Extension
 *
 * EDP = Extended Data Page Bit
 * DP  = Data Page Bit
 *
 * Parameter Group Numbers PGN:
 *
 * 24 bit : 000000 EDP DP PDU_Format PDU_Specific
 *
 * if PDU_Format < 0xF0  then LSByte of PGN = 0x00
 *
 * Destination Address DA 0xFF is a Broadcast Address
 */
#define ISO_11783_DA_BROADCAST 0xFF

#define EDP_MASK 0x02000000
#define DP_MASK  0x01000000
#define PF_MASK  0x00FF0000
#define PS_MASK  0x0000FF00
#define DA_MASK  0x0000FF00
#define GE_MASK  0x0000FF00
#define SA_MASK  0x000000FF

#define PF_PDU_2_CUTOFF 0xF0

#define PGN_REQUEST               0x00EA00
#define PGN_REQUEST_2             0x00C900
#define PGN_ACK                   0x00E800
#define PGN_PROPRIETARY_A         0x00EF00
#define PGN_PROPRIETARY_A2        0x01EF00
#define PGN_PROPRIETARY_B_FIRST   0x00FF00
#define PGN_PROPRIETARY_B_LAST    0x00FFFF
#define PGN_TRANSFER              0x00CA00

typedef union
{
    struct
    {
        u8 source;
        u8 destination;
        u8 type;
        u8 layer3;
    } bytes;
    u32 can_id;
} iso11783_can_id;

static u8 node_address;

static void iso11783_frame_handler(can_frame *frame);

result_t iso11783_init(u8 address)
{
	can_l2_target_t target;

	LOG_D("iso11783_init(0x%x)\n\r", address);

	node_address = address;
	
	/*
	 * Define our target for Layer 2 Frames and register it.
	 * Looking for Extended frame with EDP Bit set to zero
	 */
	target.mask   = CAN_EFF_FLAG | EDP_MASK;
	target.filter =  CAN_EFF_FLAG;
	target.handler = iso11783_frame_handler;

	can_l2_reg_handler(&target);

	return(SUCCESS);
}

void iso11783_frame_handler(can_frame *frame)
{
	u32 pgn;
	u8  pf;
	u8  ps;

	LOG_D("iso11783_frame_handler(frame id 0x%x)\n\r", frame->can_id);

	/*
	 * Parameter Group Numbers PGN:
	 *
	 * 24 bit : 000000 EDP DP PDU_Format PDU_Specific
	 *
	 * if PDU_Format < 0xF0  then LSByte of PGN = 0x00
	 */
	pgn = 0x00;

	/*
	 * EDP Bit is always 0! so move on to DP bit
	 */
	if(frame->can_id & DP_MASK) {
		pgn = 0x01;
	}

	pf = (u8)((frame->can_id & PF_MASK) >> 16);

	if(pf < PF_PDU_2_CUTOFF) {
		ps = (u8) ((frame->can_id & PS_MASK) >> 8);
	} else {
		ps = 0x00;
	}
	pgn = (pgn << 8) | pf;
	pgn = (pgn << 8) | ps;

	LOG_D("iso11783_frame_handler received PGN %d  =  0x%x\n\r", pgn, pgn);

	if(pgn == 128267) {
		LOG_D("Depth\n\r");
	} else if(pgn == 129025) {
		LOG_D("Position\n\r");
	} else if(pgn == 129027) {
		LOG_D("Position Delta\n\r");
	} else if(pgn == 129029) {
		LOG_D("GNSS Position Data\n\r");
	} else if(pgn == 129033) {
		LOG_D("Time & Date\n\r");
	}
}
