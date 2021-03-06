/**
 * @file libesoup/gpio/change_notification.h
 *
 * @author John Whitmore
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

/*
 * Only tested on the dsPIC33 so far
 */
typedef void (*change_notifier)(enum gpio_pin pin);

extern result_t change_notifier_register(enum gpio_pin pin, change_notifier notifier);
extern result_t change_notifier_deregister(enum gpio_pin pin);

#endif  // SYS_CHANGE_NOTIFICATION
