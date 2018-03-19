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

#define ERR_GENERAL_ERROR          1    /* Unspecified error occured in libesoup API Function execution.*/
#define ERR_BAD_INPUT_PARAMETER    2    /* Bad parameter passed to libesoup API funtion. */
#define ERR_BUFFER_OVERFLOW        3
#define ERR_RANGE_ERROR            4    /* Error in the range of a parameter.*/
#define ERR_TIMER_ACTIVE           5    /* Attempt to start a timer which is already active.*/
#define ERR_NO_RESOURCES           6    /* No resources available to execute the requested libesoup API function. */
#define ERR_ADDRESS_RANGE          7    /* An address passed to an libesoup API function is out of valid range.*/
#define ERR_NOT_READY              8    /* API function no ready to execute request.*/
#define ERR_CAN_ERROR              9    /* Error in CAN libesoup API function.*/
#define ERR_CAN_NOT_CONNECTED     10    /* Attempt to perform action on CAN Bus prior to its connection.*/
#define ERR_CAN_INVALID_BAUDRATE  11    /* Invalid Baudrate specified in libesoup API funciton call.*/
#define ERR_CAN_NO_FREE_BUFFER    12    /* No free CAN buffer to complete request.*/
#define ERR_UNINITIALISED         13    /* Attempt to call libesoup API function prior to initialisation.*/
#define ERR_BUSY                  14    /* I'm busy call back later.*/
#define ERR_NOTHING_TO_DO         15    /* Call results in no action being taken.*/
#define ERR_NO_RESPONSE           16    /* No response from the attempted API call.*/

#endif // _ERRNO_H
