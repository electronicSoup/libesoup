/**
 * @file libesoup/boards/cinnamonBun/pic24FJ/cb-PIC24FJ256GB106.h
 *
 * @author John Whitmore
 *
 * Board file for the PIC24FJ256GB106 based cinnamonBun device.
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
#ifndef _CB_PIC24FJ256GB106_H
#define _CB_PIC24FJ256GB106_H

#include "libesoup/processors/pic24/es-PIC24FJ256GB106.h"

/**
 * @brief Crystal Frequency of the Hardware Device.
 */
#define BRD_CRYSTAL_FREQ 8000000  // This board is using the internal RC

/*
 * Serial Logging
 */
#ifdef SYS_SERIAL_LOGGING
#if defined(SYS_SERIAL_PORT_GndTxRx)
        #define BRD_SERIAL_LOGGING_TX_PIN    RD4
        #define BRD_SERIAL_LOGGING_RX_PIN    RD5
#elif defined(SYS_SERIAL_PORT_GndRxTx)
        #define BRD_SERIAL_LOGGING_TX_PIN    RD5
        #define BRD_SERIAL_LOGGING_RX_PIN    RD4
#else
#error libesoup_config.h should define the orientation of serial logging port.
#endif
#endif // SYS_SERIAL_LOGGING

/**
 * @brief EEPROM Definitions
 *
 */
/**
 * @brief Maximum address of the EEPROM Memory
 *
 * This constant defines the size of the EEPROM memory on the hardware this
 * may change depending on future electronicSoup versions or on your own 
 * particular hardware. Any attempt to read or write from addresses above this 
 * limit will raise an error.
 */
#define BRD_EEPROM_MAX_ADDRESS      0x7F

/**
 * @def   BRD_EEPROM_CS_PIN
 * @brief EEPROM Chip Select Pin
 * 
 * @def   BRD_EEPROM_Select
 * @brief Macro to select the EEPROM Chip
 *
 * @def   BRD_EEPROM_DeSelect
 * @brief Macro to deselect the EEPROM Chip
 */
#define BRD_EEPROM_CS_PIN                  RD7
#define BRD_EEPROM_Select                  LATDbits.LATD7 = 0;
#define BRD_EEPROM_DeSelect                LATDbits.LATD7 = 1;

/**
 * @brief CAN Interface definitions
 */
/**
 * @def   BRD_CAN_BUS_MCP2515
 * @brief Macro to include the MCP2515 CAN Controller in the build
 *
 * @def   BRD_CAN_INTERRUPT_PIN
 * @brief CAN Controller (MCP2515) Pin
 *
 * @def   BRD_CAN_INTERRUPT
 * @brief CAN Controller (MCP2515) Input to test if there is an Interrupt
 */
#define BRD_CAN_BUS_MCP2515
#define BRD_CAN_INTERRUPT_PIN              RD0
#define BRD_CAN_INTERRUPT                  !PORTDbits.RD0

/**
 * @def   BRD_CAN_CS_PIN
 * @brief CAN Controller Chip Select pin
 *
 * @def   BRD_CAN_SELECT
 * @brief Macro to select CAN Controller on SPI Bus
 *
 * @def   BRD_CAN_DESELECT
 * @brief Macro to deselect CAN Controller Chip on SPI Bus
 *
 */
#define BRD_CAN_CS_PIN                     RD6
#define BRD_CAN_SELECT                     LATDbits.LATD6 = 0;
#define BRD_CAN_DESELECT                   LATDbits.LATD6 = 1;

/**
 *
 */
#define BRD_SPI_SCK      RD1
#define BRD_SPI_MISO     RD2
#define BRD_SPI_MOSI     RD3
//#define SPI_RW_FINISHED     SPI1STATbits.SPIRBF
//#define SPI_SCK_DIRECTION   TRISDbits.TRISD1
//#define SPI_MISO_DIRECTION  TRISDbits.TRISD2
//#define SPI_MOSI_DIRECTION  TRISDbits.TRISD3

/*
 * Definitions of Peripheral pins
 */
//#define SPI_MISO_PIN        RP23

//#define SPI_MOSI_PIN        RPOR11bits.RP22R
//#define SPI_SCK_PIN         RPOR12bits.RP24R

/**
 * @def   BRD_USB_HOST
 * @brief Turn on the 5 Volt power to the USB Bus.
 *
 * @def   BRD_USB_DEVICE
 * @brief Turn off the 5 Volt power to the USB Bus.
 */
#define BRD_USB_HOST    TRISDbits.TRISD8 = GPIO_OUTPUT_PIN; LATDbits.LATD8 = 1; USBInitialize(0);
#define BRD_USB_DEVICE  TRISDbits.TRISD8 = GPIO_OUTPUT_PIN; LATDbits.LATD8 = 0;

/**
 * @brief core definitions required by the library
 *
 * core.h contains all fundamental types used in libesoup and thus your 
 * project.
 */
#include "libesoup/core.h"


#endif // _CB_PIC24FJ256GB106_H
