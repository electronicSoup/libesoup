/**
 *
 * \file es_lib/example_system.h
 *
 * This file contains the various switches which can be used with the
 * es_lib source code library. 
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "es_lib/core.h"

/*
 * Baud rate of the serial debug port
 */
#define SERIAL_BAUD 19200

/*
 * The configuration of the Serial Port there are 3 pins, the Gnd pin is fixed
 * so you define the configuration of the other two pins. Uncomment as required
 */
//#define SERIAL_PORT_GndTxRx
#define SERIAL_PORT_GndRxTx

/*
 * Log level. these are defined in es_lib/core.h as they are included everywhere
 */
#define LOG_LEVEL LOG_DEBUG

/*
 * NUMBER_OF_TIMERS
 * 
 * Definition of the number of system timers required in the system. Increasing 
 * the number of timers uses more system RAM. This will depend on the estimated
 * demand for timers in your design.
 */
#define NUMBER_OF_TIMERS 20

#define HW_SPI

/*
 * CAN Definitions
 */
/*
 * The number of Handlers that can be registered with Layer 2
 */
#define CAN
#define CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD    SECONDS_TO_TICKS(10)
#define CAN_L2_HANDLER_ARRAY_SIZE 5
//#define CAN_LAYER_3

/*
 * Android Definitions:
 *
 * When using the library Android states one of these switches should be
 * enabled.
 */
//#define ANDROID_BOOT
//#define ANDROID_NODE
//#define ANDROID_DONGLE
