/**
 *
 * \file es_lib/app/os.c
 *
 * OS initialise routine
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
#include "es_lib/core.h"
//#include "es_lib/app/os.h"

#define OS_FNS 0x8A92

result_t (*timer_start)(u16, expiry_function, union sigval, es_timer *);
result_t (*timer_cancel)(es_timer *timer);

result_t (*eeprom_read)(u16 address, u8 *data);
result_t (*eeprom_write)(u16 address, u8 data);

result_t (*can_l2_tx_frame)(can_frame *);
result_t (*can_l2_dispatch_reg_handler)(can_l2_target_t *target);
result_t (*can_l2_dispatch_unreg_handler)(u8 id);
result_t (*can_l2_dispatch_set_unhandled_handler)(can_l2_frame_handler_t handler);

u8       (*dcncp_get_node_address)(void);

result_t (*iso15765_tx_msg)(iso15765_msg_t *msg);
result_t (*iso15765_dispatch_reg_handler)(iso15765_target_t * target);
result_t (*iso15765_dispatch_unreg_handler)(u8 id);
result_t (*iso15765_dispatch_set_unhandled_handler)(iso15765_msg_handler_t handler);

result_t (*iso11783_tx_msg)(iso11783_msg_t *msg);
result_t (*iso11783_dispatch_reg_handler)(iso11783_target_t *target);
result_t (*iso11783_dispatch_unreg_handler)(u8 id);
result_t (*iso11783_dispatch_set_unhandled_handler)(iso11783_msg_handler_t handler);

result_t (*serial_log)(log_level_t level, char *tag, char *fmt, ...);
void     (*iso15765_log)(log_level_t level, char *msg);

result_t (*get_io_address)(u8 *);
result_t  (*flash_strcpy)(char *dst, __prog__ char *src, u16 *length);

void _ISR __attribute__((__no_auto_psv__)) _DefaultInterrupt(void)
{
	/*
	 * Any unexpected interrupt will reset the processor on a Watch Dog Reset
	 */
	while(1){
	}
}

void os_init(void)
{
	timer_start = (result_t(*)(u16, expiry_function, union sigval data, es_timer *))OS_FNS;
	timer_cancel = (result_t(*)(es_timer *))(OS_FNS + 4);

	eeprom_read = (result_t(*)(u16, u8 *))(OS_FNS + 8);
	eeprom_write = (result_t(*)(u16, u8))(OS_FNS + 12);

	can_l2_tx_frame = (result_t(*)(can_frame *))(OS_FNS + 16);
	can_l2_dispatch_reg_handler = (result_t(*)(can_l2_target_t *))(OS_FNS + 20);
	can_l2_dispatch_unreg_handler = (result_t(*)(u8))(OS_FNS + 24);
	can_l2_dispatch_set_unhandled_handler = (result_t(*)(can_l2_frame_handler_t handler))(OS_FNS + 28);

	dcncp_get_node_address = (u8 (*)(void))(OS_FNS + 32);

	iso15765_tx_msg = (result_t(*)(iso15765_msg_t *))(OS_FNS + 36);
	iso15765_dispatch_reg_handler = (result_t(*)(iso15765_target_t * target))(OS_FNS + 40);
	iso15765_dispatch_unreg_handler = (result_t(*)(u8))(OS_FNS + 44);
	iso15765_dispatch_set_unhandled_handler = (result_t (*)(iso15765_msg_handler_t handler))(OS_FNS + 48);

	iso11783_tx_msg = (result_t (*)(iso11783_msg_t *msg))(OS_FNS + 52);
	iso11783_dispatch_reg_handler = (result_t (*)(iso11783_target_t *target))(OS_FNS + 56);
	iso11783_dispatch_unreg_handler = (result_t (*)(u8 id))(OS_FNS + 60);
	iso11783_dispatch_set_unhandled_handler = (result_t (*)(iso11783_msg_handler_t handler))(OS_FNS + 64);

	serial_log = (result_t (*)(log_level_t, char *, char *, ...))(OS_FNS + 68);
	iso15765_log = (void (*)(log_level_t, char *))(OS_FNS + 72);

	get_io_address = (result_t (*)(u8 *))(OS_FNS + 76);
	flash_strcpy = (result_t (*)(char *dst, __prog__ char *src, u16 *length))(OS_FNS + 80);
}
