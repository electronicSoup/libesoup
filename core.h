/**
 * @file libesoup/core.h
 *
 * @author John Whitmore
 *
 * Core definitions required by electronicSoup Code Library
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU Lesser General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _CORE_H
#define _CORE_H

/**
 * \brief Function to initialise the libesoup library. The actual required 
 * configuration should be defined using switches in libesoup_config.h. An
 * example of that configuration file is included in libesoup/examples directory
 *
 * This function must be called prior to calling any libesoup API functions. 
 */
extern void libesoup_init(void);

/*
 * Include MicroChip's definitions
 */
#if defined(XC16) || defined(__XC8)
    #include <stdint.h>     // For uintx_t types
#ifndef	NULL
#define NULL (0)
#endif	/* NULL */

    typedef uint8_t    boolean;
#define FALSE 0x00
#define TRUE (!FALSE)

#elif defined(ES_LINUX)
    #include <stdint.h>

    typedef unsigned char boolean;
    #define FALSE 0
    #define TRUE !(FALSE)

    #include <sys/socket.h>
    #include <linux/can.h>
    #include <time.h>
    #include <signal.h>

    #define can_frame struct can_frame
#endif

/*
 * Union for converting byte stream to floats
 */
typedef union {
	uint8_t bytes[4];
	float value;
} f32;

/**
 * \defgroup Timers Timers
 * @{
 * 
 * \brief Enumeration for units of time. Used to specify durations for timers.
 * 
 * These are fairly straight forward and represent human representations of
 * time durations. When used to start timers the API functions will convert the 
 * duration in to a micro-controller specific tick count or crystal pulse count.
 */
typedef enum {
    uSeconds,
    mSeconds,
    Seconds,
    Minutes,
    Hours
} ty_time_units;
/**
 * @}
 */

/** @defgroup group1 Android Message Identifiers
 *
 *  Android IPC
 *
 * App message is transmitted from the Android Device App to the Cinnamom Bun
 * Bun message is transmitted from the Cinnamon Bun to the Android Device App
 *
 * Messages carry a Byte Identifier so there are 255 possible messages. The
 * lower message id's are reserved for use by libesoup system so a user's Android
 * App messages should be defined relative to these. For portability and to 
 * future proof code you would define your messages to send from the CinnamonBun
 * as:
 *
 * #define MY_FIRST_BUN_MSG   BUN_MSG_USER_OFFSET
 * #define MY_SECOND_BUN_MSG  BUN_MSG_USER_OFFSET + 1
 *
 * And messgaes which your CinnamonBun project expects to recieve from the
 * Android App would be defined as:
 *
 * #define MY_FIRST_APP_MSG   APP_MSG_USER_OFFSET
 * #define MY_FIRST_APP_MSG   APP_MSG_USER_OFFSET + 1
 *
 *  @{
 *
 */

/**
 * @def   APP_MSG_ERROR
 * @brief Error message -> cinnamonBun
 */
#define  APP_MSG_ERROR                   0x00

/**
 * @def   APP_MSG_APP_DISCONNECT
 * @brief Android application disconnected -> cinnamonBun
 */
#define  APP_MSG_APP_DISCONNECT          0x01

/**
 * @def   APP_MSG_APP_CONNECT
 * @brief Android application connected -> cinnamonBun
 */
#define  APP_MSG_APP_CONNECT             0x02

/**
 * @def   APP_MSG_FLASH_REPROGRAM
 * @brief Initiate Reflashing of either Firmware or Applicaiton -> cinnamonBun
 */
#define  APP_MSG_FLASH_REPROGRAM         0x03

/**
 * @def   APP_MSG_FLASH_ERASE_PAGE
 * @brief Erase a page of Flash Memory -> cinnamonBun
 *
 * The address of Flash Page is passed in the message
 */
#define  APP_MSG_FLASH_ERASE_PAGE        0x04

/**
 * @def   APP_MSG_FLASH_WRITE_ROW
 * @brief Write a row of Data to the Flash Memory -> cinnamonBun
 *
 * The address of the Row to be written and Bytes to write are contained in the
 * message.
 */
#define  APP_MSG_FLASH_WRITE_ROW         0x05

/**
 * @def   APP_MSG_FLASH_REFLASHED
 * @brief Programming of Flash Memory complete -> cinnamonBun
 */
#define  APP_MSG_FLASH_REFLASHED         0x06

/**
 * @def   APP_MSG_CAN_CONNECT
 * @brief Connect the cinnamonBun to the CAN Bus -> cinnamonBun
 */
#define  APP_MSG_CAN_CONNECT             0x07

/**
 * @def   APP_MSG_CAN_STATUS_REQ
 * @brief CAN Bus status request -> cinnamonBun
 *
 * Response from the cinnamonBun is a @see BUN_MSG_CAN_STATUS message
 */
