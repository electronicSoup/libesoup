/**
 *
 * @file libesoup/processors/es-PIC24FJ256GB106.h
 *
 * @author John Whitmore
 *
 * This file contains microcontroller specific definitions for the Microchip
 * PIC24FJ256GB106. 
 *
 * The libesoup library of source code expects a libesoup_config.h header file to exist
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

#define OUTPUT_PIN  0
#define INPUT_PIN   1

#define ANALOG_PIN  0
#define DIGITAL_PIN 1

#define DISABLE     0
#define ENABLE      1

/**
 * \brief Identifers for the hardware timers in the pic24FG256GB106
 * 
 * These identifiers are used in libesoup/timers/hw_timers.c when managing the 
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
 * UART Settings.
 */
#define U1_ENABLE        U1MODEbits.UARTEN
#define U1_RX_ISR_FLAG   IFS0bits.U1RXIF
#define U1_TX_ISR_FLAG   IFS0bits.U1TXIF
#define U1_RX_ISR_ENABLE IEC0bits.U1RXIE
#define U1_TX_ISR_ENABLE IEC0bits.U1TXIE

#define U2_ENABLE        U2MODEbits.UARTEN
#define U2_RX_ISR_FLAG   IFS1bits.U2RXIF
#define U2_TX_ISR_FLAG   IFS1bits.U2TXIF
#define U2_RX_ISR_ENABLE IEC1bits.U2RXIE
#define U2_TX_ISR_ENABLE IEC1bits.U2TXIE

#define U3_ENABLE        U3MODEbits.UARTEN
#define U3_RX_ISR_FLAG   IFS5bits.U3RXIF
#define U3_TX_ISR_FLAG   IFS5bits.U3TXIF
#define U3_RX_ISR_ENABLE IEC5bits.U3RXIE
#define U3_TX_ISR_ENABLE IEC5bits.U3TXIE

#define U4_ENABLE        U4MODEbits.UARTEN
#define U4_RX_ISR_FLAG   IFS5bits.U4RXIF
#define U4_TX_ISR_FLAG   IFS5bits.U4TXIF
#define U4_RX_ISR_ENABLE IEC5bits.U4RXIE
#define U4_TX_ISR_ENABLE IEC5bits.U4TXIE

#define UARTEN_MASK      0x8000
#define USIDL_MASK       0x2000
#define IREN_MASK        0x1000
#define RTSMD_MASK       0x0800
#define UEN1_MASK        0x0200
#define UEN0_MASK        0x0100
#define WAKE_MASK        0x0080
#define LPBACK_MASK      0x0040
#define ABAUD_MASK       0x0020
#define RXINV_MASK       0x0010
#define BRGH_MASK        0x0008
#define PDSEL1_MASK      0x0004
#define PDSEL0_MASK      0x0002
#define STSEL_MASK       0x0001

#define NUM_UARTS        4
#define UART_1           0x00
#define UART_2           0x01
#define UART_3           0x02
#define UART_4           0x03



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
#define PPS_UART_1_RX    RPINR18bits.U1RXR
#define	PPS_UART_2_RX    RPINR19bits.U2RXR
#define	PPS_UART_3_RX    RPINR17bits.U3RXR
#define PPS_UART_4_RX    RPINR27bits.U4RXR

/*
 * Peripheral Output functions
 */
#define PPS_UART_1_TX         3
#define PPS_UART_2_TX         5
#define SPI_1_DATA_OUTPUT     7
#define SPI_1_CLOCK_OUTPUT    8
#define PPS_UART_3_TX        28
#define PPS_UART_4_TX        30



extern void cpu_init(void);

#endif // _ES_PIC24FJ256GB106_H
