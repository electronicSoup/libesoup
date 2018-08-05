/**
 * @file libesoup/comms/one_wire/one_wire.h
 *
 * @brief function prototypes for OneWire API Finctions
 *
 * @author John Whitmore
 *
 * Copyright 2017-2018 electronicSoup Limited
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
#ifndef _ONE_WIRE_H
#define _ONE_WIRE_H

#include "libesoup_config.h"

#ifdef SYS_ONE_WIRE

/**
 * @brief    function to initialise the One Wire module's data structure
 * @return   result (negative on error)
 */
extern result_t one_wire_init(void);

/**
 * @brief    function to initialise a One Wire Bus on a GPIO Pin
 * @param    pin  The GPIO Pin to use for the bus.
 * @return   result (negative on error)
 */
extern result_t one_wire_reserve(enum gpio_pin pin);

#endif // SYS_ONE_WIRE
#endif // _ONE_WIRE_H
