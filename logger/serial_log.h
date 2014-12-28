/**
 *
 * \file es_lib/logger/pic18_serial_log.h
 *
 * Definitions for logging to the Serial Port on the pic18 processor.
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
#ifndef PIC18_SERIAL_LOG_H
#define PIC18_SERIAL_LOG_H

/*
 * This file contains the definition of the convenience logging function
 * and various macros which can be used to simplify it's use in code.
 *
 * LOG_D -> log a debug message   
 * LOG_I -> log an Information message
 * LOG_W -> log a warning message
 * LOG_E -> log an Error message
 *
 * To use the convenience macros the configuration file system.h should define a
 * logging level to be used in the build. In additon each file can switch on and
 * off it's logging messages by defining "FILE_DEBUG". If the FILE_DEBUG switch
 * is not defined before inclusion of this header logging messages will be compiled
 * out of the build.
 *
 * Example system.h log level definition:
 *
 *   #define LOG_LEVEL LOG_DEBUG
 */

#include "system.h"
#include <stdio.h>

/*
 * LOG_D  -> Log a Debug severity message.
 */
#if (defined(DEBUG_FILE) && LOG_LEVEL <= LOG_DEBUG)
#define LOG_D printf("D :%s:", TAG); printf
#else
#define LOG_D 
#endif

/*
 * LOG_I  -> Log an Informationional severity message.
 */
#if (defined(DEBUG_FILE) && LOG_LEVEL <= LOG_INFO)
#define LOG_I printf("I :%s:", TAG); printf
#else
#define LOG_I
#endif

/*
 * LOG_W  -> Log a Warning severity message.
 */
#if (defined(DEBUG_FILE) && LOG_LEVEL <= LOG_WARNING)
#define LOG_W printf("W :%s:", TAG); printf
#else
#define LOG_W 
#endif

/*
 * LOG_E  -> Log an Error severity message.
 */
#if (LOG_LEVEL <= LOG_ERROR)
#define LOG_E printf("E :%s:", TAG); printf
#else
#define LOG_E 
#endif

#endif // PIC18_SERIAL_LOG_H
