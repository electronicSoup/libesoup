/**
 *
 * @file libesoup/examples/projects/microchip/SW_Timers.X/libesoup_config.h
 *
 * @author John Whitmore
 *
 * This file contains an example libesoup_config configuration file for
 * working with Software Based timers. 
 *
 * Copyright 2017 electronicSoup Limited
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#if defined(__dsPIC33EP256MU806__)
//#define CLOCK_FREQ 8000000     // 8MHz
#define SYS_CLOCK_FREQ 60000000    // 60MHz
#endif

#define SYS_HW_TIMERS
#define SYS_SW_TIMERS
#define SYS_NUMBER_OF_SW_TIMERS        5
#define SYS_SW_TIMER_TICK_ms           5        // mSeconds


#if defined(__dsPIC33EP256MU806__)
#include "libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h"
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
#include "libesoup/boards/cinnamonBun/pic24FJ/cb-PIC24FJ256GB106.h"
#elif defined(__18F4585)
#include "libesoup/boards/gauge/gauge-PIC18F4585.h"
#elif defined(__RPI)
#include "libesoup/boards/rpi.h"
#endif

/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */


#endif //_LIBESOUP_CONFIG_H
