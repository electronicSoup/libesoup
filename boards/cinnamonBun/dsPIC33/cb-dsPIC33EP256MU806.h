/**
 *
 * @file libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h
 *
 * @author John Whitmore
 *
 * This file contains board specific definitions for the dsPIC33EP256MU806 based
 * cinnamonBun device. The board contains an SPI based EEPROM device and OneWire
 * devices whose definitions are included in this board header file, as well as
 * CRYSTAL_FREQ for the device.
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
#ifndef _CB_dsPIC33EP256MU806_H
#define _CB_dsPIC33EP256MU806_H

#include "libesoup/processors/es-dsPIC33EP256MU806.h"

/**
 * @brief Crystal Frequency of the Hardware Device.
 *
 * The dsPIC33 contains a Phase Locked Loop so this crystal frequency is not
 * necessarily the instruction clock used by the device. The libesoup_config.h
 * file defines the requested instruction clock frequency, which will be 
 * configured, as part of libesoup_init() on startup.
 */
#define BRD_CRYSTAL_FREQ 16000000

/**
 * @brief Serial Logging pin configuration
 *
 * The cinnamonBun has three pins for the debug interface, which will be used
 * if SYS_SERIAL_LOGGING is defined in libesoup_config.h. One pin is hardwired 
 * to ground but the other two can be configured as either the recieve pin or the
 * transmit pin, using the peripheral pin select mechanism of the dsPIC33.
 * If libesoup_config.h defines SYS_SERIAL_LOGGING then it should specify the pin
 * orientaiton fo the serial logging port
 */
#ifdef SYS_SERIAL_LOGGING
#if defined(SYS_SERIAL_PORT_GndTxRx)
        #define BRD_SERIAL_LOGGING_TX_PIN  RG8
        #define BRD_SERIAL_LOGGING_RX_PIN  RG6
#elif defined(SYS_SERIAL_PORT_GndRxTx)
        #define BRD_SERIAL_LOGGING_TX_PIN  RG6
        #define BRD_SERIAL_LOGGING_RX_PIN  RG8
#else
#error Serial Logging pin orientation not defined!
#endif
#endif // SYS_SERIAL_LOGGING

/**
 * @brief EEPROM Definitions
 *
 * Definitions used for the EEPROM device
 */
/**
 * @def   EEPROM_CS_PIN_DIRECTION
 * @brief Maximum address of the EEPROM Memory
 *
 * Definition for the size of the EEPROM memory on the hardware.
 * Any attempt to read or write from addresses above this limit will raise an error.
 */
#define BRD_EEPROM_MAX_ADDRESS      0x7F

/**
 * @def   EEPROM_CS_PIN
 * @brief EEPROM Chip Select Pin
 *
 * @def   EEPROM_Select
 * @brief Macro to select the EEPROM Chip
 *
 * @def   EEPROM_DeSelect
 * @brief Macro to deselect the EEPROM Chip
 */
#define BRD_EEPROM_CS_PIN                  RD11
#define BRD_EEPROM_Select                  LATDbits.LATD11 = 0;
#define BRD_EEPROM_DeSelect                LATDbits.LATD11 = 1;


/**
 * @brief CAN Bus definitions
 *
 * Pins used by the CAN Bus interface on the board
 */
#define BRD_CAN_RX_PIN                     RG7
#define BRD_CAN_TX_PIN                     RF4

/**
 * @brief One Wire bus definition
 *
 * The pin being used by the board's One Wire bus
 */
#define BRD_ONE_WIRE_PIN                   RF3

/**
 * @brief SPI (Serial Peripheral Interface Definitions)
 *
 * @def   BRD_SPI_SCK
 * @brief Pin for SPI Clock line.
 *
 * @def   BRD_SPI_MOSI
 * @brief Pin for the SPI Master Out Slave In line. 
 *
 * @def   BRD_SPI_MISO
 * @brief Pin for the SPI Master In Slave Out line.
 *
 */
#define BRD_SPI_SCK          RF1
#define BRD_SPI_MOSI         RF0
#define BRD_SPI_MISO        RD10

/**
 * @def   USB_HOST
 * @brief Turn on the 5 Volt power to the USB Bus.
 *
 * @def   USB_DEVICE
 * @brief Turn off the 5 Volt power to the USB Bus.
 */
#define USB_HOST    TRISDbits.TRISD9 = OUTPUT_PIN; LATDbits.LATD9 = 1; USBInitialize(0);
#define USB_DEVICE  TRISDbits.TRISD9 = OUTPUT_PIN; LATDbits.LATD9 = 0;

#include "libesoup/core.h"

#endif // _CB_dsPIC33EP256MU806_H
