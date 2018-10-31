/**
 *
 * @file libesoup/utils/rand.h
 *
 * @author John Whitmore
 *
 * @brief prototype of the random initialisation function.
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

#ifndef _RAND_H
#define _RAND_H

#include "libesoup_config.h"

#ifdef SYS_RAND

extern void random_init(void);

/*
 * At present rans() is provided by Microchip Library
 */
extern int rand(void);

#endif // SYS_RAND
#endif // _RAND_H
