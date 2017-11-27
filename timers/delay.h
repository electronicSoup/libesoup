/**
 *
 * \file libesoup/timerss/delay.h
 *
 * \brief Functionality for delaying similar to a loop.
 * Microcotroller will spin
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
/**
 * \name Delay Delay 
 */
/**@{*/

/**
 * \ingroup Timers
 * \function delay
 * \brief  Delay the uC for a duration
 * @param units  Time units of the duration parameter \ref ty_time_units
 * @param duration Duration fo the delay 
 */
extern void delay(ty_time_units units, uint16_t duration);

/**}@*/ 
