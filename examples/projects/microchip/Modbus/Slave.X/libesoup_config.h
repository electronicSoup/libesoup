/**
 * libesoup/examples/projects/microchip/BareBones.X/libesoup_config.h
 *
 * Example minimum configuration file for using the libesoup library of code.
 * Simply specifies the desired clock speed and includes the board file for the
 * target cinnamonBun board.
 * 
 * The code is used in the example MPLAB-X project:
 * libesoup/examples/projects/microchip/BareBones.X
 *
 * Copyright 2018 electronicSoup Limited
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
#define SYS_TEST_BUILD

//#define SYS_CLOCK_FREQ  8000000
#define SYS_CLOCK_FREQ 60000000

#define SYS_HW_TIMERS
#define SYS_SW_TIMERS
#define SYS_NUMBER_OF_SW_TIMERS       10
#define SYS_SW_TIMER_TICK_ms           5

#define SYS_UART
#define SYS_UART_TX_BUFFER_SIZE      512

#define SYS_SERIAL_LOGGING
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL                LOG_DEBUG    // LOG_INFO, LOG_WARNING, LOG_ERROR
#define SYS_SERIAL_LOGGING_BAUD      19200

#define SYS_MODBUS

#ifdef SYS_MODBUS
#define SYS_MODBUS_SLAVE
#define SYS_MODBUS_NUM_CHANNELS                    2
#define SYS_MODBUS_RX_BUFFER_SIZE                256
#define SYS_MODBUS_RESPONSE_TIMEOUT_UNITS        mSeconds
#define SYS_MODBUS_RESPONSE_TIMEOUT_DURATION     500
#define SYS_MODBUS_TURNAROUND_TIMEOUT_UNITS      mSeconds
#define SYS_MODBUS_TURNAROUND_TIMEOUT_DURATION   200
#endif // SYS_MODEBUS
        
#include "libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h"
