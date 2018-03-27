/**
 *
 * libesoup/status/status.h
 *
 * Definitions for the status handler sub-system
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

#ifdef SYS_SYSTEM_STATUS

typedef enum {
#if defined(SYS_CAN_BUS)
	can_bus_status,
	can_bus_l2_status,
#endif
#if defined(SYS_CAN_DCNCP)
    can_bus_dcncp_status,
#endif
#if defined(ISO15765)
    iso15765_status,
#endif
#if defined(ISO11783)
    iso11783_status
#endif
} status_source_t;

union ty_status {
    struct {
        status_source_t source;
        uint8_t         status;
    } sstruct;
    uint16_t word;
};

typedef void (*status_handler_t)(union ty_status);

#endif // SYS_SYSTEM_STATUS

#endif // _STATUS_H

