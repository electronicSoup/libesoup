/**
 * @file libesoup/examples/projects/microchip/USB/USB-XPad.X/src

 * @author John Whitmore
 * 
 * @brief Example minimum configuration file for using the cinnamonBun in USB
 *        Host mode communicating with a Logitech XPad game controller.
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
 */

//#define SYS_CLOCK_FREQ  8000000
#define SYS_CLOCK_FREQ 60000000

#define SYS_UART
#define SYS_SERIAL_LOGGING
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL               LOG_DEBUG
#define SYS_UART_TX_BUFFER_SIZE           512
#define SYS_SERIAL_LOGGING_BAUD         19200

#define SYS_USB_XPAD

#if defined(__18F4585)
#include "libesoup/boards/gauge/gauge-PIC18F4585.h"
#elif defined(__PIC24FJ256GB106__)
#include "libesoup/boards/cinnamonBun/pic24FJ/cb-PIC24FJ256GB106.h"
#elif defined(__dsPIC33EP256MU806__)
#include "libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h"
#endif
