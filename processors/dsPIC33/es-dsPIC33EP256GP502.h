/**
 * @file libesoup/processors/es-dsPIC33EP256GP502.h
 *
 * @author John Whitmore
 *
 * @brief Definitions for the dsPIC33EP128GS792 micro-controller
 *
 * Copyright 2020 electronicSoup Limited
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
#if defined(__dsPIC33EP256GP502__)

#ifndef _ES_dsPIC33EP256GP502_H
#define _ES_dsPIC33EP256GP502_H


#include "libesoup_config.h"
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

#define HW_TIMER_OVERHEAD 0

/**
 * @brief UART Settings.
 */
enum uart_channel {
#ifdef SYS_UART1
        UART_1,
#endif
#ifdef SYS_UART2
        UART_2,
#endif
        NUM_UART_CHANNELS
};

#if defined(SYS_UART1)
  #define U1_ENABLE          U1MODEbits.UARTEN   ///< UART 1 Enable SFR bit

  #define U1_RX_ISR_FLAG     IFS0bits.U1RXIF     ///< UART 1 Recieve Interrupt Flag SFR Bit
  #define U1_RX_ISR_PRIOTITY IPC2bits.U1RXIP     ///< UART 1 Recieve Interrupt Priority SFR
  #define U1_RX_ISR_ENABLE   IEC0bits.U1RXIE     ///< UART 1 Recieve Interrupt Enable SFR Bit

  #define U1_TX_ISR_FLAG     IFS0bits.U1TXIF     ///< UART 1 Transmit Interrupt FLAG SFR Bit
  #define U1_TX_ISR_PRIOTITY IPC3bits.U1TXIP     ///< UART 1 Transmit Interrupt Priority SFR
  #define U1_TX_ISR_ENABLE   IEC0bits.U1TXIE     ///< UART 1 Transmit Interrupt Enable SFR Bit
#endif
#if defined(SYS_UART2)
  #define U2_ENABLE          U2MODEbits.UARTEN   ///< UART 2 Enable SFR bit

  #define U2_RX_ISR_FLAG     IFS1bits.U2RXIF     ///< UART 2 Recieve Interrupt Flag SFR Bit
  #define U2_RX_ISR_PRIOTITY IPC7bits.U2RXIP     ///< UART 2 Recieve Interrupt Priority SFR
  #define U2_RX_ISR_ENABLE   IEC1bits.U2RXIE     ///< UART 2 Recieve Interrupt Enable SFR Bit

  #define U2_TX_ISR_FLAG     IFS1bits.U2TXIF     ///< UART 2 Transmit Interrupt FLAG SFR Bit
  #define U2_TX_ISR_PRIOTITY IPC7bits.U2TXIP     ///< UART 2 Transmit Interrupt Priority SFR
  #define U2_TX_ISR_ENABLE   IEC1bits.U2TXIE     ///< UART 2 Transmit Interrupt Enable SFR Bit
#endif
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
#if defined(SYS_UART1) || defined (SYS_UART2)
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
#endif

/*
 * I2C Channels
 * 
 * Check Config setting: #pragma config ALTI2C1  AND ALTI2C2
 * those setting change the pins used by I2C Channels!
 */
enum i2c_chan_id {
#ifdef SYS_I2C_1
        I2C1,
#endif
#ifdef SYS_I2C_2
        I2C2,
#endif
        NUM_I2C_CHANNELS
};

/*
 * SPI Channels
 */
enum spi_chan_id {
#ifdef SYS_SPI1
        SPI_1,
#endif
#ifdef SYS_SPI2
        SPI_2,
#endif
        NUM_SPI_CHANNELS
};

/**
 * @brief GPIO Pins available in the uC
 */
enum gpio_pin {
	RA0,     ///< Port A Bit 0
	RA1,     ///< Port A Bit 1
	RA2,     ///< Port A Bit 2
	RA3,     ///< Port A Bit 3
	RA4,     ///< Port A Bit 4

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

	INVALID_GPIO_PIN = 0xff,     ///< Dummy Value used to represent no GPIO Pin
};

/**
 * @brief ADC Pins available in the uC
 */
enum adc_pin {
	AN0,
        AN1,
	AN2,
	AN3,
	AN4,
	AN5,

	INVALID_ADC_PIN = 0xff,     ///< Dummy Value used to represent no GPIO Pin
};

/**
 * @brief PWM Pins available in the uC
 */
enum pwm_pin {
	INVALID_PWM_PIN = 0xff,     ///< Dummy Value used to represent no GPIO Pin
};

/**
 * @brief uC Clock Sources:
 * 
 * See Microchip document DS70005131
 */
#define dsPIC33_INTERNAL_RC_PLL         0b001    ///< Internal RC with PLL
#define dsPIC33_PRIMARY_OSCILLATOR      0b010    ///< Primary Oscillator without PLL
#define dsPIC33_PRIMARY_OSCILLATOR_PLL  0b011    ///< Primary Oscillator with PLL

