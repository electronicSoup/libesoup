/**
 * @file libesoup/examples/main_can_barebones.c
 *
 * @author John Whitmore
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
 *
 */
//#define TX_NODE

#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "Main";
#include "libesoup/logger/serial_log.h"
#endif // SYS_SERIAL_LOGGING

#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/change_notification.h"
#include "libesoup/comms/can/can.h"

void rd2_change(enum gpio_pin pin)
{
	result_t  rc;
	can_frame frame;

	frame.can_id = 0x777;
	frame.can_dlc = 0x00;
	rc = can_l2_tx_frame(&frame);
	RC_CHECK_STOP
}

static void frame_handler(can_frame *frame)
{
	result_t rc;
	LOG_D("handle(0x%lx)\n\r", frame->can_id);

	rc = gpio_toggle_output(RD0);
}

int main(void)
{
	result_t         rc = 0;
	can_l2_target_t  target;

	rc = libesoup_init();
	RC_CHECK_STOP

	rc = gpio_set(RD0, GPIO_MODE_DIGITAL_OUTPUT, 0x00);
	RC_CHECK_STOP
	rc = gpio_set(RD1, GPIO_MODE_DIGITAL_OUTPUT, 0x00);
	RC_CHECK_STOP
	rc = gpio_set(RD2, GPIO_MODE_DIGITAL_INPUT, 0x00);
	RC_CHECK_STOP
	rc = change_notifier_register(RD2, rd2_change);
	RC_CHECK_STOP
	
 	rc = can_init(baud_250K, NULL, normal);
	RC_CHECK_STOP

	/*
	 * Register a frame handler
	 */
	target.filter  = 0x777;
	target.mask    = CAN_SFF_MASK;
	target.handler = frame_handler;

	rc = frame_dispatch_reg_handler(&target);
	RC_CHECK_STOP

	LOG_D("Entering the main loop\n\r");
	while(TRUE) {
		libesoup_tasks();
	}
}
