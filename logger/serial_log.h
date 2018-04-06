/**
 *
 * @file libesoup/logger/serial_log.h
 *
 * @author John Whitmore
 * 
 * @brief Definitions for configuration of the Serial Port
 *
 * Copyright 2017-2018 electronicSoup Limited
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
 */
/**
 * \defgroup SerialLogging Serial Logging
 * @{
 */
#ifndef _SERIAL_LOG_H
#define _SERIAL_LOG_H

#ifdef SYS_SERIAL_LOGGING

/**
 * @def   LOG_DEBUG
 * @brief Debug logging level
 *
 * @def   LOG_INFO
 * @brief Info logging level
 *
 * @def   LOG_WARNING
 * @brief Warning logging level
 *
 * @def   LOG_ERROR
 * @brief ERROR logging level
 *
 * @def   NO_LOGGING
 * @brief No logging in System
 *
 * This series of defines are here as they should be included very early. At 
 * leasy before the serial logging code. These macros are used to conditionally
 * compile debugging code out of an executable. The actual logging level of the
 * build should be defined in libesoup_config.h
 */
#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARNING 2
#define LOG_ERROR   3
#define NO_LOGGING  4

/**
 *  serial_logging_init()
 *
 * This function is only defined for Code compiled for the Microchip
 * microcontroller
 * 
 * In additon the libesoup_config.h config file should define the baud rate to
 * be used and the physical pin configuration of the 3 pin serial port:
 * GndRxTx or GndTxRx
 *
 * for example : #define SERIAL_BAUD 19200
 *
 * And either : #define SYS_SERIAL_PORT_GndTxRx or #define SYS_SERIAL_PORT_GndRxTx
 *
 */
#if defined(XC16) || defined(__XC8)
extern result_t serial_logging_init(void);
#ifdef SYS_DEBUG_BUILD
extern uint16_t serial_buffer_count(void);
#endif
#if defined(XC16)
extern void     serial_log(uint8_t level, const char * tag, const char * f, ...);
extern void     serial_printf(const char * f, ...);
#elif defined(__XC8)
extern void     serial_log(const char* fmt, ...);
#endif
extern result_t serial_logging_exit(void);

/*
 * The PIC18 Processors process the serial Interrupt loading up the TXREG
 * register. This function should be called from the PIC18 ISR
 */
#if defined(__18F2680) || defined(__18F4585)
//extern void serial_isr(void);
//extern void putch(char);
#endif // (__18F2680) || (__18F4585)

#endif // XC16 || __XC8

#ifdef XC16
/*
 * XC16 Compiler does support Variadic Macros see:
 * https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 */
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
#define LOG_D(...)  serial_log(LOG_DEBUG, TAG, __VA_ARGS__);
#else
#define LOG_D(...)
#endif

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
#define LOG_I(...)  serial_log(LOG_INFO, TAG, __VA_ARGS__);
#else
#define LOG_I(...)
#endif

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_WARNING))
#define LOG_W(...)  serial_log(LOG_WARNING, TAG, __VA_ARGS__);
#else
#define LOG_W(...)
#endif

#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
#define LOG_E(...)  serial_log(LOG_ERROR, TAG, __VA_ARGS__);
#else
#define LOG_E(...)
#endif

#elif defined(__XC8)
/*
 * XC8 Compiler does NOT support Variadic Macros see:
 * https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 */
#if (SYS_LOG_LEVEL <= LOG_DEBUG)
#define LOG_D     serial_log("D-");  \
                  serial_log("%s:", TAG);   \
                  serial_log
#endif

#if (SYS_LOG_LEVEL <= LOG_INFO)
#define LOG_I     serial_log("I-");  \
                  serial_log("%s:", TAG);   \
                  serial_log
#endif

#if (SYS_LOG_LEVEL <= LOG_WARNING)
#define LOG_W     serial_log("W-");  \
                  serial_log("%s:", TAG);   \
                  serial_log
#endif

#if (SYS_LOG_LEVEL <= LOG_ERROR)
#define LOG_E     serial_log("E-");  \
                  serial_log("%s:", TAG);   \
                  serial_log
#endif

#endif // __XC8

#endif // SERIAL_LOG_H

#endif // SYS_SERIAL_LOGGING
/**
 * @}
 */
