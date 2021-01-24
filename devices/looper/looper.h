/**
 * @file libesoup/devices/looper/looper.h
 *
 * @author John Whitmore
 *
 * @brief File containing API for controlling guitar looper pedals.
 *        Tested with the TC Electronic Looper Pedals.
 *
 * Copyright 2021 electronicSoup Limited
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
 *******************************************************************************
 *
 */
#include "libesoup_config.h"
#ifdef SYS_LOOPER

#ifndef _LOOPER_H
#define _LOOPER_H

/*
 * Single API Function to initialise the looper peday,
 * GPIO pin to use
 * MIDI Channel & MIDI Pitch controlling the looper pedal.
 */
extern result_t looper_init(enum gpio_pin gpio, uint8_t midi_channel, uint8_t midi_pitch);

#endif // _LOOPER_H
#endif // SYS_LOOPER
