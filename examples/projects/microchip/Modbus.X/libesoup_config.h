/**
 *
 * @file libesoup_config.h
 *
 * @author John Whitmore
 *
 * This file contains an example libesoup_config.h configuration file for 
 * working with uarts. 
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
//#define SYS_CLOCK_FREQ 8000000     // 8MHz
#define SYS_CLOCK_FREQ 60000000    // 60MHz
#elif defined(__18F4585)
#define SYS_CLOCK_FREQ 16000000     // 8MHz
#endif

#define SYS_HW_TIMERS
#define SYS_SW_TIMERS

/*
 * Enable the uart functionality in libesoup
 */
#define SYS_UART

#ifdef SYS_UART
#define SYS_UART_TX_BUFFER_SIZE   200
#endif

/*
 * System jobs
 */
//#define SYS_JOBS

#ifdef SYS_JOBS
#define SYS_NUMBER_OF_JOBS 10
#endif

/*
 * MODBUS
 */
#define SYS_MODBUS

#ifdef SYS_MODBUS
#define SYS_MODBUS_RX_BUFFER_SIZE                  256
#define SYS_MODBUS_RESPONSE_TIMEOUT                SECONDS_TO_TICKS(1)
#define SYS_MODBUS_RESPONSE_BROADCAST_TIMEOUT      MILLI_SECONDS_TO_TICKS(500)
#endif // SYS_MODBUS


/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */


#endif //_LIBESOUP_CONFIG_H
