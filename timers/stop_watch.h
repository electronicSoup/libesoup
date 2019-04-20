/**
 * @file libesoup/timers/stop_watch.h
 *
 * @author John Whitmore
 *
 * @brief API definitions and function prototypes for stop watch 
 *        (incrementing timers)
 *
 * Copyright 2019 electronicSoup Limited
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
#ifndef _STOP_WATCH_H
#define _STOP_WATCH_H

/*
 * This API Code is only included in a build if the configuration file
 * libesoup_config.h includes the definition SYS_STOP_WATCH_TIMERS
 */
#ifdef SYS_STOP_WATCH_TIMERS

#include "libesoup/errno.h"
#include "libesoup/timers/time.h"
#include "libesoup/timers/hw_timers.h"

#endif // SYS_STOP_WATCH_TIMERS

#endif // _STOP_WATCH_H
