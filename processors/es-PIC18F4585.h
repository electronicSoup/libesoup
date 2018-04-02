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
 * Copyright 2017-2018 electronicSoup Limited
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

#define DISABLED                0   ///< Disable a uC Feature 
#define ENABLED                 1   ///< Enable a uC Feature 


/*
 * Enabling & Disabling Interrupts
 */
#define INTERRUPTS_ENABLED      INTCONbits.GIE = 1;  ///< Enable uC Interrupts
#define INTERRUPTS_DISABLED     INTCONbits.GIE = 0;  ///< Disable uC Interrupts

/**
 * @brief Hardware timers in the uC
 */
#define NUMBER_HW_TIMERS        2   ///< Number of Hardware timers the uC
#define TIMER_0                 0   ///< Index/Identifier for Hardware Timer 0
#define TIMER_1                 1   ///< Index/Identifier for Hardware Timer 1

#define NUM_UARTS               1   ///< Number of UARTs in the uC
#define UART_1                  0   ///< Index/Identifier for UART 1 

#define U1_ENABLE               RCSTAbits.SPEN   ///< SFR Bit to enable UART 1
#define U1_RX_ISR_ENABLE        RCSTAbits.CREN   ///< SFR Bit to enable UART 1 Recieve Interrupt
#define U1_TX_ISR_ENABLE        TXSTAbits.TXEN   ///< SFR Bit to enable UART 1 Transmit Interrupt

/**
 * @brief Serial logging interface Pin defintions
 *
 * In other micro-controllers these definitions are in the board file
 * as the Peripheral pin select functionality can be used to configure the
 * required pin. In the PIC18 however the UART pins are hard wired and cannot
 * be configured at all.
 */
#define BRD_SERIAL_LOGGING_TX_PIN   PRC6  ///< UART 1 TX Pin
#define BRD_SERIAL_LOGGING_RX_PIN   PRC7  ///< UART 1 RX Pin

/**
 * @brief GPIO pins on the pic18f4585
 *
 * Unfortunately Microchip's header files already define RB0 etc. which is used
 * by other libesoup target micro-controllers so in this case have to prepend a
 * 'P' to make PRB0 etc, so that there's not a namespace clash.
 * 
 */
enum pin_t {
	PRA0,    ///< Port A Bit 0
	PRA1,    ///< Port A Bit 1
	PRA2,    ///< Port A Bit 2
	PRA3,    ///< Port A Bit 3
	PRA4,    ///< Port A Bit 4
	PRA5,    ///< Port A Bit 5
	PRA6,    ///< Port A Bit 6
	PRA7,    ///< Port A Bit 7
    
	PRB0,    ///< Port B Bit 0
	PRB1,    ///< Port B Bit 1
	PRB2,    ///< Port B Bit 2
	PRB3,    ///< Port B Bit 3
	PRB4,    ///< Port B Bit 4
	PRB5,    ///< Port B Bit 5
	PRB6,    ///< Port B Bit 6
	PRB7,    ///< Port B Bit 7

	PRC0,    ///< Port C Bit 0
	PRC1,    ///< Port C Bit 1
	PRC2,    ///< Port C Bit 2
	PRC3,    ///< Port C Bit 3
	PRC4,    ///< Port C Bit 4
	PRC5,    ///< Port C Bit 5
	PRC6,    ///< Port C Bit 6
	PRC7,    ///< Port C Bit 7
    
	PRD0,    ///< Port D Bit 0
	PRD1,    ///< Port D Bit 1
	PRD2,    ///< Port D Bit 2
	PRD3,    ///< Port D Bit 3
	PRD4,    ///< Port D Bit 4
	PRD5,    ///< Port D Bit 5
	PRD6,    ///< Port D Bit 6
	PRD7,    ///< Port D Bit 7
    
	PRE0,    ///< Port E Bit 0
	PRE1,    ///< Port E Bit 1
	PRE2,    ///< Port E Bit 2
	PRE3,    ///< Port E Bit 3
    
	INVALID_PIN = 0xff,   ///< Dummy value to indicate no GPIO Pin
};

/**
 * @brief micro-controller specific initialisation code
 */
extern void cpu_init(void);

#endif // _ES_PIC18F2680_H
