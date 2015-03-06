/**
 *
 * \file es_lib/os/os.c
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

result_t (*timer_start)(UINT16, expiry_function, union sigval, es_timer *);
result_t (*timer_cancel)(es_timer *timer);

result_t (*eeprom_read)(UINT16 address, BYTE *data);
result_t (*eeprom_write)(UINT16 address, BYTE data);

result_t (*can_l2_tx_frame)(can_frame *);
result_t (*can_l2_dispatch_reg_handler)(can_l2_target_t *target);
result_t (*can_l2_dispatch_unreg_handler)(u8 id);

result_t (*get_node_address)(u8 *address);
result_t (*iso15765_tx_message)(iso15765_msg_t *msg);
result_t (*iso15765_dispatch_reg_handler)(iso15765_target_t * target);
result_t (*iso15765_dispatch_unreg_handler)(u8 id);

void (*serial_log)(log_level_t level, char *tag, char *fmt, ...);

void (*iso15765_log)(log_level_t level, char *msg);

result_t (*invalidate_app)(void);
result_t (*get_io_address)(u8 *);

void _ISR __attribute__((__no_auto_psv__)) _DefaultInterrupt(void)
{
	invalidate_app();
}

void os_init(void)
{
	timer_start = (result_t(*)(UINT16, expiry_function, union sigval data, es_timer *))OS_FNS;
	timer_cancel = (result_t(*)(es_timer *))(OS_FNS + 4);

	eeprom_read = (result_t(*)(UINT16, BYTE *))(OS_FNS + 8);
	eeprom_write = (result_t(*)(UINT16, BYTE))(OS_FNS + 12);

	can_l2_tx_frame = (result_t(*)(can_frame *))(OS_FNS + 16);
	can_l2_dispatch_reg_handler = (result_t(*)(can_l2_target_t *))(OS_FNS + 20);
	can_l2_dispatch_unreg_handler = (result_t(*)(u8))(OS_FNS + 24);

	get_node_address = (result_t(*)(BYTE *))(OS_FNS + 28);
	iso15765_tx_message = (result_t(*)(iso15765_msg_t *))(OS_FNS + 32);
	iso15765_dispatch_reg_handler = (result_t(*)(iso15765_target_t * target))(OS_FNS + 36);
	iso15765_dispatch_unreg_handler = (result_t(*)(u8))(OS_FNS + 40);

	serial_log = (void (*)(log_level_t, char *, char *, ...))(OS_FNS + 44);
    
	iso15765_log = (void (*)(log_level_t, char *))(OS_FNS + 48);
	invalidate_app = (result_t (*)(void))(OS_FNS + 52);

	get_io_address = (result_t (*)(u8 *))(OS_FNS + 56);
}
