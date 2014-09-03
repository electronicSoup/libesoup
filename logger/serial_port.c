/**
 *
 * \file es_can/logger/serial_port.c
 *
 * Functions for initialisation of the Serial Port.
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "system.h"

void serial_init(void)
{
	/*
	 * Serial Port pin configuration should be defined 
	 * in include file system.h
	 */
#if defined(SERIAL_PORT_GndTxRx)
	RPOR12bits.RP25R = 3;
#elif defined(SERIAL_PORT_GndRxTx)
	RPOR10bits.RP20R = 3;
#endif

	U1MODE = 0x8800;
	U1STA = 0x0410;

	/*
	 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
	 *
	 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
	 *
	 * UxBRG = ((CLOCK/SERIAL_BAUD)/16) -1
	 *
	 */
	U1BRG = ((CLOCK_FREQ / SERIAL_BAUD) / 16) - 1;
}

