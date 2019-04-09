/**
 *
 * @file libesoup/examples/projects/microchip/ChangeNotification.X/libesoup_config.h
 *
 * @author John Whitmore
 *
 * Example configuration file for using Change Notification on a GPIO Pin.
 *
 * Copyright 2019 electronicSoup
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
//#define SYS_CLOCK_FREQ 8000000     // 8MHz
//#define SYS_CLOCK_FREQ 40000000    // 40MHz
#define SYS_CLOCK_FREQ 60000000    // 60MHz
#endif

#define SYS_UART
#define SYS_UART_TX_BUFFER_SIZE 1024

#define SYS_SERIAL_LOGGING
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_SERIAL_LOGGING_BAUD 19200
#define SYS_LOG_LEVEL LOG_DEBUG

#define SYS_CHANGE_NOTIFICATION
#define SYS_CHANGE_NOTIFICATION_MAX_PINS  10

#define SYS_TIMERS
#define SYS_HW_TIMERS

#define SYS_SW_TIMERS
#define SYS_SW_TIMER_TICK_ms        5
#define SYS_NUMBER_OF_SW_TIMERS    10

/*
 * Include a board file
 */
#if   defined(__18F2680) 
#include "libesoup/boards/gauge-PIC18F2680.h"
#elif defined(__18F4585)
#include "libesoup/boards/gauge-PIC18F4585.h"
#elif defined  (__PIC24FJ64GB106__)
#include "libesoup/boards/cb-PIC24FJ64GB106.h"
#elif defined(__PIC24FJ256GB106__)
#include "libesoup/boards/cb-PIC24FJ256GB106.h"
#elif defined(__dsPIC33EP256MU806__)
#include "libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h"
#endif

/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */
#endif //_LIBESOUP_CONFIG_H
