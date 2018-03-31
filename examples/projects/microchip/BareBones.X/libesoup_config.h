/**
 *
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
#include <xc.h>

//#define SYS_CLOCK_FREQ  8000000
#define SYS_CLOCK_FREQ 60000000

#if defined(__18F4585)
#include "libesoup/boards/gauge/gauge-PIC18F4585.h"
#elif defined(__PIC24FJ256GB106__)
#include "libesoup/boards/cinnamonBun/pic24FJ/cb-PIC24FJ256GB106.h"
#elif defined(__dsPIC33EP256MU806__)
#include "libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h"
#endif
