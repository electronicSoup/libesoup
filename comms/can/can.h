/**
 * @file libesoup/comms/can/can.h
 *
 * @author John Whitmore
 *
 * Core CAN Bus definitions required by electronicSoup Code Library
 *
 * Copyright 2017 electronicSoup Limited
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
