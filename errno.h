/**
 * @file libesoup/core.h
 *
 * @author John Whitmore
 *
 * Error number definitions required by electronicSoup Code Library
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

#define RC_CHECK if(rc < 0) return(rc);
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
#define RC_CHECK_PRINT(x) if(rc < 0) LOG_E("x");
#endif

#define ERR_GENERAL_ERROR          1    /* Unspecified error occured in libesoup API Function execution.*/
#define ERR_BAD_INPUT_PARAMETER    2    /* Bad parameter passed to libesoup API funtion. */
#define ERR_NOT_CODED              3
#define ERR_BUFFER_OVERFLOW        4
#define ERR_RANGE_ERROR            5    /* Error in the range of a parameter.*/
#define ERR_TIMER_ACTIVE           6    /* Attempt to start a timer which is already active.*/
#define ERR_NO_RESOURCES           7    /* No resources available to execute the requested libesoup API function. */
#define ERR_ADDRESS_RANGE          8    /* An address passed to an libesoup API function is out of valid range.*/
#define ERR_NOT_READY              9    /* API function no ready to execute request.*/
#define ERR_CAN_ERROR             10    /* Error in CAN libesoup API function.*/
#define ERR_CAN_NOT_CONNECTED     11    /* Attempt to perform action on CAN Bus prior to its connection.*/
#define ERR_CAN_BITRATE_LOW       12
#define ERR_CAN_BITRATE_HIGH      13
#define ERR_CAN_INVALID_BAUDRATE  14    /* Invalid Baudrate specified in libesoup API funciton call.*/
#define ERR_CAN_NO_FREE_BUFFER    15    /* No free CAN buffer to complete request.*/
#define ERR_UNINITIALISED         16    /* Attempt to call libesoup API function prior to initialisation.*/
#define ERR_BUSY                  17    /* I'm busy call back later.*/
#define ERR_NOTHING_TO_DO         18    /* Call results in no action being taken.*/
#define ERR_NO_RESPONSE           19    /* No response from the attempted API call.*/

#endif // _ERRNO_H
