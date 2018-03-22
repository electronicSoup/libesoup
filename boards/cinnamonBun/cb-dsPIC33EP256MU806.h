/**
 *
 * @file libesoup/processors/cb-PIC24FJ256GB106.h
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
#ifndef _CB_dsPIC33EP256MU806_H
#define _CB_dsPIC33EP256MU806_H

#include "libesoup/processors/es-dsPIC33EP256MU806.h"

/**
 * @brief Crystal Frequency of the Hardware Device.
 */
#define CRYSTAL_FREQ 16000000

/**
 * @brief Bootloader pause on powerup, and listen for new Firmware?
 *
 * The cinnamonBun Hardware has a "Boot" Jumper. If the jumper is not connected
 * the bootloader does NOT attempt to connect to an Android device and allow
 * firmware update.
 *
 * NOTE: This definition might look incorrect with the '=' but it IS 
 * Correct! The switch is expected to be used in an 'if' statement. The 
 * first part will be false but it's the second part of the or statement
 * that will dictate the action of the if!
 */

/*
 * Serial Logging
 */
#if defined(SYS_SERIAL_PORT_GndTxRx)
        #define SERIAL_LOGGING_TX_PIN  RG8
        #define SERIAL_LOGGING_RX_PIN  RG6
#elif defined(SYS_SERIAL_PORT_GndRxTx)
        #define SERIAL_LOGGING_TX_PIN  RG6
        #define SERIAL_LOGGING_RX_PIN  RG8
#endif

/*
 * EEPROM Definitions
 *
 * @brief Maximum address of the EEPROM Memory
 *
 * This constant defines the size of the EEPROM memory on the hardware this
 * may change depending on future electronicSoup versions or on your own 
 * particular hardware. Any attempt to read or write from addresses above this 
 * limit will raise an error.
 */
#define EEPROM_MAX_ADDRESS      0x7F

/**
 * @def   EEPROM_CS_PIN_DIRECTION
 * @brief EEPROM Chip Select Direction Pin of the connected EEPROM chip
 * 
 * @def   EEPROM_CS
 * @brief Chip Select Pin of the connected EEPROM chip
 *
 *
 * @def   EEPROM_Select
 * @brief Macro to select the EEPROM Chip
 *
 * @def   EEPROM_DeSelect
 * @brief Macro to deselect the EEPROM Chip
 */
#define EEPROM_CS_PIN                  RD11
#define EEPROM_CS                      LATDbits.LATD11
#define EEPROM_Select                  EEPROM_CS = 0;
#define EEPROM_DeSelect                EEPROM_CS = 1;


/*
 * CAN Bus definitions
 */
#define CAN_RX_PIN                     RG7
#define CAN_TX_PIN                     RF4

/*
 * One Wire bus
 */
#define ONE_WIRE_PIN                   RF3

/**
 * SPI (Serial Peripheral Interface Definitions.
 *
 * @def   SPI_RW_FINISHED
 * @brief Macro to check if current SPI Read/Write operation is complete
 *
 * An SPI read or write operation shouldn't be started till the previous one 
 * has completed. The Processor sets this bit when operation complete.
 *
 * @def   SPI_SCK_DIRECTION
 * @brief Data Direction Register pin for SPI Clock line.
 *
 * @def   SPI_MISO_DIRECTION
 * @brief Data Direction Register pin for the SPI Master In Slave Out line.
 *
 * @def   SPI_MOSI_DIRECTION
 * @brief Data Direction Register pin for the SPI Master Out Slave In line. 
 *
 */
#define BRD_SPI_SCK          RF1
#define BRD_SPI_MOSI         RF0
#define BRD_SPI_MISO        RD10

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