#define  APP_MSG_CAN_STATUS_REQ          0x08

/**
 * @def   APP_MSG_CAN_L2_FRAME
 * @brief CAN Bus Frame -> cinnamonBun
 *
 * A CAN Bus message sent by the Android Applicaiton for the cinnamonBun to 
 * transmit on the CAN Bus.
 */
#define  APP_MSG_CAN_L2_FRAME            0x09

/**
 * @def   APP_MSG_CAN_L2_TARGET
 * @brief CAN Layer 2 Frame Target -> cinnamonBun
 *
 * The Android application declares its interest in a particular CAN Bus Frame
 * by filling out a target structure and sending it to the cinnamonBun. If the 
 * cinnamonBun matches a CAN Frame to the target then the Frame will be posted
 * to the connected Android Application.
 */
#define  APP_MSG_CAN_L2_TARGET           0x0a

/**
 * @def   APP_MSG_ISO15765_MSG
 * @brief ISO15765 -> cinnamonBun
 *
 * ISO15765 Message sent by the connected Android Application for transmission
 * on the CAN Bus network.
 */
#define  APP_MSG_ISO15765_MSG            0x0b

/**
 * @def   APP_MSG_ISO15765_TARGET
 * @brief ISO15765 Target -> cinnamonBun
 *
 * When the Android Application is interested in receiving ISO15765 messages it
 * fills out a Target structure and send the target to the cinnamonBun.
 */
#define  APP_MSG_ISO15765_TARGET         0x0c

/**
 * @def   APP_MSG_ISO11783_MSG
 * @brief ISO11783 -> cinnamonBun
 *
 * ISO11783 message sent from the Android Application to the cinnamonBun for 
 * transmission on the CAN Bus Network.
 */
#define  APP_MSG_ISO11783_MSG            0x0d

/**
 * @def   APP_MSG_ISO11783_TARGET
 * @brief ISO11783 Target -> cinnamonBun
 *
 * When the Android Application is interested in receiving ISO11783 messages it
 * fills out a target structure and sends this to the cinnamonBun. If the 
 * cinnamonBun receives a messages matching the target it is sent to the 
 * Android Application.
 */
#define  APP_MSG_ISO11783_TARGET         0x0e

/**
 * @def   APP_MSG_USER_OFFSET
 * @brief First User defined message -> cinnamonBun
 *
 * End of the reserved Android -> cinnamonBun messages. This and all following 
 * Byte codes are application specific.
 */
#define  APP_MSG_USER_OFFSET             0x20

/**
 * @def   BUN_MSG_ERROR
 * @brief Error message -> Android Application
 */
#define  BUN_MSG_ERROR                   0x00

/**
 * @def   BUN_COMMAND_READY
 * @brief cinnamonBun ready for next command -> Android Application
 *
 * during reprogramming of the Flash Memory, either Firmware or Application, 
 * when the cinnamonBun is ready to process another message it reports its
 * readiness with this message identifier.
 */
#define  BUN_COMMAND_READY               0x01

/**
 * @def   BUN_MSG_CAN_STATUS
 * @brief CAN Bus status -> Android Application
 *
 * If the Android Application requests a CAN Status then the cinnamonBun
 * responds with the current status in this message.
 */
#define  BUN_MSG_CAN_STATUS              0x02

/**
 * @def   BUN_MSG_CAN_L2_FRAME
 * @brief CAN Frame -> Android Application
 *
 * A CAN Frame which matches a Target previously registered by the Android 
 * application with a @see APP_MSG_CAN_L2_TARGET message has been received
 * from the CAN Bus Network and is forwarded to the Android Applicaiton for
 * further processing.
 */
#define  BUN_MSG_CAN_L2_FRAME            0x03

/**
 * @def   BUN_MSG_ISO15765_MSG
 * @brief ISO15765 Message -> Android Application
 *
 * A ISO16765 Message received from the CAN Bus matches a previously registered
 * target @see APP_MSG_ISO15765_TARGET. The received massage is forwarded to the
 * Android Application.
 */
#define  BUN_MSG_ISO15765_MSG            0x04

/**
 * @def   BUN_MSG_ISO11783_MSG
 * @brief ISO11783 Message -> Android Application
 *
 * An ISO11783 Message received from the CAN Bus matches a previously registered
 * target @see APP_MSG_ISO11783_TARGET. The received massage is forwarded to the
 * Android Application.
 */
#define  BUN_MSG_ISO11783_MSG            0x05

/**
 * @def   BUN_MSG_USER_OFFSET
 * @brief FIrst User defined massage -> Android Application
 *
 * This is the first user defined message identifier.
 */
