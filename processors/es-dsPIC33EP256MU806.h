/**
 *
 * @file libesoup/processors/es-dsPIC33EP256MU806.h
 *
 * @author John Whitmore
 *
 * @brief Definitions for the dsPIC33EP256MU806 micro-controller
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

#define DISABLED    0   ///< Disable a uC Feature 
#define ENABLED     1   ///< Enable a uC Feature

#define OFF         0   ///< Turn off a uC Feature
#define ON          1   ///< Turn on a uC Feature

#define INTERRUPTS_DISABLED    INTCON2bits.GIE = 0; ///< Disable uC Interrupts
#define INTERRUPTS_ENABLED     INTCON2bits.GIE = 1; ///< Enable uC Interrups

/**
 * @brief Hardware timers in the dsPIC33EP256MU806 uC
 */
#define TIMER_1                0     ///< Index for HW Timer 1 in the system
#define TIMER_2                1     ///< Index for HW Timer 2 in the system
#define TIMER_3                2     ///< Index for HW Timer 3 in the system
#define TIMER_4                3     ///< Index for HW Timer 4 in the system
#define TIMER_5                4     ///< Index for HW Timer 5 in the system

#define NUMBER_HW_TIMERS       5     ///< Number of HW Timers in the system

#ifdef SYS_RTC
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
#endif // SYS_RTC

/**
 * @brief UART Settings.
 */
#define NUM_UARTS          4                   ///< Number of UARTS in the uC
#define UART_1             0x00                ///< Index/Identifier used for UART 1
#define UART_2             0x01                ///< Index/Identifier used for UART 2
#define UART_3             0x02                ///< Index/Identifier used for UART 3
#define UART_4             0x03                ///< Index/Identifier used for UART 4

#define U1_ENABLE          U1MODEbits.UARTEN   ///< UART 1 Enable SFR bit

#define U1_RX_ISR_FLAG     IFS0bits.U1RXIF     ///< UART 1 Recieve Interrupt Flag SFR Bit
#define U1_RX_ISR_PRIOTITY IPC2bits.U1RXIP     ///< UART 1 Recieve Interrupt Priority SFR
#define U1_RX_ISR_ENABLE   IEC0bits.U1RXIE     ///< UART 1 Recieve Interrupt Enable SFR Bit

#define U1_TX_ISR_FLAG     IFS0bits.U1TXIF     ///< UART 1 Transmit Interrupt FLAG SFR Bit
#define U1_TX_ISR_PRIOTITY IPC3bits.U1TXIP     ///< UART 1 Transmit Interrupt Priority SFR
#define U1_TX_ISR_ENABLE   IEC0bits.U1TXIE     ///< UART 1 Transmit Interrupt Enable SFR Bit

#define U2_ENABLE          U2MODEbits.UARTEN   ///< UART 2 Enable SFR bit

#define U2_RX_ISR_FLAG     IFS1bits.U2RXIF     ///< UART 2 Recieve Interrupt Flag SFR Bit
#define U2_RX_ISR_PRIOTITY IPC7bits.U2RXIP     ///< UART 2 Recieve Interrupt Priority SFR
#define U2_RX_ISR_ENABLE   IEC1bits.U2RXIE     ///< UART 2 Recieve Interrupt Enable SFR Bit

#define U2_TX_ISR_FLAG     IFS1bits.U2TXIF     ///< UART 2 Transmit Interrupt FLAG SFR Bit
#define U2_TX_ISR_PRIOTITY IPC7bits.U2TXIP     ///< UART 2 Transmit Interrupt Priority SFR
#define U2_TX_ISR_ENABLE   IEC1bits.U2TXIE     ///< UART 2 Transmit Interrupt Enable SFR Bit

#define U3_ENABLE          U3MODEbits.UARTEN   ///< UART 3 Enable SFR bit

#define U3_RX_ISR_FLAG     IFS5bits.U3RXIF     ///< UART 3 Recieve Interrupt Flag SFR Bit
#define U3_RX_ISR_PRIOTITY IPC20bits.U3RXIP    ///< UART 3 Recieve Interrupt Priority SFR
#define U3_RX_ISR_ENABLE   IEC5bits.U3RXIE     ///< UART 3 Recieve Interrupt Enable SFR Bit

#define U3_TX_ISR_FLAG     IFS5bits.U3TXIF     ///< UART 3 Transmit Interrupt FLAG SFR Bit
#define U3_TX_ISR_PRIOTITY IPC20bits.U3TXIP    ///< UART 3 Transmit Interrupt Priority SFR
#define U3_TX_ISR_ENABLE   IEC5bits.U3TXIE     ///< UART 3 Transmit Interrupt Enable SFR Bit

#define U4_ENABLE          U4MODEbits.UARTEN   ///< UART 4 Enable SFR bit

#define U4_RX_ISR_FLAG     IFS5bits.U4RXIF     ///< UART 4 Recieve Interrupt Flag SFR Bit
#define U4_RX_ISR_PRIOTITY IPC22bits.U4RXIP    ///< UART 4 Recieve Interrupt Priority SFR
#define U4_RX_ISR_ENABLE   IEC5bits.U4RXIE     ///< UART 4 Recieve Interrupt Enable SFR Bit

