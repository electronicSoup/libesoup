/**
 *
 * @file libesoup/processors/es-PIC18F4585.h
 *
 * @author John Whitmore
 *
 * This file contains an example libesoup libesoup_config.h configuration file. 
 *
 * The libesoup library of source code expects a libesoup_config.h header file to exist
 * in your include path. The file contains the various switches and definitions
 * which configure the various features of the library.
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
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
#ifndef _ES_PIC18F4585_H
#define _ES_PIC18F4585_H

//#include <p18cxxx.h>
#include <xc.h>

#define OUTPUT_PIN      0
#define INPUT_PIN       1

#define DISABLE         0
#define ENABLE          1
/*
 * I/O Pin Definitions
 */
#define RC6            25
#define RC7            26

#define TIMER_0         0
#define TIMER_1         1

#define NUM_UARTS       1
#define UART_1       0x00

#define U1_ENABLE          RCSTAbits.SPEN
#define U1_RX_ISR_ENABLE   RCSTAbits.CREN
#define U1_TX_ISR_ENABLE   TXSTAbits.TXEN

#define SERIAL_LOGGING_TX_PIN RC6
#define SERIAL_LOGGING_RX_PIN RC7

#define NUMBER_HW_TIMERS  2

extern void cpu_init(void);

#endif // _ES_PIC18F2680_H
