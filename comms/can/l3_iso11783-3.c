/**
 *
 * \file es_lib/can/l3_can.c
 *
 * Implementation of ISO 11783-3
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "system.h"
#include "es_lib/can/es_can.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "ISO11783"

/*
 * The SYS_CAN ID as used by the 11783-3 Protocol
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
#define ISO_11783_DA_BROADCAST    0xFF

#define CAN_EDP_MASK              0x02000000
#define CAN_DP_MASK               0x01000000
#define CAN_PF_MASK               0x00FF0000
#define CAN_PS_MASK               0x0000FF00
#define CAN_DA_MASK               0x0000FF00
#define CAN_GE_MASK               0x0000FF00
#define CAN_SA_MASK               0x000000FF

#define PGN_EDP_MASK              0x020000
#define PGN_DP_MASK               0x010000
#define PGN_PF_MASK               0x00FF00
#define PGN_PS_MASK               0x0000FF

#define DEFAULT_PRIORITY_CONTROL  0x03
#define DEFAULT_PRIORITY          0x06

#define PF_PDU_2_CUTOFF           0xF0

#define DEFAULT_PRIORITY_CONTROL  0x03
#define DEFAULT_PRIORITY          0x06

#define PGN_REQUEST               0x00EA00
#define PGN_REQUEST_2             0x00C900
#define PGN_ACK                   0x00E800
#define PGN_PROPRIETARY_A         0x00EF00
#define PGN_PROPRIETARY_A2        0x01EF00
#define PGN_PROPRIETARY_B_FIRST   0x00FF00
#define PGN_PROPRIETARY_B_LAST    0x00FFFF
#define PGN_TRANSFER              0x00CA00
#define PGN_DATA_TRANSFER         0x00EB00

#define PGN_TP_CM                 0x00EC00

#define TP_CM_RTS                 16
#define TP_CM_CTS                 17
#define TP_CM_End_Of_Msg_ACK      19
#define TP_CM_BAM                 32
#define TP_CM_Connection_Abort    255

#define TIMER_Tr                  200
#define TIMER_Th                  500
#define TIMER_1                   750
#define TIMER_2                   1250
#define TIMER_3                   1250
#define TIMER_4                   1050

#define ACK                       0
#define NACK                      1
#define ACCESS_DENIED             2
#define CANNOT_RESPOND            3

static uint8_t node_address;
static uint8_t da_valid = 0x00;

typedef struct
{
    uint8_t                used;
    uint32_t               pgn;
    iso11783_msg_handler_t handler;
} iso11783_register_t;

static iso11783_register_t registered[SYS_ISO11783_REGISTER_ARRAY_SIZE];

static iso11783_msg_handler_t unhandled_handler;

static void iso11783_frame_handler(can_frame *frame);

result_t iso11783_init(uint8_t address)
{
	uint16_t loop;
	can_l2_target_t target;

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso11783_init(0x%x)\n\r", address);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	node_address = address;

	for(loop = 0; loop < SYS_ISO11783_REGISTER_ARRAY_SIZE; loop++) {
		registered[loop].used = FALSE;
		registered[loop].pgn = 0x00;
		registered[loop].handler = (iso11783_msg_handler_t)NULL;
	}

	unhandled_handler = (iso11783_msg_handler_t)NULL;

	/*
	 * Define our target for Layer 2 Frames and register it.
	 * Looking for Extended frame with EDP Bit set to Zero
	 */
	target.mask   = SYS_CAN_EFF_FLAG | CAN_EDP_MASK;
	target.filter =  SYS_CAN_EFF_FLAG;
	target.handler = iso11783_frame_handler;

	can_l2_dispatch_reg_handler(&target);

	return(SUCCESS);
}

u32 pgn_to_canid(uint8_t priority, uint32_t pgn, u8 dst)
{
//	u32 pgn;
	uint8_t  pf;
//	uint8_t  ps;
//	uint8_t  sa;
//	uint8_t  da;
//	uint8_t  loop;
	u32 canid = 0x00;

	canid = (priority & 0x07);
	canid = (canid << 2) | ((pgn & (PGN_EDP_MASK | PGN_DP_MASK)) >> 16);

	pf = (uint8_t)((pgn & PGN_PF_MASK) >> 8);
	canid = canid << 8 | pf;

	if(pf < PF_PDU_2_CUTOFF) {
		canid = canid << 8 | (pgn & PGN_PS_MASK);
	} else {
		canid = canid << 8 | dst;
	}

	canid = canid << 8 | node_address;

	return(canid);
}

u32 canid_to_pgn(u32 canid)
{
	u32 pgn = 0x00;
#if 0
	uint8_t  pf;
	uint8_t  ps;

	sa = (uint8_t)(frame->can_id & 0xff);

	/*
	 * EDP Bit is always 0! so move on to DP bit
	 */
	pgn = (uint8_t)((canid & (CAN_EDP_MASK | SYS_CAN_DP_MASK)) >> 24);

	pf = (uint8_t)((canid & SYS_CAN_PF_MASK) >> 16);

	if(pf < PF_PDU_2_CUTOFF) {
		da = (uint8_t) ((frame->can_id & PS_MASK) >> 8);
		da_valid = 0x01;
		ps = 0x00;
	} else {
		ps = (uint8_t) ((frame->can_id & PS_MASK) >> 8);
	}
	pgn = (pgn << 8) | pf;
	pgn = (pgn << 8) | ps;
#endif
	return(pgn);
}

uint8_t get_source_address_from_canid(u32 canid)
{
	return((uint8_t)(canid & SYS_CAN_SA_MASK));
}

