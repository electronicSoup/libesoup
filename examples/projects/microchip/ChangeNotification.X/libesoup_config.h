/**
 *
 * @file libesoup/example_libesoup_config.h
 *
 * @author John Whitmore
 *
 * This file contains an example libesoup libesoup_config.h configuration file. 
 *
 * The libesoup library of source code expects a libesoup_config.h header file to exist
 * in your include path. The file contains the various switches and definitions
 * which configure the various features of the library.
 *
 * Copyright 2015 John Whitmore <jwhitmore@electronicsoup.com>
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


/**
 * @brief The required Instruction clock frequency of the device. 
 * 
 * The actual Hardware clock frequency is defined by the MACRO CRYSTAL_FREQ in
 * core.h. That constant is used in conjunction with this required SYS_CLOCK_FREQ 
 * in the function clock_init() to set the desired frequency with the PLL.
 *  
 * The function clock_init() has to be called on entry to main() to 
 * initialise the device to this clock frequency.
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
#define SYS_CLOCK_FREQ 16000000     // 8MHz
#elif defined(__dsPIC33EP256MU806__)
//#define SYS_CLOCK_FREQ 8000000     // 8MHz
//#define SYS_CLOCK_FREQ 40000000    // 40MHz
#define SYS_CLOCK_FREQ 60000000    // 60MHz
#elif defined(__18F4585)
#define SYS_CLOCK_FREQ 16000000     // 8MHz
#endif

#define SYS_UART
#define SYS_UART_TX_BUFFER_SIZE 256

#define SYS_SERIAL_LOGGING

/**
 * @brief Pin Orientation of the Serial Logging port.
 *
 * The three pin serial logging port has a fixed Gnd pin, but the remaining two
 * pins may either be fixed, or may use Microchip peripheral select
 * functionality, depending on microncontroller being used, to configure
 * orientation of the three pin port.
 *
 * There are two valid settings SYS_SERIAL_PORT_GndTxRx or SERIAL_PORT_GndRXTx 
 * Once the orientation has been defined this libesoup_config.h file should inclue a 
 * board file which defines the pins being used on the board.
 * 
 * Default : SYS_SERIAL_PORT_GndRxTx
 */
//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx

/**
 * @brief Baud rate of the serial logging port
 *
 * This is Baud rate is set by the function serial_init(). This call to 
 * initialise the baud rate has to be preceeded by a call to clock_init() so
 * that the device's instruction clock speed is know, and the correct scaling
 * can be preformed for the required baud rate.
 * 
 * Default set to 19k2
 */
#define SYS_SERIAL_LOGGING_BAUD 19200

/**
 * @brief Serial logging level
 *
 * The valid log levels are defined in libesoup/core.h.
 * Default set to Debug logging level. This switch is used in conjunction
 * with libesoup/logging/serial_log.h. That file defines Logging macros which 
 * expect a logging level to be defined and a #define of "DEBUG_FILE" at the 
 * top of the file for logging to be enabled.
 */
//#define SYS_LOG_LEVEL NO_LOGGING
#define SYS_LOG_LEVEL LOG_DEBUG

#define SYS_CHANGE_NOTIFICATION
#define SYS_CHANGE_NOTIFICATION_MAX_PINS  10
//#define SYS_ONE_WIRE

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
