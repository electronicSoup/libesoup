/**
 *
 * @file libesoup/comms/can/l2_lx_can.c
 *
 * @author John Whitmore
 * 
 * @brief CAN L2 Functionality for RPi
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
//#include "os_api.h"

#include "libesoup_config.h"

#ifdef ES_LINUX

//#include "logging.h"

#include <string.h>
#include <pthread.h>

#include <errno.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
 
#include <linux/can.h>
#include <linux/can/raw.h>

#include "libesoup/can/es_can.h"
#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
#include "libesoup/logger/serial_log.h"
static const char *TAG = "CAN_L2_LX";
#endif // SYS_SERIAL_LOGGING

#define REGISTER_ARRAY_SIZE 5

typedef struct
{
    struct {
        uint8_t used : 1;
        uint8_t system:1;
    } bitField;

    can_l2_target_t target;
} canRegister;

static canRegister registered[REGISTER_ARRAY_SIZE];

void *create_read_thread(void *);

static int                can_socket;
static can_status_t       can_status;
void (*can_status_handler)(uint8_t mask, can_status_t status, can_baud_rate_t baud);
static can_baud_rate_t    can_baud_rate;

result_t can_l2_init(can_baud_rate_t arg_baud_rate,
                 void (*arg_status_handler)(uint8_t mask, can_status_t status, can_baud_rate_t baud))
{
	uint8_t loop;
	int result;
	pthread_t thread_id;
	struct ifreq ifr;
	struct sockaddr_can addr;

        can_status_handler = arg_status_handler;
	can_status.byte = 0x00;

	for(loop = 0; loop < REGISTER_ARRAY_SIZE; loop++) {
		registered[loop].bitField.used = FALSE;
		registered[loop].bitField.system = FALSE;
		registered[loop].target.mask = 0x00;
		registered[loop].target.filter = 0x00;
		registered[loop].target.handler = (can_l2_frame_handler_t)NULL;
	}

	char *ifname = SYS_CAN_INTERFACE;

	LOG_D("l2_can_init(%s)\n\r", SYS_CAN_INTERFACE);
 
	if((can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		LOG_E("Error while opening socket\n\r");
		return(ERR_GENERAL_ERROR);
	}

	strcpy(ifr.ifr_name, ifname);
	ioctl(can_socket, SIOCGIFINDEX, &ifr);
 
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex; 
 
	if(bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		LOG_D("Error binding socket\n\r");
		return(ERR_GENERAL_ERROR);
	}

	result = pthread_create(&thread_id, NULL, create_read_thread, (void *)can_socket);
	if(result != 0){
		LOG_E("Failed to create read thead\n\r");
		return(ERR_GENERAL_ERROR);
	}
	LOG_D("New Thread created\n\r");

	pthread_detach(thread_id);
	sched_yield();

	can_baud_rate = arg_baud_rate;
	can_status.bit_field.l2_status = L2_Connected;

	if(can_status_handler)
		can_status_handler(L2_STATUS_MASK, can_status, can_baud_rate);

	return(SUCCESS);
}

result_t can_l2_tx_frame(can_frame *frame)
{
	int nbytes;

	LOG_D("L2_CanTxMessage(0x%x)\n\r", frame->can_id);
	if(can_status.bit_field.l2_status != L2_Connected)
		return(ERR_CAN_NOT_CONNECTED);

	nbytes = write(can_socket, frame, sizeof(can_frame));
 
	if(nbytes < 0) {
		printf("write : Failed\n\r");
		printf("Error no is : %d\n\r", errno);
		printf("Error description is : %s\n\r",strerror(errno));
	}

	if(nbytes != sizeof(can_frame)){
		LOG_E("Wrote %d bytes! Expected %d\n\r", nbytes, sizeof(can_frame));
		return(ERR_GENERAL_ERROR);
	}
	
	return (SUCCESS);
}

result_t can_l2_dispatch_reg_handler(can_l2_target_t *target)
{
	int loop;

	LOG_D("sys_l2_can_dispatch_reg_handler() Mask 0x%x, Filter 0x%x\n\r", target->mask, target->filter);

	/*
	 * clean up the target in case the caller has included spurious bits
	 */
	if(target->mask & CAN_EFF_FLAG)
		target->mask = target->mask & (CAN_EFF_FLAG | CAN_EFF_MASK);
	else
		target->mask = target->mask & CAN_SFF_MASK;

	for(loop = 0; loop < REGISTER_ARRAY_SIZE; loop++) {
		if(registered[loop].bitField.used == FALSE) {
			LOG_D("Target stored at target %d\n\r", loop);
			registered[loop].bitField.used = TRUE;
			registered[loop].bitField.system = FALSE;
			registered[loop].target.mask = target->mask;
			registered[loop].target.filter = target->filter;
			registered[loop].target.handler = target->handler;
//			*id = loop;
			return(SUCCESS);
		}
	}

	return(ERR_NO_RESOURCES);
}

void *create_read_thread(void *arg)
{
	can_frame frame;
	int nbytes;
	int s;
	uint8_t loop;

	s = (int)arg;
  
        while(1) {
		nbytes = read(s, &frame, sizeof(can_frame));

		if (nbytes < 0) {
			LOG_E("thread error in size of read data\n\r");
			return((void *)NULL);
    		}

    		/* paranoid check ... */
    		if (nbytes < (int)sizeof(can_frame)) {
			LOG_E("thread error in size of read data\n\r");
			return((void *)NULL);
    		}

		for(loop = 0; loop < REGISTER_ARRAY_SIZE; loop++) {
			if(  (registered[loop].bitField.used) 
			     &&((frame.can_id & registered[loop].target.mask) == (registered[loop].target.filter & registered[loop].target.mask)))
				registered[loop].target.handler(&frame);
		}
	}
	return((void *)NULL);
}

#endif // ES_LINUX