uint8_t get_destination_address_from_canid(u32 canid)
{
	uint8_t  pf;

	pf = (uint8_t)((canid & SYS_CAN_PF_MASK) >> 16);

	if(pf < PF_PDU_2_CUTOFF) {
		return((uint8_t) ((canid & SYS_CAN_PS_MASK) >> 8));
	}
	return(0xff);
}

result_t iso11783_tx_msg(iso11783_msg_t *msg)
{
	can_frame frame;

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso11783_tx_msg()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	frame.can_id = pgn_to_canid(msg->priority, msg->pgn, msg->destination);
	frame.can_dlc = 0x00;

	can_l2_tx_frame(&frame);

	return(SUCCESS);
}

result_t iso11783_tx_pgn_request(uint8_t priority, u8 dst, u32 pgn)
{
	can_frame frame;
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso11783_tx_pgn_request()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	frame.can_id = pgn_to_canid(priority, PGN_REQUEST, dst);
	frame.can_dlc = 0x03;

	frame.data[0] = (uint8_t)(pgn & 0xff);
	frame.data[1] = (uint8_t)((pgn & 0xff00) >> 8);
	frame.data[2] = (uint8_t)((pgn & 0xff0000) >> 16);

	can_l2_tx_frame(&frame);

	return(SUCCESS);
}

result_t iso11783_tx_ack_pgn(uint8_t priority, u8 dst, u32 pgn, iso11783_ack_t ack_value)
{
	uint8_t        loop;
	u32       tmp_pgn;
	can_frame frame;
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso11783_tx_ack_pgn()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	frame.can_id = pgn_to_canid(priority, PGN_ACK, dst);
	frame.can_dlc = 0x08;

	frame.data[0] = ack_value;
	frame.data[1] = 0xff;      // Group Function Value
	frame.data[2] = 0xff;      // reserved
	frame.data[3] = 0xff;      // reserved
	frame.data[4] = dst;       // Address Acknowledged

	tmp_pgn = pgn;
	for(loop = 0; loop < 3; loop++) {
		frame.data[loop + 5] = tmp_pgn & 0xff;
		tmp_pgn = tmp_pgn >> 8;
	}

	can_l2_tx_frame(&frame);

	return(SUCCESS);
}

void iso11783_frame_handler(can_frame *frame)
{
	u32            pgn;
//	uint8_t             pf;
//	uint8_t             ps;
	uint16_t            loop;
	uint8_t             handled;
	iso11783_msg_t msg;

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso11783_frame_handler(frame id 0x%x)\n\r", frame->can_id);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	/*
	 * Parameter Group Numbers PGN:
	 *
	 * 24 bit : 000000 EDP DP PDU_Format PDU_Specific
	 *
	 * if PDU_Format < 0xF0  then LSByte of PGN = 0x00
	 */
	pgn = canid_to_pgn(frame->can_id);

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso11783_frame_handler received PGN %d  =  0x%lx\n\r", pgn, pgn);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

//	printf("iso11783:(frame id 0x%x) SA 0x%x,", frame->can_id, sa);
//	if(da_valid) {
//		printf(" DA 0x%x, ", da);
//	}
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, " PGN %d (0x%x) [", pgn, pgn);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	for(loop = 0; loop < frame->can_dlc; loop++) {
		printf("%x,", frame->data[loop]);
	}
	printf("\n\r");

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	if(pgn == 126992) {
		log_d(TAG, "System Time\n\r");
	} else if(pgn == 128267) {
		log_d(TAG, "Depth\n\r");
	} else if(pgn == 129025) {
		log_d(TAG, "Position\n\r");
	} else if(pgn == 129026) {
		log_d(TAG, "Corse Over Ground (COG) & Speed over Ground (SOG)\n\r");
	} else if(pgn == 129027) {
		log_d(TAG, "Position Delta\n\r");
	} else if(pgn == 129029) {
		log_d(TAG, "GNSS Position Data\n\r");
	} else if(pgn == 129033) {
		log_d(TAG, "Time & Date\n\r");
	} else if(pgn == 129539) {
		log_d(TAG, "GNSS and DOP Dilution Of Precision\n\r");
	} else if(pgn == 129540) {
		log_d(TAG, "Satalites in View\n\r");
	}
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	handled = 0x00;

	for (loop = 0; loop < SYS_ISO11783_REGISTER_ARRAY_SIZE; loop++) {
		if (registered[loop].used && (pgn == registered[loop].pgn)) {
			registered[loop].handler(&msg);
			handled = 1;
		}
	}

	if(!handled) {
		if(unhandled_handler) {
			unhandled_handler(&msg);
		}
	}
}

result_t iso11783_dispatch_reg_handler(iso11783_target_t *target)
{
	uint8_t loop;

	target->handler_id = 0xff;

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso11783_dispatch_register_handler(0x%lx)\n\r", target->pgn);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	/*
	 * Find a free slot and add the Protocol
	 */
	for(loop = 0; loop < SYS_ISO11783_REGISTER_ARRAY_SIZE; loop++) {
		if(registered[loop].used == FALSE) {
			registered[loop].used = TRUE;
			registered[loop].pgn = target->pgn;
			registered[loop].handler = target->handler;
			target->handler_id = loop;
			return(SUCCESS);
		}
	}

#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
	log_e(TAG, "ISO11783 Dispatch full!\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
	return(ERR_NO_RESOURCES);
}

result_t iso11783_dispatch_unreg_handler(uint8_t id)
{
	if((id < SYS_ISO11783_REGISTER_ARRAY_SIZE) && (registered[id].used)) {
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