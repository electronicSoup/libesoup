/**
 * @file libesoup/core.h
 *
 * @author John Whitmore
 *
 * Core definitions required by electronicSoup Code Library
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
#ifndef _CORE_H
#define _CORE_H

#include "libesoup_config.h"

#ifndef SYS_SERIAL_LOGGING
#ifdef XC16
#define LOG_D(...)
#define LOG_I(...)
#define LOG_W(...)
#define LOG_E(...)
#elif defined(__XC8)
#define LOG_D()
#define LOG_I()
#define LOG_W()
#define LOG_E()
#endif  // Compiler
#endif  // SYS_SERIAL_LOGGING

/**
 * This series of defines are here as they should be included very early. At 
 * leasy before the serial logging code. These macros are used to conditionally
 * compile debugging code out of an executable. The actual logging level of the
 * build should be defined in libesoup_config.h
 */
#define LOG_ERROR   0      ///< ERROR logging level
#define LOG_WARNING 1      ///< Warning logging level
#define LOG_INFO    2      ///< Info logging level
#define LOG_DEBUG   3      ///< Debug logging level
#define NO_LOGGING  4      ///< No logging in System


/*
 * include stdint.h to get access to uint8_t style types
 */
#if defined(XC16) || defined(__XC8)
    #include <stdint.h>

/**
 * @var   sys_clock_freq
 * @brief Instruction/Peripheral Clock frequency being used by the system
 *
 * The system has a crystal frequency defined in the board file by
 * BRD_CRYSTAL_FREQ and the libesoup_config.h file requests a frequency with
 * SYS_CLOCK_FREQ, but the frequency being used by the system is given by
 * this variable.
 */
extern uint32_t sys_clock_freq;


#ifndef	NULL
/**
 * @def    NULL
 * @brief  Definition for 'NULL' if it is not already included in the build system
 */
#define NULL (0)
#endif	// #ifndef NULL

/**
 * @typedef boolean
 * @brief   definition of a boolean type
 */
typedef uint8_t    boolean;

#ifndef FLASE
/**
 * @def    FALSE
 * @brief  Definition of FALSE if not already defined by build system
 */
#define FALSE 0x00
#endif  // #ifndef FALSE

#ifndef TRUE
/**
 * @def   TRUE
 * @brief Definition of TRUE if not already defined by the build system
 */
#define TRUE (!FALSE)
#endif // #ifndef TRUE

#elif defined(ES_LINUX)
    #include <stdint.h>

    typedef unsigned char boolean;
    #define FALSE 0
    #define TRUE !(FALSE)

    #include <sys/socket.h>
    #include <linux/can.h>
    #include <time.h>
    #include <signal.h>

//    #define can_frame struct can_frame
#endif

/** @}*/

/**
 * @typedef  result_t
 * @brief    Return type from API function call. Negative on error condition
 */
typedef int16_t result_t;

/**
 * @ingroup Core
 * @fn      libesoup_init()
 * @brief   Function to initialise the libesoup library.
 *
 * The configuraiton required should be defined using switches in the
 * configuraiton file libesoup_config.h. An example of that configuration file
 * is included in libesoup/examples directory.
 *
 * In addition example projects are included in libesoup/examples/projects. 
 * Each example project contains the necessary configuration file for the project.
 *
 * This function must be called prior to calling any libesoup API functions. 
 * The function calls each individual module initialisation function depending
 * on the code included in the build, as specified by the configuration file
 * libesoup_cofig.h. The fact that the function has been called is *NOT* 
 * checked by API Functions, this is an embedded system and performing a check
 * on each API call is viewed as a waste of clock cycles. 
 */
extern result_t libesoup_init(void);

#endif // _CORE_H
