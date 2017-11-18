/**
 *
 * @file libesoup/processors/es-dsPIC33EP256MU806.h
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
#ifndef _ES_dsPIC33EP256MU806_H
#define _ES_dsPIC33EP256MU806_H

#include <xc.h>

#define OUTPUT_PIN  0
#define INPUT_PIN   1

#define DIGITAL_PIN 0
#define ANALOG_PIN  1

#define DISABLED    0
#define ENABLED     1

#define OFF         0
#define ON          1

#define TIMER_1                           0
#define TIMER_2                           1
#define TIMER_3                           2
#define TIMER_4                           3
#define TIMER_5                           4

#define NUMBER_HW_TIMERS                  5

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
 * 
 */
enum pin_t {
    RE5,
    RE6,
    RE7,
    RG6,
    RG7,
    RG8,
    RG9,
    RB5,
    RB4,
    RB3,
    RB2,
    RB1,
    RB0,
    RB6,
    RB7,
    RB8,
    RB9,
    RB10,
    RB11,
    RB12,
    RB13,
    RB14,
    RB15,
    RF4,
    RF5,
    RF3,
    RG3,
    RG2,
    RC12,
    RC15,
    RD8,
    RD9,
    RD10,
    RD11,
    RD0,
    RC13,
    RC14,
    RD1,
    RD2,
    RD3,
    RD4,
    RD5,
    RD6,
    RD7,
    RF0,
    RF1,
    RE0,
    RE1,
    RE2,
    RE3,
    RE4,
    INVALID_PIN = 0xff,
};

/**
 * \ingroup Core
 * \bried Microchip Peripheral Input Pins
 */
typedef enum { 
    RP0   = 0,
    RP1   = 1,
    RP13  = 13,
    RP20  = 20,
    RP23  = 23,
    RP25  = 25,
    RP28  = 28,
    RP64  = 64,
    RPI74 = 74,
    RP118 = 118,
    RP120 = 120,
} ty_peripheral_pin;

/*
 * Peripheral Input functions
 */
#define PPS_UART_1_RX                   RPINR18bits.U1RXR
#define PPS_UART_2_RX                   RPINR19bits.U2RXR
#define PPS_UART_3_RX                   RPINR27bits.U3RXR
#define PPS_UART_4_RX                   RPINR28bits.U4RXR
 
/*
 * Peripheral Output functions
 */
#define PPS_UART_1_TX                   0x01
#define PPS_UART_2_TX                   0x03
#define PPS_UART_3_TX                   0x1B
#define PPS_UART_4_TX                   0x1D
 
#define PPS_RP64                        RPOR0bits.RP64R
#define PPS_RP118                       RPOR13bits.RP118R
#define PPS_RP120                       RPOR14bits.RP120R
    
/*
 * Clock Sources:
 * 
 * See Microchip document DS70005131
 */
#define dsPIC33_PRIMARY_OSCILLATOR      0b010
#define dsPIC33_PRIMARY_OSCILLATOR_PLL  0b011

extern void cpu_init(void);

#endif // _ES_dsPIC33EP256MU806_H
