/**
 *
 * \file libesoup/usb/android/android.h
 *
 * Function prototypes for Android Comms
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
#ifndef ANDROID_COMMS_H
#define ANDROID_COMMS_H

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



extern void      android_init(void* data);
extern void      android_set_tx_finished_callback(void (*data)(void));
extern void      android_tasks(void);
extern uint8_t   android_transmit(uint8_t *buffer, uint8_t size);


#endif /* ANDROID_COMMS_H */
