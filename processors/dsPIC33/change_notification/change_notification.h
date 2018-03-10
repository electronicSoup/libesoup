/**
 *
 * \file libesoup/processors/dsPIC33/change_notification/change_notification.h
 *
 * Copyright 2018 electronicSoup Limited
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
#ifdef SYS_CHANGE_NOTIFICATION
typedef void (*change_notifier)(uint8_t *port, uint8_t bit);

extern result_t change_notifier_init(void);
extern result_t change_notifier_register(uint8_t *port, uint8_t bit, change_notifier notifier);
extern result_t change_notifier_deregister(uint8_t *port, uint8_t bit);
#endif  // SYS_CHANGE_NOTIFICATION
