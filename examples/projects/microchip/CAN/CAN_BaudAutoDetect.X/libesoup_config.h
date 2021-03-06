/**
 *
 * @file libesoup/examples/projects/microchip/CAN_BaudAutoDetect.X/libesoup_config.h
 *
 * @author John Whitmore
 *
 * Copyright 2017-2018 electronicSoup Limited
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#if defined(__dsPIC33EP256MU806__)
//#define SYS_CLOCK_FREQ 8000000     // 8MHz
#define SYS_CLOCK_FREQ 60000000    // 60MHz
#endif

#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

#define SYS_UART
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL LOG_DEBUG
#define SYS_SERIAL_LOGGING_BAUD           19200
//#define SYS_SERIAL_LOGGING_BAUD           38400
//#define SYS_SERIAL_LOGGING_BAUD           76800
//#define SYS_SERIAL_LOGGING_BAUD           115200
#define SYS_UART_TX_BUFFER_SIZE 300

#endif // defined(SYS_SERIAL_LOGGING)

#define SYS_HW_TIMERS
#define SYS_SW_TIMERS
#define SYS_NUMBER_OF_SW_TIMERS    10
#define SYS_SW_TIMER_TICK_ms        5

/*
 * EEPROM Chip is connected via the SPI Interface so enable
 */
#define SYS_SPI_BUS
#define SYS_SPI_NUM_CHANNELS      1
#define SYS_SPI_NUM_DEVICES       1
#define SYS_EEPROM

#define SYS_SYSTEM_STATUS

/*
 * Include CAN BUS Functionality in this project
 */
#define SYS_CAN_BUS
#define SYS_CAN_FRAME_HANDLER_ARRAY_SIZE  10
#define SYS_CAN_L2_HANDLER_ARRAY_SIZE      5
#define SYS_CAN_RX_CIR_BUFFER_SIZE         5        
#define SYS_CAN_BAUD_AUTO_DETECT
#define SYS_CAN_BAUD_AUTO_DETECT_PERIOD   20

/*
 * Include a board file
 */
#if defined(__dsPIC33EP256MU806__)
#include "libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h"
#elif defined (__PIC24FJ256GB106__)
#include "libesoup/boards/cinnamonBun/pic24FJ/cb-PIC24FJ256GB106.h"
#elif defined(__18F4585)
#include "libesoup/boards/gauge-PIC18F4585.h"
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

#endif // _LIBESOUP_CONFIG_H
