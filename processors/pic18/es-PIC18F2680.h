/**
 *
 * @file libesoup/processors/es-dsPIC33EP256MU806.h
 *
 * @author John Whitmore
 *
 * This file contains an example libesoup libesoup_config.h configuration file. 
 *
 * The libesoup library of source code expects a libesoup_config.h header file to exist
 * in your include path. The file contains the various switches and definitions
 * which configure the various features of the library.
 *
 * Copyright 2017 electronicSoup Limited
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
#ifndef _ES_PIC18F2680_H
#define _ES_PIC18F2680_H

#include <p18cxxx.h>

#define TIMER_0 0
#define TIMER_1 1

#define NUMBER_HW_TIMERS  2

extern void pic18f_init(void);

#endif // _ES_PIC18F2680_H
