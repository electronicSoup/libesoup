/**
 *
 * @file libesoup/status/status.h
 *
 * @author John Whitmore
 * 
 * @brief Definitions for the status handler sub-system
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
 *******************************************************************************
 *
 */
#ifndef _STATUS_H
#define _STATUS_H

#include "libesoup_config.h"

#ifdef SYS_SYSTEM_STATUS

typedef enum {
#if defined(SYS_CAN_BUS)
	can_bus_status,        ///< Status update from CAN Bus Module
	can_bus_l2_status,     ///< Status update from CAN Bus Layer 2
#endif
#if defined(SYS_CAN_DCNCP)
    can_bus_dcncp_status,  ///< Status update from DCNCP Module
#endif
#if defined(ISO15765)
    iso15765_status,       ///< Status update from ISO15765 Module
#endif
#if defined(ISO11783)
    iso11783_status        ///< Status update from ISO11783 Module
#endif
} status_source_t;

/**
 * @brief Definition of a status handler in the system
 * @param source - The source of the status update
 * @param status - The status being reported
 * @param data   - Any data associated with the status update
 */
typedef void (*status_handler_t)(status_source_t source, int16_t status, int16_t data);

#endif // SYS_SYSTEM_STATUS

#endif // _STATUS_H