#define U4_TX_ISR_FLAG     IFS5bits.U4TXIF     ///< UART 4 Transmit Interrupt FLAG SFR Bit
#define U4_TX_ISR_PRIOTITY IPC22bits.U4TXIP    ///< UART 4 Transmit Interrupt Priority SFR
#define U4_TX_ISR_ENABLE   IEC5bits.U4TXIE     ///< UART 4 Transmit Interrupt Enable SFR Bit

/**
 * @brief UART Modes of operation bit field for the UxMODE SFR
 */
/**
 * @def UEN1_MASK
 * @def UEN0_MASK
 * @brief UEN1:UEN0: UARTx Enable bits
 *  11 = UxTX, UxRX and BCLKx pins are enabled and used; UxCTS pin controlled by port latches
 *  10 = UxTX, UxRX, UxCTS and UxRTS pins are enabled and used
 *  01 = UxTX, UxRX and UxRTS pins are enabled and used; UxCTS pin controlled by port latches
 *  00 = UxTX and UxRX pins are enabled and used; UxCTS and UxRTS/BCLKx pins controlled by port latches
 */
/**
 * @def PDSEL1_MASK
 * @def PDSEL0_MASK
 * @brief PDSEL<1:0>: Parity and Data Selection bits
 *  11 = 9-bit data, no parity
 *  10 = 8-bit data, odd parity
 *  01 = 8-bit data, even parity
 *  00 = 8-bit data, no parity
 */
#define UARTEN_MASK      0x8000   ///< UART Enable BIT
#define USIDL_MASK       0x2000   ///< Stop in Idle Mode bit
#define IREN_MASK        0x1000   ///< IrDA Encoder and Decoder Enable bit
#define RTSMD_MASK       0x0800   ///< Mode Selection for UxRTS Pin bit
#define UEN1_MASK        0x0200
#define UEN0_MASK        0x0100
#define WAKE_MASK        0x0080   ///< Wake-up on Start Bit Detect During Sleep Mode Enable bit
#define LPBACK_MASK      0x0040   ///< UARTx Loopback Mode Select bit
#define ABAUD_MASK       0x0020   ///< Auto-Baud Enable bit
#define RXINV_MASK       0x0010   ///< Receive Polarity Inversion bit
#define BRGH_MASK        0x0008   ///< High Baud Rate Enable bit
#define PDSEL1_MASK      0x0004 
#define PDSEL0_MASK      0x0002
#define STSEL_MASK       0x0001   ///< Stop Bit Selection bit

/**
 * @brief GPIO Pins available in the uC
 */
enum pin_t {
	RB0,     ///< Port B Bit 0
	RB1,     ///< Port B Bit 1
	RB2,     ///< Port B Bit 2
	RB3,     ///< Port B Bit 3
	RB4,     ///< Port B Bit 4
	RB5,     ///< Port B Bit 5
	RB6,     ///< Port B Bit 6
	RB7,     ///< Port B Bit 7
	RB8,     ///< Port B Bit 8
	RB9,     ///< Port B Bit 9
	RB10,    ///< Port B Bit 10
	RB11,    ///< Port B Bit 11
	RB12,    ///< Port B Bit 12
	RB13,    ///< Port B Bit 13
	RB14,    ///< Port B Bit 14
	RB15,    ///< Port B Bit 15

	RC12,    ///< Port C Bit 12
	RC13,    ///< Port C Bit 13
	RC14,    ///< Port C Bit 14
	RC15,    ///< Port C Bit 15

	RD0,     ///< Port D Bit 0
	RD1,     ///< Port D Bit 1
	RD2,     ///< Port D Bit 2
	RD3,     ///< Port D Bit 3
	RD4,     ///< Port D Bit 4
	RD5,     ///< Port D Bit 5
	RD6,     ///< Port D Bit 6
	RD7,     ///< Port D Bit 7
	RD8,     ///< Port D Bit 8
	RD9,     ///< Port D Bit 9
	RD10,    ///< Port D Bit 10
	RD11,    ///< Port D Bit 11

	RE0,     ///< Port E Bit 0
	RE1,     ///< Port E Bit 1
	RE2,     ///< Port E Bit 2
	RE3,     ///< Port E Bit 3
	RE4,     ///< Port E Bit 4
	RE5,     ///< Port E Bit 5
	RE6,     ///< Port E Bit 6
	RE7,     ///< Port E Bit 7

	RF0,     ///< Port F Bit 0
	RF1,     ///< Port F Bit 1
	RF3,     ///< Port F Bit 3
	RF4,     ///< Port F Bit 4
	RF5,     ///< Port F Bit 5
    
	RG2,     ///< Port G Bit 2
	RG3,     ///< Port G Bit 3
	RG6,     ///< Port G Bit 6
	RG7,     ///< Port G Bit 7
	RG8,     ///< Port G Bit 8
	RG9,     ///< Port G Bit 9

	INVALID_PIN = 0xff,     ///< Dummy Value used to represent no GPIO Pin
};

/**
 * @brief uC Clock Sources:
 * 
 * See Microchip document DS70005131
 */
#define dsPIC33_PRIMARY_OSCILLATOR      0b010    ///< Primary Oscillator without PLL
#define dsPIC33_PRIMARY_OSCILLATOR_PLL  0b011    ///< Primary Oscillator with PLL

#define SECONDARY_OSCILLATOR            OSCCONbits.LPOSCEN   ///< Secondary Oscillator enable SFR Bit

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

/**
 * @brief micro-controller specific initialisation code
 */
extern void cpu_init(void);

#endif // _ES_dsPIC33EP256MU806_H