#define SECONDARY_OSCILLATOR            OSCCONbits.LPOSCEN   ///< Secondary Oscillator enable SFR Bit


#define PPS_I_EX_INT_1      RPINR0bits.INT1      ///< External Interrupt 1
#define PPS_I_EX_INT_2      RPINR1bits.INT2      ///< External Interrupt 2
#define PPS_I_T2CK          RPINR3bits.T2CK      ///< Timer 2 clock input source
#define PPS_I_IC1           RPINR7bits.IC1       ///< Input Capture 1
#define PPS_I_IC2           RPINR7bits.IC2       ///< Input Capture 2
#define PPS_I_IC3           RPINR8bits.IC3       ///< Input Capture 3
#define PPS_I_IC4           RPINR8bits.IC4       ///< Input Capture 4
#define PPS_I_OCFA          RPINR11bits.OCFA     ///< Output Compare Fault A
#define PPS_I_FLT1          RPINR12bits.FLT1     ///< PMW Fault 1
#define PPS_I_FLT2          RPINR12bits.FLT2     ///< PMW Fault 2
#define PPS_I_QE1_APH       RPINR14bits.
#define PPS_I_QE1_BPH       RPINR14bits.
#define PPS_I_QE1_IDX       RPINR15bits.
#define PPS_I_QE1_HM        RPINR15bits.
#define PPS_I_UART_1_RX     RPINR18bits.U1RXR    ///< UART 1 Recieve
#define PPS_I_UART_2_RX     RPINR19bits.U2RXR    ///< UART 2 Recieve
#define PPS_I_SPI_2_DI      RPINR22bits.SDI2R    ///< SPI 2 Data Input
#define PPS_I_SPI_2_CLK     RPINR22bits.SCK2R    ///< SPI 2 Clock
#define PPS_I_SPI_2_SS      RPINR22bits.SS2R     ///< SPI 2 Slave Select
#define PPS_I_CAN1_RX       RPINR26bits.C1RXR    ///< CAN 1 Recieve
#define PPS_I_PWM_SYNC1     RPINR37bit

/**
 * @brief Peripheral Output functions
 */
#define PPS_O_DEFAULRT_PORT             0x00     ///< Defauly Pin
#define PPS_O_UART_1_TX                 0x01     ///< UART 1 Transmit
#define PPS_O_UART_2_TX                 0x03     ///< UART 2 Transmit
#define PPS_O_SPI2DO                    0x08     ///< SPI 2 Data Output
#define PPS_O_SPI2CLK                   0x09     ///< SPI 2 Clock Output
#define PPS_O_SPI2SS                    0x0A     ///< SPI 2 Slave Select Output
#define PPS_O_CAN1_TX                   0x0E     ///< CAN 1 Transmit
#define PPS_O_OC1                       0x10     ///< Output Compare 1 Output
#define PPS_O_OC2                       0x11     ///< Output Compare 2 Output
#define PPS_O_OC3                       0x12     ///< Output Compare 3 Output
#define PPS_O_OC4                       0x13     ///< Output Compare 4 Output
#define PPS_O_C1OUT                     0x18     ///< Comparator Output 1
#define PPS_O_C2OUT                     0x19     ///< Comparator Output 2
#define PPS_O_C3OUT                     0x1A     ///< Comparator Output 3
#define PPS_O_SYNCO1                    0x2D     ///< PWM Primary Time Base Sync Output
#define PPS_O_QEI1CCMP                  0x2F
#define PPS_O_REFCLK                    0x31     ///< Reference Clock Output
#define PPS_O_C4OUT                     0x32     ///< Comparator Output 3

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
//#define FLASH_PAGE_SIZE                0x800
//#define FLASH_LAST_ADDRESS             0x2ABF9
//#define FLASH_NUM_INSTRUCTION_PER_ROW  128

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
//#define FLASH_FIRMWARE_START_ADDRESS   0x08800
//#define FLASH_APP_START_ADDRESS        0x18000

/**
 * @def   FLASH_APP_HANDLE_PAGE
 * @brief Address of the Applications Handle page in Flash.
 *
 * Because of limitations in the PIC Architecture, specifically in how far in
 * Flash Memory a branching instruction can jump a page of Flash is reserved in
 * low memory for the handlers to be able to jump up to high memory of the 
 * Application's code.
 */
//#define FLASH_APP_HANDLE_PAGE        0x400

/**
 * @brief micro-controller specific initialisation code
 */
extern void cpu_init(void);
extern enum adc_pin  get_adc_from_gpio(enum gpio_pin);
extern enum gpio_pin get_gpio_from_adc(enum adc_pin);

extern enum pwm_pin get_pwm_from_gpio(enum gpio_pin gpio_pin);
extern enum gpio_pin get_gpio_from_pwm(enum pwm_pin pwm_pin);

#endif // _ES_dsPIC33EP256GP502_H

#endif // defined(__dsPIC33EP256GP502__)
