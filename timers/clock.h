/**
 *
 * \file libesoup/utils/clock.h
 *
 * Definitions for changing the clock speed of the  processor.
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
#ifndef _CLOCK_H
#define _CLOCK_H

/**
 * Function to initialise the micro-controller's required clock frequency, which
 * should be defined, as SYS_CLOCK_FREQ, in your libesoup_config.h file. See
 * example definition in libesoup/examples/libesoup_config.h
 * 
 * The initialisation of the clock signal depends on the Crystal being used by
 * the hardware. This is defined as CRYSTAL_FREQ in the board header file 
 * included from libesoup/boards. 
 */
extern void clock_init(void);

#endif // _CLOCK_H
