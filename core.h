/**
 * @file libesoup/core.h
 *
 * @author John Whitmore
 *
 * Core definitions required by electronicSoup Code Library
 *
 * Copyright 2017 - 2018 electronicSoup Limited
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

/*
 * include stdint.h to get access to uint8_t style types
 */
#if defined(XC16) || defined(__XC8)
    #include <stdint.h>     // For uintx_t types

/*
 * The actual Instruction/Peripheral Clock frequency being used by the system
 * as opposed to the requested Clock Frequence SYS_CLOCK_FREQ
 */
extern uint32_t sys_clock_freq;


#ifndef	NULL
#define NULL (0)
#endif	/* NULL */

    typedef uint8_t    boolean;
#define FALSE 0x00
#define TRUE (!FALSE)

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

#if 0  // Tidy up 20171117 - don't think this is used.
/*
 * Union for converting byte stream to floats
 */
typedef union {
	uint8_t bytes[4];
	float value;
} f32;
#endif // 0

/** @defgroup Core Core Definitions
 *  @{
 * 
 * @enum  result_t
 * @brief Error codes returned by libesoup API Functions.
 * 
 * All libesoup API functions, if they are not of type void, return a return
 * code indicating the result of the requested API function call.
 * 
 * All user code is free to use these return codes or define their own
 * enumerated type for user code return codes.
 */
typedef enum {
    SUCCESS = 0x00,           /**< API function executed without error. */
    ERR_GENERAL_ERROR,        /**< Unspecified error occured in libesoup API Function execution.*/
    ERR_BAD_INPUT_PARAMETER,  /**< Bad parameter passed to libesoup API funtion. */
    ERR_BUFFER_OVERFLOW,
    ERR_RANGE_ERROR,          /**< Error in the range of a parameter.*/
    ERR_TIMER_ACTIVE,         /**< Attempt to start a timer which is already active.*/
    ERR_NO_RESOURCES,         /**< No resources available to execute the requested libesoup API function. */
    ERR_ADDRESS_RANGE,        /**< An address passed to an libesoup API function is out of valid range.*/
    ERR_NOT_READY,            /**< API function no ready to execute request.*/
    ERR_CAN_ERROR,            /**< Error in CAN libesoup API function.*/
    ERR_CAN_NOT_CONNECTED,    /**< Attempt to perform action on CAN Bus prior to its connection.*/
    ERR_CAN_INVALID_BAUDRATE, /**< Invalid Baudrate specified in libesoup API funciton call.*/
    ERR_CAN_NO_FREE_BUFFER,   /**< No free CAN buffer to complete request.*/
    ERR_UNINITIALISED,        /**< Attempt to call libesoup API function prior to initialisation.*/
    ERR_BUSY,                 /**< I'm busy call back later.*/
    ERR_NOTHING_TO_DO,        /**< Call results in no action being taken.*/
    ERR_NO_RESPONSE           /**< No response from the attempted API call.*/
} result_t;

/** @}*/

/**
 * \ingroup Core
 * \function libesoup_init()
 * \brief Function to initialise the libesoup library. The configuraiton required 
 * should be defined using switches in the configuraiton file libesoup_config.h. An
 * example of that configuration file is included in libesoup/examples directory.
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
