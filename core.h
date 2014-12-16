/**
 *
 * \file es_lib/core.h
 *
 * Core definitions required by electronicSoup Cinnamon Bun
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef ES_CAN_CORE_H
#define ES_CAN_CORE_H

#include <p24Fxxxx.h>

/*
 * Clock speed of the Hardware.
 */
#define CLOCK_FREQ 16000000

/*
 *  EEPROM Address Map
 */
#define EEPROM_BOOT_PAGE_SIZE   0x20
#define EEPROM_MAX_ADDRESS      0x7F

/*
 *  RD1/RP24  - SCK
 *  RD2       - SO  (Pin 14 of 18 DIP 2515)
 *  RD3       - SI  (Pin 15 of 18 DIP 2515)
 *  RD7       - /EEPROM CS
 *  RD6       - /CAN CS
 *  RD0       - /CAN INT
 */


//  RD7 - /EEPROM CS
#define EEPROM_CS_PIN_DIRECTION    TRISDbits.TRISD7
#define EEPROM_CS                  LATDbits.LATD7

#define EEPROM_Select()            EEPROM_CS = 0
#define EEPROM_DeSelect()          EEPROM_CS = 1

/*
 * EEPROM SPI Commands.
 */
#define EEPROM_READ           0x03
#define EEPROM_WRITE          0x02
#define EEPROM_WRITE_DISABLE  0x04
#define EEPROM_WRITE_ENABLE   0x06
#define EEPROM_STATUS_READ    0x05
#define EEPROM_STATUS_WRITE   0x01

//  RD0  - /CAN INT
#define CAN_INTERRUPT_PIN_DIRECTION    TRISDbits.TRISD0
#define CAN_INTERRUPT_PIN                  PORTDbits.RD0
#define CAN_INTERRUPT                  !CAN_INTERRUPT_PIN

//  RD6 - /CAN CS
#define CAN_CS_PIN_DIRECTION    TRISDbits.TRISD6
#define CAN_CS                  LATDbits.LATD6

#define CAN_SELECT()            CAN_CS = 0;
#define CAN_DESELECT()          CAN_CS = 1

/*
 * SPI (Serial Peripheral Interface Definitions.
 *
 * Pins:  RD1  - SCK
 *        RD2  - MISO
 *        RD3  - MOSI
 */
#define SPI_RW_FINISHED SPI1STATbits.SPIRBF

#define SPI_SCK_DIRECTION   TRISDbits.TRISD1
#define SPI_MISO_DIRECTION  TRISDbits.TRISD2
#define SPI_MOSI_DIRECTION  TRISDbits.TRISD3

/*
 * I/O pin definitions
 */
#define INPUT_PIN  1
#define OUTPUT_PIN 0

/*
 * Flash addresses
 */
/*
 * The Flash page size is 512 Instructions, which is 1536 as each instruction is 3 Bytes.
 * But the Flash is addressed in Words so the length given here is 0x400 (512 * 2)
 */
#define FLASH_PAGE_SIZE                0x400
#define FLASH_LAST_ADDRESS             0x2ABF9
#define FLASH_NUM_INSTRUCTION_PER_ROW  64

/*
 * Start of Firmware code
 */
#define FLASH_FIRMWARE_START_ADDRESS   0x8800

/*
 * Address of the App's page in low memory
 */
#define FLASH_APP_HANDLE_PAGE        0x400

/*
 * USB Host Power pin
 */
#define USB_HOST    TRISDbits.TRISD8 = OUTPUT_PIN; LATDbits.LATD8 = 1; USBInitialize(0);
#define USB_DEVICE  TRISDbits.TRISD8 = OUTPUT_PIN; LATDbits.LATD8 = 0;

/*
 * The Hardware has a "Boot" Jumper. If the jumper is not connected the
 * bootloader does NOT attempt to connect to an Android device and allow
 * firmware update.
 *
 * NOTE: This definition might look incorrect with the '=' but it IS 
 * Correct. The switch is expected to be used in an 'if' statement. The 
 * first part will be false but it's the second part of the or statement
 * that will dictate the action of the if!
 */
#define BOOT_FLAG      (TRISDbits.TRISD11 = 0 || PORTDbits.RD11)

/*
 * Android IPC
 *
 * App message is transmitted from the Android Device App to the Cinnamom Bun
 * Bun message is transmitted from the Cinnamon Bun to the Android Device App
 *
 * Messages carry a Byte Identifier so there are 255 possible messages. The
 * lower message id's may be used by es_lib system so a user's Android App
 * messages should be defined relative to these. For example you would define
 * your messages to send from the CinnamonBun as:
 *
 * #define MY_FIRST_BUN_MSG   BUN_MSG_USER_OFFSET
 * #define MY_SECOND_BUN_MSG  BUN_MSG_USER_OFFSET + 1
 *
 * And messgaes which your CinnamonBun project expects to recieve from the
 * Android App would be defined as:
 *
 * #define MY_FIRST_APP_MSG   APP_MSG_USER_OFFSET
 * #define MY_FIRST_APP_MSG   APP_MSG_USER_OFFSET + 1
 */
#define  APP_MSG_COMMAND_APP_DISCONNECT  0x00
#define  APP_MSG_COMMAND_APP_CONNECT     0x01

#define BUN_MSG_USER_OFFSET    0x00
#define APP_MSG_USER_OFFSET    0x02

/*
 * Include MicroChip's definitions
 */
#if defined(MCP)
    #include <GenericTypeDefs.h>

    typedef UINT8    u8;
    typedef UINT16   u16;
    typedef UINT32   u32;

