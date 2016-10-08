/**
 *
 * @file es_lib/processors/es-dsPIC33EP256MU806.h
 *
 * @author John Whitmore
 *
 * This file contains an example es_lib system.h configuration file. 
 *
 * The es_lib library of source code expects a system.h header file to exist
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
#ifndef _ES_PIC18F4585_H
#define _ES_PIC18F4585_H

#include <p18cxxx.h>

#define TIMER_0 0
#define TIMER_1 1

#define NUMBER_HW_TIMERS  2

extern void pic18f_init(void);

#endif // _ES_PIC18F2680_H
