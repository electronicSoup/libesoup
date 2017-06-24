/**
 *
 * \file libesoup/usb/android/android.h
 *
 * Function prototypes for Android Comms
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#ifndef ANDROID_COMMS_H
#define ANDROID_COMMS_H

extern void      android_init(void* data);
extern void      android_set_tx_finished_callback(void (*data)(void));
extern void      android_tasks(void);
extern uint8_t   android_transmit(uint8_t *buffer, uint8_t size);


#endif /* ANDROID_COMMS_H */