#elif defined(ES_LINUX)
    #include <stdint.h>

    typedef uint8_t  u8;
    typedef uint16_t u16;
    typedef uint32_t u32;

    typedef unsigned char bool;
    #define FALSE 0
    #define TRUE !(FALSE)

    #include <linux/can.h>
    #include <time.h>
    #include <signal.h>

    #define can_frame struct can_frame
#endif

/**
 *
 * \enum result_t
 *
 * \brief Error codes returned by API Functions.
 */
typedef enum {
    SUCCESS = 0x00, /**< enum value Success */
    ERR_GENERAL_ERROR,
    ERR_TIMER_ACTIVE,
    ERR_NO_RESOURCES,
    ERR_ADDRESS_RANGE,
    ERR_GENERAL_CAN_ERROR,
    ERR_CAN_NOT_CONNECTED,
    ERR_CAN_TARGET_OVERLAP,
    ERR_CAN_NO_FREE_BUFFER,
    ERR_L3_UNINITIALISED,
    ERR_GENERAL_L3_ERROR,
    ERR_L3_NO_ADDRESS,	  
    ERR_L3_NOT_REGISTERED,
    ERR_L3_ZERO_LENGTH,
    ERR_L3_MAX_LENGTH,
    ERR_L3_TX_BUSY,
    ERR_L3_PROTOCOL
} result_t;

/**
 *  This is the maximum string length of a status reported
 *  over the network.
 */
//#define MAX_STATUS_LEN 24

/**
 * \brief Duration of the system timer tick in MilliSeconds.
 *
 * If code is written in a protable fashion using the macros provided in
 * es_lib/timers.h then this value can be changed if greater grandularity
 * timers are required. At present it is expected to be in milliSeconds and if
 * even greater grandularity then 1mS is required then initialisation code
 * in es_lib/timers/timers.c will have to be changed and the macros in
 * es_lib/timers/tiemrs.h
 */
#define SYSTEM_TICK_ms 5 //5 mS

/****************************************************
 *
 * CAN Bus Layer 2 types
 *
 ****************************************************
 *
 */
/**
 * \brief CAN_DATA_LENGTH
 *
 * A CAN Bus Layer 2 message can accept 8 bytes of additional data
 */
#define CAN_DATA_LENGTH 8

/*
 * There are two main versions of the CAN Bus protocol. Origionally each
 * Layer 2 CAN Message Identifier was 11 Bits in length. This origional is
 * called a Standard CAN Message. A later version of the Protocol extended 
 * the Layer 2 CAN Message Identifier length to 29 Bits.
 *
 * Both message types can be sent over the same CAN Bus Network and are supported
 * by the CAN Node Boards.
 */

#if defined(MCP)
/*
 * special address description flags for the CAN_ID
 * 
 * SFF - Standard Frame Format
 * EFF - Extended Frame Format
 */
#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error message frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

/*
 * Controller Area Network Identifier structure
 *
 * bit 0-28	: CAN identifier (11/29 bit)
 * bit 29	: error message frame flag (0 = data frame, 1 = error message)
 * bit 30	: remote transmission request flag (1 = rtr frame)
 * bit 31	: frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
 */
typedef u32 canid_t;


/**
 * \brief can_msg_t Can Message Type
 *
 * Structure to define the Layer 2 CAN Message. Simply the header above and
 * and array for the Data Bytes.
 */
typedef struct __attribute__ ((packed))
{
    canid_t can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    u8            can_dlc;
    u8          data[CAN_DATA_LENGTH];
} can_frame;
#endif //MCP

/**
 * \brief l2_msg_handler
 *
 * In the CAN Node Board API an Application can define CAN Layer 2 message
 * handlers. A handler accepts as parameter a pointer to a CAN Message sructure
 * defined above and returns nothing.
 */
typedef void (*can_l2_msg_handler_t)(can_frame *msg);

/**
 * \brief can_target_t
 *
 * Typedef for a CAN Layer 2 Target Message Identifier.
 */
typedef struct 
{
    u32           mask;
    u32           filter;
    can_l2_msg_handler_t handler;
} can_l2_target_t;

/**
 * \brief L3_CAN_MAX_MSG
 * CAN Layer 3 message
 *
 * The maximum Layer 3 Message size is 74 Bytes including the protocol!
 * So one protocol Byte and 73 Data Bytes
 */
#define L3_CAN_MAX_MSG 73 // reduced by one make protocol seperate 74  //4095

/**
 * \brief l3_can_msg_t
 *
 */
typedef struct
{
    u8 address;
    u8 size;
    u8 protocol;
    u8 *data;
} can_l3_msg_t;

/**
 * \brief l3_msg_handler
 *
 * CAN Layer 3 Message Handler function.
 */
typedef void (*can_l3_msg_handler_t)(can_l3_msg_t *msg);


/**
 * \brief log_level_t
 *
 * Logger levels
 */
typedef enum
{
    Debug = 0,
    Info,
    Warning,
    Error
} log_level_t;

/*
 * This series of defines are here as they should be included very early. At 
 * leasy before the serial logging code. These macros are used to conditionally
 * compile debugging code out of an executable. The actual logging level of the
 * build should be defined in system.h
 */
#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_WARNING 2
#define LOG_ERROR 3
#define NO_LOGGING 4


typedef enum {
    HWInfoRequest = 0x01,
    HWInfoResponse = 0x02,
    OSInfoRequest = 0x03,
    OSInfoResponse = 0x04,
    AppInfoRequest = 0x05,
    AppInfoResponse = 0x06,
    AppStatusRequest = 0x07,
    AppStatusResponse = 0x08
} node_management_msg_t;


#endif // ES_CAN_CORE_H
