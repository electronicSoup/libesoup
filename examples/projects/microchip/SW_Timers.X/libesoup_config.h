/**
 *
 * @file libesoup_config.h
 *
 * @author John Whitmore
 *
 * This file contains an example libesoup_config configuration file for
 * working with Software Based timers. 
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "libesoup/boards/cb-dsPIC33EP256MU806.h"
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
#include "libesoup/boards/cb-PIC24FJ256GB106.h"
#elif defined(__18F4585)
#include "libesoup/boards/gauge-PIC18F4585.h"
#elif defined(__RPI)
#include "libesoup/boards/rpi.h"
#endif

/**
 * @brief The required clock frequency of the device. 
 * 
 * The function clock_init() has to be called on entry to main() to 
 * initialise the device to this clock frequency.
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
#define SYS_CLOCK_FREQ 16000000     // 8MHz
#elif defined(__dsPIC33EP256MU806__)
//#define CLOCK_FREQ 8000000     // 8MHz
#define SYS_CLOCK_FREQ 60000000    // 60MHz
#elif defined(__18F4585)
#define SYS_CLOCK_FREQ 16000000     // 8MHz
#endif

#define SYS_LOG_LEVEL NO_LOGGING

#define SYS_HW_TIMERS

/**
 * @brief Software based timers
 *
 * If your project includes libesoup Software Timer code then you must use this 
 * definition to define number of timers actually avalible in the system. 
 *
 * In addition to adding this Switch your code must initialise the timer
 * functionality and regularly check the timers with the "CHECK_TIMERS()" MACRO
 * defined in libesoup/timers/sw_timers.h
 *
 * For an example of using the Software timers take a look at:
 * libesoup/examples/main_sw_timers.c
 * 
 * Default : Switch is commented out assuming timer functionality not included
 * in project.
 */
#define SYS_SW_TIMERS

#ifdef SYS_SW_TIMERS
/*
 * Software timers are based on a single Hardware timer so Hardware timers
 * have to be enabled in the system.
 */
#ifndef SYS_HW_TIMERS  
#define SYS_HW_TIMERS
#endif

#define SYS_NUMBER_OF_SW_TIMERS       20 
#define SYS_SW_TIMER_TICK_ms           5        // mSeconds
#endif // SYS_SW_TIMERS

/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */


#endif //_LIBESOUP_CONFIG_H