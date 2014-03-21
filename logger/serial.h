/**
 *
 * \file es_can/logger/serial.h
 *
 * Definitions for logging to the Serial Port of the electronicSoup CAN code Library
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
#ifndef SERIAL_LOGGER_H
#define SERIAL_LOGGER_H

#if defined(MCP)
extern void serial_init(void);
#endif

#if (defined(DEBUG_FILE) && LOG_LEVEL <= LOG_DEBUG)
#define DEBUG_D(...) serial_log(Debug, TAG, __VA_ARGS__)
#else
#define DEBUG_D //
#endif

#if (defined(DEBUG_FILE) && LOG_LEVEL <= LOG_INFO)
#define DEBUG_I(...) serial_log(Info, TAG, __VA_ARGS__)
#else
#define DEBUG_I //
#endif

#if (defined(DEBUG_FILE) && LOG_LEVEL <= LOG_WARNING)
#define DEBUG_W(...) serial_log(Warning, TAG, __VA_ARGS__)
#else
#define DEBUG_W //
#endif

#if (LOG_LEVEL <= LOG_ERROR)
#define DEBUG_E(...)  serial_log(Error, TAG, __VA_ARGS__)
#else
#define DEBUG_E //
#endif

#ifdef __C30__
extern void serial_log(log_level_t level, char* tag, char* fmt, ...);
#elif defined(__18CXX)
extern void serial_log(log_level_t level, const rom char* tag, const rom char* fmt, ...);
#elif defined(ES_LINUX)
extern void serial_log(log_level_t level, char* tag, char* fmt, ...);
#endif


#endif // NODE_LOGGER_H
