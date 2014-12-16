/**
 *
 * \file es_lib/example_system.h
 *
 * This file contains the various switches which can be used to configure
 * the es_lib source code library. 
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

#include "es_lib/core.h"

/*
 * Baud rate of the serial debug port
 */
#define SERIAL_BAUD 19200

/*
 * The configuration of the Serial Port there are 3 pins, the Gnd pin is fixed
 * so you define the configuration of the other two pins. Uncomment as required
 */
//#define SERIAL_PORT_GndTxRx
#define SERIAL_PORT_GndRxTx

/*
 * Log level. these are defined in es_lib/core.h as they are included everywhere
 */
#define LOG_LEVEL LOG_DEBUG

/*
 * NUMBER_OF_TIMERS
 * 
 * Definition of the number of system timers required in the system. Increasing 
 * the number of timers uses more system RAM. This will depend on the estimated
 * demand for timers in your design.
 */
#define NUMBER_OF_TIMERS 20

/*
 * CAN Definitions
 */
//#define CAN

#ifdef CAN
#define CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD    SECONDS_TO_TICKS(10)
/*
 * The number of Handlers that can be registered with Layer 2
 */
#define CAN_L2_HANDLER_ARRAY_SIZE 5
#endif

//#define CAN_L2_IDLE_PING

#ifdef CAN_L2_IDLE_PING
#define CAN_L2_IDLE_PING_FRAME_ID 0x666
/*
 * The Ping Period will be between CAN_IDLE_PERIOD - 0.5 Seconds and +0.5 Seconds
 * As a result this shold probably be greater then 1 Second. If a node picks a 
 * random value of Zero then it'll do nothing but ping!
 */
#define CAN_L2_IDLE_PING_PERIOD     SECONDS_TO_TICKS(2)
#endif

/*
 * Include CAN Layer 3 functionality
 */
//#define CAN_LAYER_3


/*
 * Android Definitions:
 *
 */
/*
 * Definition of the function used to change State to the App connected
 * state. If you are going to use an Android connected device you must
 * define a function for moving into the App Connected state. See example
 * below:
 *
 *   extern void example_set_app_connected_state(void);
 *   #define ANDROID_SET_APPLICATION_CONNECTED_STATE example_set_app_connected_state();
 */

/*
 * Android main processing when the Android Application is NOT Connected.
 * If different processing is required when the Android Applicaiton is not
 * connected to your device then define the function to perform this 
 * functionality and the MACRO.
 * This is NOT Mandatory. If you project does not require it then don't 
 * define the MACRO.
 *
 *  extern void example_no_android_app_function(void);
 *  #define NO_ANDROID_APP_FN example_no_android_app_function();
 */

/*
 * If project is to use the BOOT Page of EEPROM then define this option.
 */
//#define EEPROM_USE_BOOT_PAGE

/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */

/*
 * Android IPC Messages.
 *
 * App message is transmitted from the Android Device App to the Cinnamom Bun
 * Bun message is transmitted from the Cinnamon Bun to the Android Device App
 *
 * Messages for communications with an Android App should be defined as 
 * project specific constants relative to USER_OFFSETS.
 *
 * #define MY_FIRST_BUN_MSG     BUN_MSG_USER_OFFSET
 * #define MY_SECOND_BUN_MSG    BUN_MSG_USER_OFFSET + 1
 *
 * #define MY_FIRST_APP_MSG     APP_MSG_USER_OFFSET
 * #define MY_SECOND_APP_MSG    APP_MSG_USER_OFFSET + 1
 *
 */
