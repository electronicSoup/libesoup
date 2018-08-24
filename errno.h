/**
 * @file libesoup/errno.h
 *
 * @author John Whitmore
 *
 * As the name suggests this project defines errors in the libesoup based
 * project. The convention used is that API functions return a result_t type
 * which is just a signed 16 bit value. Negative numbers are an indication of
 * an error having occurred. Conversely a zero or positive result is a successful
 * API call and can communicate information to the caller. So for example in the
 * case of starting a Software based timer the returned result_t, if there has
 * not been an error, the returned value is the identifier of the started
 * Software timer. This timer identifier can then be used to cancel the started
 * timer.
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
 */
#ifndef _ERRNO_H
#define _ERRNO_H

#include "libesoup_config.h"

/**
 * @typedef  result_t
 * @brief    Return type from API function call. Negative on error condition
 *
 * Throughout the library variables of this type are called 'rc' for return
 * code.
 */
typedef int16_t result_t;

/**
 * @def      RC_CHECK
 * @brief    Check the previous return code and return on error on previous API
 *           call.
 *
 * This MACRO assumes that the result_t variable is called 'rc' which is the
 * convention used through out the library.
 */
#define RC_CHECK if(rc < 0) return(rc);

/**
 * @def      RC_CHECK_STOP
 * @brief    Define a macro to halt the uC on error.
 *
 * This MACRO is intended to be used on Application layer of the project. In
 * the main() function for example there is no point in calling the RC_CHECK
 * MACRO which will make a call to "return()". There is little point in
 * returning from the main() function to this MACRO will enter an infinite
 * loop on detecting an error from a previous API call.
 *
 * Once again this MACRO assumes that the type result_t variable is called 'rc'.
 */
#if defined(SYS_SERIAL_LOGGING)
#define RC_CHECK_STOP           if (rc <0){ LOG_E("%s-%d\n\r", __FILE__, __LINE__); while (1); }
#else
#define RC_CHECK_STOP           if (rc <0) while (1);
#endif // SYS_SERIAL_LOGGING

/**
 * @def      RC_CHECK_PRINT_CONT
 * @brief    Convenience MACRO to print a message on error and continue.
 *
 * If the project is using SYS_SERIAL_LOGGING this MACRO simply tests the
 * return code from a previously called API function and on error logs an
 * error message to serial logging output. The error is then ignored and
 * execution continues, as if there was no error encountered.
 *
 * If SYS_SERIAL_LOGGING is not enabled then any error is ignored.
 */
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL >= LOG_ERROR))
#define RC_CHECK_PRINT_CONT(x) if(rc < 0) LOG_E(x);
#else
#define RC_CHECK_PRINT_CONT(x)
#endif

/**
 * @def      RC_CHECK_PRINT_VOID
 * @brief    Convenience MACRO to print a message on error and return.
 *
 * If the project is using SYS_SERIAL_LOGGING this MACRO simply tests the
 * return code from a previously called API function and on error logs an
 * error message to serial logging output. The MACRO then returns immediately.
 *
 * If SYS_SERIAL_LOGGING is not enabled then any error causes an immediate
 * return to the caller.
 *
 * Note - This MACRO is indented to be used in functions with a 'void'
 * return type.
 */
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL >= LOG_ERROR))
#define RC_CHECK_PRINT_VOID(x) if(rc < 0) { LOG_E("x"); return; }
#else
#define RC_CHECK_PRINT_VOID(x) if(rc < 0) { return; }
#endif

#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL >= LOG_ERROR))
#define RC_CHECK_LINE_CONT     if(rc < 0) LOG_E("&s-%d\n\r", __FILE__, __LINE__);
#else
#define RC_CHECK_LINE_CONT
#endif

#define FILE_LINE      LOG_E("%s-%d\n\r", __FILE__, __LINE__);

/**
 * @def      SUCCESS
 * @brief    Much of the code uses a magic number of '0' to indicate
 *           successful execution of a function. This MACRO is used to
 *           avoid the magic number.
 */
#define SUCCESS                     0

/**
 * @def      ERR_*
 * @brief    Error numbers returned from API Calls.
 *
 * Note - Returned as a negative number, so for example:
 *        return(-ERR_GENERAL_ERROR);
 */
#define ERR_GENERAL_ERROR           1
#define ERR_BAD_INPUT_PARAMETER     2
#define ERR_NOT_CODED               3
#define ERR_BUFFER_OVERFLOW         4
#define ERR_RANGE_ERROR             5
#define ERR_TIMER_ACTIVE            6
#define ERR_NO_RESOURCES            7
#define ERR_ADDRESS_RANGE           8
#define ERR_NOT_READY               9
#define ERR_CAN_ERROR              10
#define ERR_CAN_NOT_CONNECTED      11
#define ERR_CAN_BITRATE_LOW        12
#define ERR_CAN_BITRATE_HIGH       13
#define ERR_CAN_INVALID_BAUDRATE   14
#define ERR_CAN_NO_FREE_BUFFER     15
#define ERR_UNINITIALISED          16
#define ERR_BUSY                   17
#define ERR_NOTHING_TO_DO          18
#define ERR_NO_RESPONSE            19
#define ERR_NOT_MASTER             20
#define ERR_NOT_SLAVE              21
#define ERR_IM_A_TEAPOT           418

#endif // _ERRNO_H
