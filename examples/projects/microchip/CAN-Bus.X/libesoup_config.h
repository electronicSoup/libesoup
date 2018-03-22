/**
 *
 * @file libesoup/examples/projects/microchip/CAN-Bus.x/libesoup_config.h
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

#include <xc.h>

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
#define SYS_CLOCK_FREQ 4000000     // 4MHz
#elif defined(__dsPIC33EP256MU806__)
//#define SYS_CLOCK_FREQ 8000000     // 8MHz
#define SYS_CLOCK_FREQ 60000000    // 60MHz
#elif defined(__18F4585)
#define SYS_CLOCK_FREQ 16000000     // 8MHz
#endif

#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

/**
 * @brief Physical Pin configuration of the Serial Logging port.
 *
 * On the cinnamonBun the the Gnd pin is physically fixed but the remaining two
 * pins use Microchip peripheral select functionality to configure which pin is
 * the Receive pin and which is the Transmit.
 *
 * There are two valid settings SYS_SERIAL_PORT_GndTxRx or SERIAL_PORT_GndRXTx 
 * 
 * Default : SYS_SERIAL_PORT_GndRxTx
 */
//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx

/**
 * @brief Serial logging level
 *
 * The valid log levels are defined in es_lib/core.h.
 * Default set to Debug logging level. This switch is used in conjunction
 * with es_lib/logging/serial_log.h. That file defines Logging macros which 
 * expect a logging level to be defined and a #define of "DEBUG_FILE" at the 
 * top of the file for logging to be enabled.
 */
#define SYS_LOG_LEVEL LOG_DEBUG

#define SYS_UART
        
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
#define SYS_SERIAL_LOGGING_BAUD           19200
//#define SYS_SERIAL_LOGGING_BAUD           38400
//#define SYS_SERIAL_LOGGING_BAUD           76800
//#define SYS_SERIAL_LOGGING_BAUD           115200

/**
 * @brief The size of the Transmit buffer to be used by the UARTs.
 *
 * Default set to 300 Bytes as the serial port
 */
#define SYS_UART_TX_BUFFER_SIZE 300

#else  // defined(SYS_SERIAL_LOGGING)
#define SYS_UART
/**
 * @brief The size of the Transmit buffer to be used by the UARTs.
 *
 * Default set to 300 Bytes as the serial port
 */
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

/*
 * Include access to the EEPROM Chip in the project
 */
#define SYS_EEPROM

/*
 * CAN Bus depends on System Status Code
 */
#define SYS_SYSTEM_STATUS

/*
 * Include CAN BUS Functionality in this project
 */
#define SYS_CAN_BUS
//#define SYS_CAN_LOOPBACK
#define SYS_CAN_FRAME_HANDLER_ARRAY_SIZE  10
#define SYS_CAN_L2_HANDLER_ARRAY_SIZE      5
#define SYS_CAN_RX_CIR_BUFFER_SIZE         5        

/*
 * Include a board file
 */
#if defined(__dsPIC33EP256MU806__)
#include "libesoup/boards/cb-dsPIC33EP256MU806.h"
#elif defined (__PIC24FJ256GB106__)
#include "libesoup/boards/cb-PIC24FJ256GB106/cb-PIC24FJ256GB106.h"
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
