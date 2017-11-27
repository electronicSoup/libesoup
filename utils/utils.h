/**
 *
 * \file libesoup/utils/utils.h
 *
 * General utility functions of the electronicSoup CAN code Library
 *
 * Copyright 2017 electronicSoup Limited
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

#ifndef UTILS_H
#define UTILS_H

#include "libesoup/core.h"
#include "libesoup_config.h"

#ifdef HEARTBEAT
extern void heartbeat_on(union sigval data);
extern void heartbeat_off(union sigval data);
#endif

#if 0
extern BYTE generate_hash(char *string);
#endif

#endif //UTILS_H
