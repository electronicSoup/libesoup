/**
 *
 * \file es_lib/app/os.h
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

extern void        os_init(void);

extern result_t  (*timer_start)(UINT16, expiry_function, union sigval data, es_timer *timer);
extern result_t  (*timer_cancel)(es_timer *timer);

extern result_t  (*eeprom_read)(UINT16 address, BYTE *data);
extern result_t  (*eeprom_write)(UINT16 address, BYTE data);

extern result_t  (*can_l2_tx_frame)(can_frame *);
extern result_t  (*can_l2_dispatch_reg_handler)(can_l2_target_t *target);
extern result_t  (*can_l2_dispatch_unreg_handler)(u8 id);

extern result_t  (*get_node_address)(BYTE *address);
extern result_t  (*iso15765_tx_message)(iso15765_msg_t *msg);
extern result_t  (*iso15765_dispatch_reg_handler)(iso15765_target_t * target);
extern result_t  (*iso15765_dispatch_unreg_handler)(u8 id);

extern void      (*serial_log)(log_level_t level, char *tag, char *fmt, ...);

extern void      (*iso15765_log)(log_level_t level, char *msg);

extern void      (*invalidate_app)(void);
extern result_t  (*get_io_address)(u8 *);

#endif // ES_OS_H
