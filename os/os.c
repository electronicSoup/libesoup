/**
 *
 * \file es_can/os/os.c
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
#include "es_lib/os/os.h"

#define OS_FNS 0x7E92

result_t (*os_start_timer)(UINT16, expiry_function, BYTE *, es_timer *timer);
result_t (*os_cancel_timer)(es_timer *timer);

result_t (*os_eeprom_read)(UINT16 address, BYTE *data);
result_t (*os_eeprom_write)(UINT16 address, BYTE data);

result_t (*os_l2_can_tx_msg)(can_frame *);
result_t (*os_l2_can_dispatch_register_handler)(can_target_t *target, BYTE *id);
result_t (*os_l2_can_dispatch_unregister_handler)(BYTE id);

result_t (*os_l3_get_address)(BYTE *address);
result_t (*os_l3_can_tx_message)(l3_can_msg_t *msg);
result_t (*os_l3_can_dispatch_register_handler)(BYTE protocol, l3_msg_handler_t handler, BYTE *id);
result_t (*os_l3_can_dispatch_unregister_handler)(BYTE id);

void (*os_serial_log)(log_level_t level, char *tag, char *fmt, ...);

void (*os_net_log)(log_level_t level, char *msg);
result_t (*os_net_log_register_as_handler)(void (*)(log_level_t, char *), log_level_t);
result_t (*os_net_log_unregister_as_handler)(void);

void (*os_invalidate_app)(void);

void _ISR __attribute__((__no_auto_psv__)) _DefaultInterrupt(void)
{
    os_invalidate_app();
}

void os_init(void)
{
    os_start_timer = (result_t(*)(UINT16, expiry_function, BYTE *, es_timer *))OS_FNS;
    os_cancel_timer = (result_t(*)(es_timer *))(OS_FNS + 4);

    os_eeprom_read = (result_t(*)(UINT16 address, BYTE * data))(OS_FNS + 8);
    os_eeprom_write = (result_t(*)(UINT16 address, BYTE data))(OS_FNS + 12);

    os_l2_can_tx_msg = (result_t(*)(can_frame *))(OS_FNS + 16);
    os_l2_can_dispatch_register_handler = (result_t(*)(can_target_t *target, BYTE * id))(OS_FNS + 20);
    os_l2_can_dispatch_unregister_handler = (result_t(*)(BYTE id))(OS_FNS + 24);

    os_l3_get_address = (result_t(*)(BYTE * address))(OS_FNS + 28);
    os_l3_can_tx_message = (result_t(*)(l3_can_msg_t * msg))(OS_FNS + 32);
    os_l3_can_dispatch_register_handler = (result_t(*)(BYTE protocol, l3_msg_handler_t handler, BYTE * id))(OS_FNS + 36);
    os_l3_can_dispatch_unregister_handler = (result_t(*)(BYTE id))(OS_FNS + 40);

    os_serial_log = (void (*)(log_level_t, char *, char *, ...))(OS_FNS + 44);

    os_net_log = (void (*)(log_level_t level, char *msg))(OS_FNS + 48);
    os_net_log_register_as_handler = (result_t(*)(void (*)(log_level_t, char *), log_level_t))(OS_FNS + 52);
    os_net_log_unregister_as_handler = (result_t(*)(void))(OS_FNS + 56);
    os_invalidate_app = (void (*)(void))(OS_FNS + 60);
}
