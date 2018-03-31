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
#ifndef _ES_dsPIC33EP256MU806_H
#define _ES_dsPIC33EP256MU806_H

#include <xc.h>

#define DISABLED    0
#define ENABLED     1

#define OFF         0
#define ON          1


#define INTERRUPTS_DISABLED    INTCON2bits.GIE = 0;
#define INTERRUPTS_ENABLED     INTCON2bits.GIE = 1;

/*
 * DMA 
 */

#define ECAN1_TX    0b01000110

#define TIMER_1                           0
#define TIMER_2                           1
#define TIMER_3                           2
#define TIMER_4                           3
#define TIMER_5                           4

#define NUMBER_HW_TIMERS                  5

/*
 * Real Time Clock / Calendar Module
 */
#define DATETIME_WRITE  __builtin_write_RTCWEN();
#define DATETIME_LOCK   RCFGCALbits.RTCWREN = DISABLED;

/*
 * According to Microchip DS70584 RTCWREN has to be set to change RTCEN
 */
#define RTCC_ON    __builtin_write_RTCWEN();    \
                   RCFGCALbits.RTCEN = ENABLED;     \
 	               RCFGCALbits.RTCWREN = DISABLED; 

#define RTCC_OFF   __builtin_write_RTCWEN();    \
                   RCFGCALbits.RTCEN = DISABLED;    \
	               RCFGCALbits.RTCWREN = DISABLED;

#define VALUE_POINTER   RCFGCALbits.RTCPTR

#define RTC_ALARM_EVERY_HALF_SECOND  0b0000
#define RTC_ALARM_EVERY_SECOND       0b0001
#define RTC_ALARM_EVERY_TEN_SECONDS  0b0010
#define RTC_ALARM_EVERY_MINUTE       0b0011
#define RTC_ALARM_EVERY_TEN_MINUTES  0b0100
#define RTC_ALARM_EVERY_TEN_HOUR     0b0101
#define RTC_ALARM_EVERY_TEN_DAY      0b0110
#define RTC_ALARM_EVERY_TEN_WEEK     0b0111
#define RTC_ALARM_EVERY_TEN_MONTH    0b1000
#define RTC_ALARM_EVERY_TEN_YEAR     0b1001

#define ALARM_ON	    ALCFGRPTbits.ALRMEN = ENABLED;
#define ALARM_OFF	    ALCFGRPTbits.ALRMEN = DISABLED;
#define ALARM_POINTER   ALCFGRPTbits.ALRMPTR
#define ALARM_CHIME     ALCFGRPTbits.CHIME
#define ALARM_REPEAT    ALCFGRPTbits.ARPT

#define RTCC_ISR_FLAG     IFS3bits.RTCIF
#define RTCC_ISR_PRIOTITY IPC15bits.RTCIP
#define RTCC_ISR_ENABLE   IEC3bits.RTCIE

#define RTCC_PIN          RCFGCALbits.RTCOE

/*
 * UART Settings.
 */
/*
 * UART_1
 */
#define U1_ENABLE          U1MODEbits.UARTEN

#define U1_RX_ISR_FLAG     IFS0bits.U1RXIF
#define U1_RX_ISR_PRIOTITY IPC2bits.U1RXIP
#define U1_RX_ISR_ENABLE   IEC0bits.U1RXIE

#define U1_TX_ISR_FLAG     IFS0bits.U1TXIF
#define U1_TX_ISR_PRIOTITY IPC3bits.U1TXIP
#define U1_TX_ISR_ENABLE   IEC0bits.U1TXIE

/*
 * UART_2
 */
#define U2_ENABLE          U2MODEbits.UARTEN

#define U2_RX_ISR_FLAG     IFS1bits.U2RXIF
#define U2_RX_ISR_PRIOTITY IPC7bits.U2RXIP
#define U2_RX_ISR_ENABLE   IEC1bits.U2RXIE

#define U2_TX_ISR_FLAG     IFS1bits.U2TXIF
#define U2_TX_ISR_PRIOTITY IPC7bits.U2TXIP
#define U2_TX_ISR_ENABLE   IEC1bits.U2TXIE

/*
 * UART_3
 */
#define U3_ENABLE          U3MODEbits.UARTEN

