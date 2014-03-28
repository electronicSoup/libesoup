/**
 *
 * \file es_can/android/android.h
 *
 * Android device comms Definitions
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
#ifndef ANDROID_H
#define ANDROID_H

extern BOOL androidReceive(BYTE *buffer, UINT16 *size, BYTE *errorCode);
extern BYTE androidTransmit(BYTE *buffer, BYTE Size);
extern BYTE androidTasks(void* device_handle);


#endif /* SEND_ANDROID_H */
