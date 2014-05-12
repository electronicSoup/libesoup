/**
 *
 * \file es_can/os/os.h
 *
 * OS function prototypes
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
#ifndef ES_OS_H
#define ES_OS_H

#include "es_lib/core.h"

#define OS_FNS 0x7E92
extern void        os_init(void);

extern result_t  (*os_start_timer)(UINT16, expiry_function, BYTE *, es_timer *timer);
extern result_t  (*os_cancel_timer)(es_timer *timer);

extern  result_t (*os_eeprom_read)(UINT16 address, BYTE *data);
extern result_t  (*os_eeprom_write)(UINT16 address, BYTE data);

extern result_t  (*os_l2_can_tx_msg)(can_frame *);
extern result_t  (*os_l2_can_dispatch_register_handler)(can_target_t *target, BYTE *id);
extern result_t  (*os_l2_can_dispatch_unregister_handler)(BYTE id);

extern result_t  (*os_l3_get_address)(BYTE *address);
extern result_t  (*os_l3_can_tx_message)(l3_can_msg_t *msg);
extern result_t  (*os_l3_can_dispatch_register_handler)(BYTE protocol, l3_msg_handler_t handler, BYTE *id);
extern result_t  (*os_l3_can_dispatch_unregister_handler)(BYTE id);

extern void      (*os_serial_log)(log_level_t level, char *tag, char *fmt, ...);

extern void      (*os_net_log)(log_level_t level, char *msg);
extern result_t  (*os_net_log_register_as_handler)(void (*)(log_level_t, char *), log_level_t);
extern result_t  (*os_net_log_unregister_as_handler)(void);
extern void      (*os_invalidate_app)(void);

#endif // ES_OS_H