#define  BUN_MSG_USER_OFFSET             0x20

/** @}*/

/** @defgroup group2 libesoup Return Codes
 *
 * @enum  result_t
 * @brief Error codes returned by libesoup API Functions.
 *
 *  @{
 *
 * @enum  SUCCESS
 * @brief API function executed without error.
 *
 * @enum  ERR_GENERAL_ERROR,
 * @brief Unspecified error occured in libesoup API Function execution.
 *
 * @enum  ERR_BAD_INPUT_PARAMETER,
 * @brief Bad parameter passed to libesoup API funtion.
 *
 * @enum  ERR_TIMER_ACTIVE,
 * @brief Attempt to start a timer which is already active.
 *
 * @enum  ERR_NO_RESOURCES,
 * @brief No resources available to execute the requested libesoup API function.
 *
 * This error indicates that system resources have been exhausted. The solution
 * to this error can be as simple as changing the number of resources declared
 * in your libesoup_config.h file.
 *
 * For example if an attempt to start a timer returns the no resources return
 * code then you can rebuild your system with more timers, 
 * @see SYS_NUMBER_OF_TIMERS Of course more system Main Memory will be used
 * up in timer table, in this case.
 *
 * @enum  ERR_ADDRESS_RANGE,
 * @brief An address passed to an libesoup API function is out of valid range.
 *
 * @enum  ERR_NOT_READY,
 * @brief API function no ready to execute request.
 *
 * @enum  ERR_GENERAL_CAN_ERROR,
 * @brief Error in CAN libesoup API function.
 *
 * @enum  ERR_CAN_NOT_CONNECTED,
 * @brief Attempt to perform action on CAN Bus prior to its connection.
 *
 * @enum  ERR_CAN_BAUDRATE,
 * @brief Invalid Baudrate specified in libesoup API funciton call.
 *
 * @enum  ERR_CAN_NO_FREE_BUFFER,
 * @brief No free CAN buffer to complete request.
 *
 * @enum  ERR_UNINITIALISED,
 * @brief Attempt to call libesoup API function prior to initialisation.
 *
 * @enum  ERR_BUSY
 * @brief I'm busy call back later.
 *
 */
typedef enum {
    SUCCESS = 0x00,
    ERR_GENERAL_ERROR,
    ERR_BAD_INPUT_PARAMETER,
    ERR_RANGE_ERROR,
    ERR_TIMER_ACTIVE,
    ERR_NO_RESOURCES,
    ERR_ADDRESS_RANGE,
    ERR_NOT_READY,
    ERR_GENERAL_CAN_ERROR,
    ERR_CAN_NOT_CONNECTED,
    ERR_CAN_BAUDRATE,
    ERR_CAN_NO_FREE_BUFFER,
    ERR_UNINITIALISED,
    ERR_BUSY,
    ERR_NOTHING_TO_DO,
    ERR_NO_RESPONSE
} result_t;

/** @}*/


/** @defgroup group3 CAN Bus definitions
 *
 * There are two main versions of the CAN Bus protocol. Origionally each
 * Layer 2 CAN Frame Identifier was 11 Bits in length. This origional is
 * called a "Standard" CAN Frame. A later version of the Protocol extended
 * the Layer 2 CAN Frame Identifier length to 29 Bits. These are reffered to 
 * as "Extended" CAN Frames
 *
 * Both message types can be sent over the same CAN Bus Network and are supported
 * by the cinnamonBun.
 *
 * The CAN definitions in this file mirror the CAN definitions used by the Linux
 * CAN code. This should mean that code for the cinnamonBun can be easily ported
 * to a Linux based platform. 
 *
 *  @{
 *
 */
/**
 * @def   CAN_DATA_LENGTH
 * @brief Data length of a CAN Frame
 *
 * A CAN Bus Layer 2 Frame can accept 8 bytes of additional data
 */
#define CAN_DATA_LENGTH 8

#if defined(XC16) || defined(__XC8)
/**
 * @def   CAN_EFF_FLAG
 * @brief Extended CAN Frame format Flag.
 *
 * A CAN Identifer passed around the libesoup is a 32 bit integer. as the Extended
 * CAN Frame format has a 29 Bit identifer the 3 most significant bits of the 
 * 32 Bit integer are used for flags.
 *
 * If the most significant bit of the 32 bits is set then the frame is a 29 bit
 * extended frame format frame.
 *
 * If the bit is not set then the Frame is a Standard Frame Format. 
 * 
 * SFF - Standard Frame Format
 * EFF - Extended Frame Format
 */
#define CAN_EFF_FLAG 0x80000000U /**< EFF/SFF is set in the MSB */

