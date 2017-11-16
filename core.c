/*
 *
 * libesoup/core.c
 *
 * File containing the function to initialise the libesoup library
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "libesoup_config.h"

#ifdef SYS_HW_TIMERS
#include "libesoup/timers/hw_timers.h"
#endif

#ifdef SYS_SW_TIMERS
#include "libesoup/timers/sw_timers.h"
#endif

#ifdef SYS_UART
#include "libesoup/comms/uart/uart.h"
#endif

#ifdef SYS_SPI_BUS
#include "libesoup/utils/spi.h"
#endif


void libesoup_init(void)
{
	cpu_init();

#ifdef SYS_HW_TIMERS
	hw_timer_init();
#endif
	
#ifdef SYS_SW_TIMERS
	sw_timer_init();
#endif

#ifdef SYS_UART
	uart_init();
#endif
	
#ifdef SYS_SPI_BUS
        spi_init();
#endif
	
}
