/**
 *
 * \file libesoup/comms/can/frame_dispatch.c
 *
 * CAN L2 Functionality for dispatching received frames
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
#include "libesoup_config.h"

#ifdef SYS_CAN_BUS

/*
 * Check required libesoup_config.h defines are found
 */
#ifdef SYS_SERIAL_LOGGING
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#ifndef SYS_CAN_FRAME_HANDLER_ARRAY_SIZE
#error libesoup_config.h file should define SYS_CAN_FRAME_HANDLER_ARRAY_SIZE (see libesoup/examples/libesoup_config.h)
#endif


#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
#include "libesoup/logger/serial_log.h"

static const char *TAG = "CAN_DISPATCH";
#endif // SYS_SERIAL_LOGGING

#include "libesoup/comms/can/can.h"

typedef struct
{
	uint8_t used;
	can_l2_target_t target;
} can_register_t;

static can_register_t registered_handlers[SYS_CAN_FRAME_HANDLER_ARRAY_SIZE];
static can_l2_frame_handler_t unhandled_handler;

void frame_dispatch_init(void)
{
	uint16_t loop;
	
	unhandled_handler = (can_l2_frame_handler_t)NULL;

        /*
         * Initialise the Handlers table
         */
        for(loop = 0; loop < SYS_CAN_FRAME_HANDLER_ARRAY_SIZE; loop++) {
		registered_handlers[loop].used = FALSE;
		registered_handlers[loop].target.mask = 0x00;
		registered_handlers[loop].target.filter = 0x00;
		registered_handlers[loop].target.handler = (can_l2_frame_handler_t)NULL;
	}
}

result_t frame_dispatch_reg_handler(can_l2_target_t *target)
{
	uint8_t loop;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("sys_l2_can_dispatch_reg_handler mask 0x%lx, filter 0x%lx\n\r",
		target->mask, target->filter);
#endif
	/*
	 * clean up the target in case the caller has included spurious bits
	 */
	if(target->mask & CAN_EFF_FLAG) {
		target->mask = target->mask & (CAN_EFF_FLAG | CAN_EFF_MASK);
	} else {
		target->mask = target->mask & CAN_SFF_MASK;
	}

	// Find a free slot
	for(loop = 0; loop < SYS_CAN_L2_HANDLER_ARRAY_SIZE; loop++) {
		if(registered_handlers[loop].used == FALSE) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
			LOG_I("Target stored at target %d\n\r", loop);
#endif
			registered_handlers[loop].used = TRUE;
			registered_handlers[loop].target.mask = target->mask;
			registered_handlers[loop].target.filter = target->filter;
			registered_handlers[loop].target.handler = target->handler;
			target->handler_id = loop;
			return(SUCCESS);
		}
	}
	return(ERR_NO_RESOURCES);
}

result_t frame_dispatch_unreg_handler(uint8_t id)
{
	if(id < SYS_CAN_L2_HANDLER_ARRAY_SIZE) {
		if (registered_handlers[id].used) {
			registered_handlers[id].used = FALSE;
			registered_handlers[id].target.mask = 0x00;
			registered_handlers[id].target.filter = 0x00;
			registered_handlers[id].target.handler = (void (*)(can_frame *))NULL;
			return (SUCCESS);
		}
	}
	return(ERR_CAN_ERROR);
}

result_t frame_dispatch_set_unhandled_handler(can_l2_frame_handler_t handler)
{
	unhandled_handler = (can_l2_frame_handler_t)handler;
	return(SUCCESS);
}

void frame_dispatch_handle_frame(can_frame *frame)
{
	uint8_t loop;
	boolean found = FALSE;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("frame_dispatch_handle_frame(%lx)\n\r", frame->can_id);
#endif
//	printf("L2_CanDispatcherL2MsgHandler 0x%lx [", frame->can_id);
//	for(loop = 0; loop < frame->can_dlc; loop++) {
//		printf("0x%2x,", frame->data[loop]);
//	}
//	printf("]\n\r");

	for (loop = 0; loop < SYS_CAN_L2_HANDLER_ARRAY_SIZE; loop++) {
		if(registered_handlers[loop].used) {
			if ((frame->can_id & registered_handlers[loop].target.mask) == (registered_handlers[loop].target.filter & registered_handlers[loop].target.mask)) {
				registered_handlers[loop].target.handler(frame);
				found = TRUE;
			}
		}
	}

	if(!found) {
		/*
		 * No handler found so pass the received message to the Application
		 */
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("No Handler for 0x%lx\n\r", frame->can_id);
#endif
	}
}

#endif // SYS_CAN_BUS
