/**
 *
 * @file es_lib/processors/es-PIC24FJ356GB106.h
 *
 * @author John Whitmore
 *
 * This file contains microcontroller specific definitions for the Microchip
 * PIC24FJ256GB106. 
 *
 * The es_lib library of source code expects a system.h header file to exist
 * in your include path. The file contains the various switches and definitions
 * which configure the various features of the library.
 *
 * Copyright 2015 John Whitmore <jwhitmore@electronicsoup.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _ES_PIC24FJ256GB106_H
#define _ES_PIC24FJ256GB106_H

//#include <p24Fxxxx.h>
#include <xc.h>

/**
 * \brief Identifers for the hardware timers in the pic24FG256GB106
 * 
 * These identifiers are used in es_lib/timers/hw_timers.c when managing the 
 * timers in the micro-controller. Different micro-controllers will use
 * different numbering, perhaps starting a timer 0.
 */
#define TIMER_1 0
#define TIMER_2 1
#define TIMER_3 2
#define TIMER_4 3
#define TIMER_5 4

/**
 * \brief Total number of Hardware timers present in the micro-controller
 */
#define NUMBER_HW_TIMERS  5

/*
 * Peripheral Input Pin Definitions
 */
#define RP0  0
#define RP1  1
#define RP13 13
#define RP20 20
#define RP23 23
#define RP25 25
#define RP28 28

/*
 * Peripheral Input functions
 */
#define SPI_1_DATA_INPUT RPINR20bits.SDI1R 
#define UART_1_RX        RPINR18bits.U1RXR

/*
 * Peripheral Output functions
 */
#define UART_1_TX             3
#define SPI_1_DATA_OUTPUT     7
#define SPI_1_CLOCK_OUTPUT    8

#endif // _ES_PIC24FJ256GB106_H