#define U3_RX_ISR_FLAG     IFS5bits.U3RXIF
#define U3_RX_ISR_PRIOTITY IPC20bits.U3RXIP
#define U3_RX_ISR_ENABLE   IEC5bits.U3RXIE

#define U3_TX_ISR_FLAG     IFS5bits.U3TXIF
#define U3_TX_ISR_PRIOTITY IPC20bits.U3TXIP
#define U3_TX_ISR_ENABLE   IEC5bits.U3TXIE

/*
 * UART_4
 */
#define U4_ENABLE          U4MODEbits.UARTEN

#define U4_RX_ISR_FLAG     IFS5bits.U4RXIF
#define U4_RX_ISR_PRIOTITY IPC22bits.U4RXIP
#define U4_RX_ISR_ENABLE   IEC5bits.U4RXIE

#define U4_TX_ISR_FLAG     IFS5bits.U4TXIF
#define U4_TX_ISR_PRIOTITY IPC22bits.U4TXIP
#define U4_TX_ISR_ENABLE   IEC5bits.U4TXIE

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
 * CAN Bus Modes
 */
typedef enum { 
    normal      = 0b000,
    disabled    = 0b001,
    loopback    = 0b010,
    listen_only = 0b011,
    config      = 0b100,
    listen_all  = 0b111,
} ty_can_mode;

/*
 * 
 */
enum pin_t {
    RB0,
    RB1,
    RB2,
    RB3,
    RB4,
    RB5,
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

    RC12,
    RC13,
    RC14,
    RC15,

    RD0,
    RD1,
    RD2,
    RD3,
    RD4,
    RD5,
    RD6,
    RD7,
    RD8,
    RD9,
    RD10,
    RD11,

    RE0,
    RE1,
    RE2,
    RE3,
    RE4,
    RE5,
    RE6,
    RE7,

    RF0,
    RF1,
    RF3,
    RF4,
    RF5,
    
    RG2,
    RG3,
    RG6,
    RG7,
    RG8,
    RG9,

    INVALID_PIN = 0xff,
};

/*
 * Clock Sources:
 * 
 * See Microchip document DS70005131
 */
#define dsPIC33_PRIMARY_OSCILLATOR      0b010
#define dsPIC33_PRIMARY_OSCILLATOR_PLL  0b011

#define SECONDARY_OSCILLATOR            OSCCONbits.LPOSCEN

/*
 * Flash parameters
 */
/**
 * @def   FLASH_PAGE_SIZE
 * @brief The size of Flash memory Pages.
 *
 * The Flash page size is 1024 Instructions, which is 3072 as each instruction
 * is 3 Bytes. But the Flash is addressed in Words so the length given here is
 * 0x800 (1024 * 2).
 *
 * Flash memory is erased on a Page by page basis.
 *
 * @def   FLASH_LAST_ADDRESS
 * @brief The last Address of Flash memory.
 *
 * @def   FLASH_NUM_INSTRUCTION_PER_ROW
 * @brief Flash is written row by row. This is the Row size of Flash Memory
 */
#define FLASH_PAGE_SIZE                0x800
#define FLASH_LAST_ADDRESS             0x2ABF9
#define FLASH_NUM_INSTRUCTION_PER_ROW  128

/**
 * @def   FLASH_FIRMWARE_START_ADDRESS
 * @brief Start of Firmware code in Flash Memory.
 *
 * The bootloader code occupies the lower portion of Flash memory. Firmware
 * starts at this address.
 *
 * @def   FLASH_APP_START_ADDRESS
 * @brief Start of the Application code in Flash Memory.
 *
 * If you use the electronicSoup SYS_CAN Node Operating System then the application 
 * Code starts at this address in Flash Memory.
 */
#define FLASH_FIRMWARE_START_ADDRESS   0x08800
#define FLASH_APP_START_ADDRESS        0x18000

/**
 * @def   FLASH_APP_HANDLE_PAGE
 * @brief Address of the Applications Handle page in Flash.
 *
 * Because of limitations in the PIC Architecture, specifically in how far in
 * Flash Memory a branching instruction can jump a page of Flash is reserved in
 * low memory for the handlers to be able to jump up to high memory of the 
 * Application's code.
 */
#define FLASH_APP_HANDLE_PAGE        0x400


extern void cpu_init(void);

#endif // _ES_dsPIC33EP256MU806_H
