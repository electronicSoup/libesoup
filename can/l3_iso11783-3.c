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

#define EDP_MASK  0x02000000
#define DP_MASK   0x01000000
#define PF_MASK   0x00FF0000
#define PS_MASK   0x0000FF00
#define DA_MASK   0x0000FF00
#define GE_MASK   0x0000FF00
#define SA_MASK   0x000000FF

#define PF_PDU_2_CUTOFF 0xF0

#define PGN_REQUEST               0x00EA00
#define PGN_REQUEST_2             0x00C900
#define PGN_ACK                   0x00E800
#define PGN_PROPRIETARY_A         0x00EF00
#define PGN_PROPRIETARY_A2        0x01EF00
#define PGN_PROPRIETARY_B_FIRST   0x00FF00
#define PGN_PROPRIETARY_B_LAST    0x00FFFF
#define PGN_TRANSFER              0x00CA00

#if 0
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
#endif

static u8 node_address;

typedef struct
{
    u8 used;
    u32 pgn;
    iso11783_msg_handler_t handler;
} iso11783_register_t;

static iso11783_register_t registered[ISO11783_REGISTER_ARRAY_SIZE];

static iso11783_msg_handler_t unhandled_handler;

static void iso11783_frame_handler(can_frame *frame);

result_t iso11783_init(u8 address)
{
	u16 loop;
	can_l2_target_t target;

	LOG_D("iso11783_init(0x%x)\n\r", address);

	node_address = address;

	for(loop = 0; loop < ISO11783_REGISTER_ARRAY_SIZE; loop++) {
		registered[loop].used = FALSE;
		registered[loop].pgn = 0x00;
		registered[loop].handler = (iso11783_msg_handler_t)NULL;
	}

	unhandled_handler = (iso11783_msg_handler_t)NULL;
	
	/*
	 * Define our target for Layer 2 Frames and register it.
	 * Looking for Extended frame with EDP Bit set to zero
	 */
	target.mask   = CAN_EFF_FLAG | EDP_MASK;
	target.filter =  CAN_EFF_FLAG;
	target.handler = iso11783_frame_handler;

	can_l2_dispatch_reg_handler(&target);

	return(SUCCESS);
}

result_t iso11783_tx_msg(iso11783_msg_t *msg)
{
	LOG_D("iso11783_tx_msg()\n\r");
	return(SUCCESS);
}

void iso11783_frame_handler(can_frame *frame)
{
	u32            pgn;
	u8             pf;
	u8             ps;
	u16            loop;
	u8             handled;
	iso11783_msg_t msg;

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

	handled = 0x00;

	for (loop = 0; loop < ISO11783_REGISTER_ARRAY_SIZE; loop++) {
		if (registered[loop].used && (pgn == registered[loop].pgn)) {
			registered[loop].handler(&msg);
			handled = 1;
		}
	}

	if(!handled) {
		unhandled_handler(&msg);
	}
}

result_t iso11783_dispatch_reg_handler(iso11783_target_t *target)
{
	u8 loop;

	target->handler_id = 0xff;

	LOG_D("iso11783_dispatch_register_handler(0x%lx)\n\r", target->pgn);

	/*
	 * Find a free slot and add the Protocol
	 */
	for(loop = 0; loop < ISO11783_REGISTER_ARRAY_SIZE; loop++) {
		if(registered[loop].used == FALSE) {
			registered[loop].used = TRUE;
			registered[loop].pgn = target->pgn;
			registered[loop].handler = target->handler;
			target->handler_id = loop;
			return(SUCCESS);
		}
	}

	LOG_E("ISO11783 Dispatch full!\n\r");
	return(ERR_NO_RESOURCES);
}

result_t iso11783_dispatch_unreg_handler(u8 id)
{
	if((id < ISO11783_REGISTER_ARRAY_SIZE) && (registered[id].used)) {
		registered[id].used = FALSE;
		registered[id].pgn = 0x00;
		registered[id].handler = (iso11783_msg_handler_t)NULL;
		return(SUCCESS);
	}
	return(ERR_BAD_INPUT_PARAMETER);
}

result_t iso11783_dispatch_set_unhandled_handler(iso11783_msg_handler_t handler)
{
	unhandled_handler = (iso11783_msg_handler_t)handler;
	return(SUCCESS);
}
