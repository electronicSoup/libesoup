/**
 *
 * \file libesoup/can/es_can.h
 *
 * Core CAN Bus definitions
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
#ifndef ES_CAN_H
#define ES_CAN_H

#include "libesoup_config.h"

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

/*
 * ISO15765 Protocols
 */
#if defined(SYS_ISO15765)
#define ISO15765_LOGGER_PROTOCOL_ID    0x01
#define ISO15765_DCNCP_PROTOCOL_ID     0x02
#endif

/*
 * A Node address of 0xff is a Broadcast Network node address in ISO11783 so
 * we'll not use it.
 */
#define BROADCAST_NODE_ADDRESS         0xff

/*
 * Have to change this status to cater for L2, DCNCP and L3
 *
 * Layer 2 - 5 values so 3 bits L2_STATUS_MASK 0x07
 *
 *	L2_Uninitialised 0x00,
 *	L2_Listening     0x01,
 *	L2_Connecting    0x02,
 *	L2_Connected     0x03,
 *	L2_ChangingBaud  0x04
 *
 * DCNCP - 3 Values for status so 2 bits
 *         Actually use a bit filed for this
 *
 *         DCNCP_STATUS_INITIALISED_MASK 0x08
 *
 *             DCNCP_Uninitilised 0x00,
 *             DCNCP_Initialised  0x08,
 *
 *         DCNCP_L3_ADDRESS_STATUS_MASK 0x10
 *
 *             DCNCP_NODE_Address_Not_Final 0x00,
 *             DCNCP_NODE_Address_Finalised 0x10,
 *
 * Layer 3 - 2 values Initialised or not. 1 bit
 *
 *         L3_STATUS_INITIALISED_MASK 0x20
 *
 *             L3_Uninitialised 0x00
 *             L3_Inititialised 0x20
 */

#define L2_STATUS_MASK                 0x07

#define L2_Uninitialised               0x00
#define L2_Listening                   0x01
#define L2_Connecting                  0x02
#define L2_Connected                   0x03
#define L2_ChangingBaud                0x04

#if SYS_LOG_LEVEL < NO_LOGGING
extern char can_l2_status_strings[5][17];
#endif

#ifdef SYS_CAN_DCNCP
#define DCNCP_INIT_STATUS_MASK         0x08
#define DCNCP_NODE_ADDRESS_STATUS_MASK 0x10
#endif // SYS_CAN_DCNCP

typedef struct {
    union {
        struct {
            uint8_t l2_status : 3;
            uint8_t dcncp_initialised : 1;
            uint8_t dcncp_node_address_valid :1;
        } bit_field;
        uint8_t byte;
    };
} can_status_t;

typedef enum {
	baud_10K   = 0x00,
	baud_20K   = 0x01,
	baud_50K   = 0x02,
	baud_125K  = 0x03,
	baud_250K  = 0x04,
	baud_500K  = 0x05,
	baud_800K  = 0x06,
	baud_1M    = 0x07,
	no_baud    = 0x08
} can_baud_rate_t;

typedef void (*can_status_handler_t)(can_status_t, can_baud_rate_t);

#if SYS_LOG_LEVEL < NO_LOGGING
extern char can_baud_rate_strings[8][10];
#endif

//#define BAUD_MAX baud_1M

extern result_t can_init(can_baud_rate_t      baud,
	can_status_handler_t   status_default_handler);

//extern bool can_initialised(void);

extern result_t can_l2_init(can_baud_rate_t arg_baud_rate,
                 void (*arg_status_handler)(uint8_t mask, can_status_t status, can_baud_rate_t baud));
extern void can_l2_tasks(void);

extern result_t can_l2_tx_frame(can_frame *frame);
extern result_t can_l2_dispatch_reg_handler(can_l2_target_t *target);
extern result_t can_l2_dispatch_unreg_handler(uint8_t id);
extern result_t can_l2_dispatch_set_unhandled_handler(can_l2_frame_handler_t handler);

//extern void can_l2_ISR(void);

//extern void can_l2_tx_error(uint8_t node_type, u8 node_number, u32 errorCode);

extern can_baud_rate_t can_l2_get_baudrate(void);
extern void can_l2_set_node_baudrate(can_baud_rate_t baudrate);
extern void can_l2_get_status(can_status_t *, can_baud_rate_t *);

#if defined(XC16) || defined(__XC8)
extern void can_tasks(void);
#endif


#if defined(ISO15765)

extern uint8_t node_get_address(void);

extern result_t iso15765_init(uint8_t address);
extern uint8_t iso15765_initialised(void);

extern result_t iso15765_tx_msg(iso15765_msg_t *msg);
extern result_t iso15765_dispatch_reg_handler(iso15765_target_t *target);
extern result_t iso15765_dispatch_unreg_handler(uint8_t id);
extern result_t iso15765_dispatch_set_unhandled_handler(iso15765_msg_handler_t handler);

#endif

#if defined(ISO11783)


typedef enum {
    ack            = 0x00,
    nack           = 0x01,
    denied         = 0x02,
    cannot_respond = 0x03
} iso11783_ack_t;


extern result_t  iso11783_init(uint8_t);

extern result_t iso11783_tx_msg(iso11783_msg_t *msg);
extern result_t iso11783_dispatch_reg_handler(iso11783_target_t *target);
extern result_t iso11783_dispatch_unreg_handler(uint8_t id);
extern result_t iso11783_dispatch_set_unhandled_handler(iso11783_msg_handler_t handler);
#endif // SYS_ISO11783

#endif // ES_CAN_H