/**
 * @def   CAN_RTR_FLAG
 * @brief Remote Transmission Request Flag.
 *
 * CAN Frame identifiers signify a data value on the CAN Bus network. So for
 * example CAN id of 0x0123 might represent a water coolant temperature in 
 * the system. Whenever there is a water coolant temperature change an
 * "intelligent" agent can broadcast the new data on the network. Any nodes
 * interested in the water coolant tempreture can react to that data.
 *
 * If on the other hand a node on the network wants to find out the current
 * temperature of the water coolant it can send a CAN Frame with the Identifier
 * 0x0123 and the Remote Transmission Reqest (RTR) bit set. The node responsible
 * for Water Coolant tempreture receives this Frame and responds with the 
 * requested data. 
 */
#define CAN_RTR_FLAG 0x40000000U /**< remote transmission request */

/**
 * @def   CAN_ERR_FLAG
 * @brief CAN Frame Error Flag
 *
 * The received CAN Frame is in Error.
 */
#define CAN_ERR_FLAG 0x20000000U /**< error message frame */

/**
 * @def   CAN_SFF_MASK
 * @brief Standard Frame Mask
 *
 * Bitmask for the 11 bits of a Standard CAN Frame Identifer.
 */
#define CAN_SFF_MASK 0x000007FFU

/**
 * @def   CAN_EFF_MASK
 * @brief Extended Frame Mask
 *
 * Bitmask for the 29 bits of an Extended CAN Frame Identifer.
 */
#define CAN_EFF_MASK 0x1FFFFFFFU

/**
 * @type  canid_t
 * @brief typedef for 32 bit CAN Identifier
 *
 * Controller Area Network Identifier structure
 *
 * bit 0-28	: CAN identifier (11/29 bit)
 * bit 29	: error message frame flag (0 = data frame, 1 = error message)
 * bit 30	: remote transmission request flag (1 = rtr frame)
 * bit 31	: frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
 */
typedef uint32_t canid_t;


/**
 * @type  can_frame
 * @brief Can Frame Type
 *
 * Structure to define the Layer 2 CAN Frame. Simply the CAN Identifier, Data
 * length info and an array for the Data Bytes.
 */
#if defined(XC16)
typedef struct __attribute__ ((packed))
#elif defined(__XC8)  // (__18F2680) || defined(__18F4585)
typedef struct
#endif //__18F2680
{
    canid_t can_id; /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    uint8_t      can_dlc;
    uint8_t      data[CAN_DATA_LENGTH];
} can_frame;
#endif // XC16

/**
 * @type  can_l2_frame_handler_t
 * @brief typedef for a CAN Frame handler function
 *
 * code can register an interest in a CAN Frame identifer by registering a 
 * Frame target. This target will specify a frame handler function which will
 * be passed a received frame.
 */
typedef void (*can_l2_frame_handler_t)(can_frame *msg);

/**
 * @type  can_target_t
 * @brief Frame Target type 
 *
 * Typedef for a CAN Layer 2 Target Frame Identifier.
 */
typedef struct 
{
    uint32_t                     mask;
    uint32_t                     filter;
    can_l2_frame_handler_t  handler;
    uint8_t                      handler_id;
} can_l2_target_t;

/**
 * @type  iso15765_msg_t
 * @brief typedef of the ISO15765 Message
 *
 */
typedef struct
{
    uint8_t  address;
    uint16_t size;
    uint8_t  protocol;
    uint8_t *data;
} iso15765_msg_t;

/**
 * @type  iso15765_msg_handler
 * @brief typedef for a iso15765 message handler function.
 *
 */
typedef void (*iso15765_msg_handler_t)(iso15765_msg_t *msg);

/**
 * @type  iso15765_target_t
 * @brief ISO15765 Target
 *
 * ISO-15765 target
 */
typedef struct
{
    uint8_t                       protocol;
    iso15765_msg_handler_t   handler;
    uint8_t                       handler_id;
} iso15765_target_t;

/**
 * @type  iso11783_msg_t
 * @brief ISO11783 Message structure
 *
 */
typedef struct
{
    uint8_t source;
    uint8_t destination;
    uint8_t priority;
    uint32_t pgn;
    uint8_t *data;
} iso11783_msg_t;

/**
 * @type  iso11783_msg_handler
 * @brief ISO11783 Message Handler function.
 *
 * ISO-11783 Message Handler function.
 */
typedef void (*iso11783_msg_handler_t)(iso11783_msg_t *msg);

/**
 * @type  iso11783_target_t
 * @brief ISO11783 Target structure
 *
 * ISO-11783 target
 */
typedef struct
{
    uint32_t                      pgn;
    iso11783_msg_handler_t   handler;
    uint8_t                       handler_id;
} iso11783_target_t;

#endif // _CORE_H
