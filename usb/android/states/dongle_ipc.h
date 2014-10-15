/**
 *
 * \file dongle_ipc.h
 *
 * Definitions for communications with an Android Device for ES CAN Dongle
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
#ifndef DONGLE_IPC_H
#define DONGLE_IPC_H

//#include "can/can_ids.h"
//#include "can/l2_can_types.h"

/**
 * \brief message types that can be sent between the Dongle an the Android
 * App.
 *
 * Check out the Android Library ESCanDongle DongleMessage.java
 */
typedef enum android_command_t
{
    /**
     * Bi-Directional message sent to the app when it connects to tell the
     * connected Application what Baud Rate the network is running at.
     * Sent from the Android App to this Dongle to force it to change the
     * Baud Rate of the Network.
     */
    AC_BAUD_RATE           = 0x01,

    AC_REG_CAN_INTEREST    = 0x02,

    AC_CAN_L2_MESSAGE      = 0x03,
    AC_CAN_L3_MESSAGE      = 0x04,

    AC_CAN_STATUS          = 0x05,
            
    COMMAND_APP_CONNECT    = 0xFE,
    COMMAND_APP_DISCONNECT = 0xFF
} android_command_t;

#endif //DONGLE_IPC_H
