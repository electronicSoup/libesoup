/**
 *
 * @file libesoup/boards/stereo_guitar.h
 *
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
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
#ifndef _STEREO_GUITAR_H
#define _STEREO_GUITAR_H

#include "libesoup/processors/es-dsPIC33EP128GS702.h"

/**
 * @brief Crystal Frequency of the Hardware Device.
 *
 * The dsPIC33 contains a Phase Locked Loop so this crystal frequency is not
 * necessarily the instruction clock used by the device. The libesoup_config.h
 * file defines the requested instruction clock frequency, which will be 
 * configured, as part of libesoup_init() on startup.
 */
#define BRD_CRYSTAL_FREQ 7600000

/**
 * @brief Serial Logging pin configuration
 *
 * The cinnamonBun has three pins for the debug interface, which will be used
 * if SYS_SERIAL_LOGGING is defined in libesoup_config.h. One pin is hardwired 
 * to ground but the other two can be configured as either the recieve pin or the
 * transmit pin, using the peripheral pin select mechanism of the dsPIC33.
 * If libesoup_config.h defines SYS_SERIAL_LOGGING then it should specify the pin
 * orientaiton fo the serial logging port
 */
#ifdef SYS_SERIAL_LOGGING
#if defined(SYS_SERIAL_PORT_GndTxRx)
        #define BRD_SERIAL_LOGGING_TX_PIN  RA3
        #define BRD_SERIAL_LOGGING_RX_PIN  RA4
#elif defined(SYS_SERIAL_PORT_GndRxTx)
        #define BRD_SERIAL_LOGGING_TX_PIN  RA4
        #define BRD_SERIAL_LOGGING_RX_PIN  RA3
#else
#error Serial Logging pin orientation not defined!
#endif
#endif // SYS_SERIAL_LOGGING

#include "libesoup/core.h"

#endif // _STEREO_GUITAR_H
