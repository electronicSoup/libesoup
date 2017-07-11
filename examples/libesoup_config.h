/**
 *
 * @file libesoup/example/libesoup_config.h
 *
 * @author John Whitmore
 *
 * This file contains an example libesoup libesoup_config.h configuration file. 
 *
 * The libesoup library of source code expects a libesoup_config.h header file to exist
 * in your include path. The file contains the various switches and definitions
 * which configure the various features of the library.
 *
 * Copyright 2015 John Whitmore <jwhitmore@electronicsoup.com>
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
#ifndef _SYSTEM_H
#define _SYSTEM_H

/**
 * @brief Physical Pin configuration of the Serial Logging port.
 *
 * On the cinnamonBun the the Gnd pin is physically fixed but the remaining two
 * pins use Microchip peripheral select functionality to configure which pin is
 * the Receive pin and which is the Transmit.
 *
 * There are two valid settings SYS_SERIAL_PORT_GndTxRx or SERIAL_PORT_GndRXTx 
 * 
 * Default : SYS_SERIAL_PORT_GndRxTx
 */
//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx

/*
 * Include a board file
 */
#if defined(__dsPIC33EP256MU806__)
#include "libesoup/boards/cb-dsPIC33EP256MU806.h"
#elif defined (__PIC24FJ256GB106__)
#include "libesoup/boards/cb-PIC24FJ256GB106.h"
#elif defined(__18F4585)
#include "libesoup/boards/gauge-PIC18F4585.h"
#elif defined(__RPI)
#include "libesoup/boards/rpi.h"
#endif

/**
 * @brief The required Instruction clock frequency of the device. 
 * 
 * The actual Hardware clock frequency is defined by the MACRO CRYSTAL_FREQ in
 * core.h. That constant is used in conjunction with this required SYS_CLOCK_FREQ 
 * in the function clock_init() to set the desired frequency with the PLL.
 *  
 * The function clock_init() has to be called on entry to main() to 
 * initialise the device to this clock frequency.
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
#define SYS_CLOCK_FREQ 16000000     // 8MHz
#elif defined(__dsPIC33EP256MU806__)
//#define SYS_CLOCK_FREQ 8000000     // 8MHz
#define SYS_CLOCK_FREQ 60000000    // 60MHz
#elif defined(__18F4585)
#define SYS_CLOCK_FREQ 16000000     // 8MHz
#endif

/**
 * @brief The size of the Transmit buffer to be used by the Serial Logging port.
 *
 * Default set to 300 Bytes as the serial port should be used for
 * relatively short debug messages and memory is limited.
 */
#define SYS_UART_TX_BUFFER_SIZE 300

/**
 * @brief Baud rate of the serial logging port
 *
 * This is Baud rate is set by the function serial_init(). This call to 
 * initialise the baud rate has to be preceeded by a call to clock_init() so
 * that the device's instruction clock speed is know, and the correct scaling
 * can be preformed for the required baud rate.
 * 
 * Default set to 19k2
 */
#define SYS_SERIAL_LOGGING_BAUD 19200

/**
 * @brief Serial logging level
 *
 * The valid log levels are defined in libesoup/core.h.
 * Default set to Debug logging level. This switch is used in conjunction
 * with libesoup/logging/serial_log.h. That file defines Logging macros which 
 * expect a logging level to be defined and a #define of "DEBUG_FILE" at the 
 * top of the file for logging to be enabled.
 *
 * #define TAG "MyMain"
 *
 * #if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
 *         LOG_D("logging message\n\r");
 # #endif
 */
#define SYS_LOG_LEVEL LOG_DEBUG

/**
 * @brief Number of Software timers available in the system.
 *
 * If your project includes libesoup Software Timer code then you must use this 
 * definition to define number of timers actually avalible in the system. 
 *
 * In addition to adding this Switch your code must initialise the timer
 * functionality and regularly check the timers with the "CHECK_TIMERS()" MACRO
 * defined in libesoup/timers/sw_timers.h
 *
 * For an example of using the Software timers take a look at:
 * libesoup/examples/main_sw_timers.c
 * 
 * Default : Switch is commented out assuming timer functionality not included
 * in project.
 */
//#define SYS_SW_TIMERS

#ifdef SYS_SW_TIMERS
#define SYS_NUMBER_OF_SW_TIMERS 20 
#define SYS_SW_TIMER_TICK_ms           5        // mSeconds
#endif // SYS_SW_TIMERS

/*
 * System jobs
 */
//#define SYS_JOBS

#ifdef SYS_JOBS
#define SYS_NUMBER_OF_JOBS 10
#endif

/*
 * MODBUS
 */
//#define SYS_MODBUS

#ifdef SYS_MODBUS
#define SYS_MODBUS_RX_BUFFER_SIZE                  256
#define SYS_MODBUS_RESPONSE_TIMEOUT                SECONDS_TO_TICKS(1)
#define SYS_MODBUS_RESPONSE_BROADCAST_TIMEOUT      MILLI_SECONDS_TO_TICKS(500)
#endif // SYS_MODBUS

/*
 * SYS_SPI Definitions
 */
/**
 * Enable the SYS_SPI_BUS switch if the project includes devices on the SPI
 * bus. EEPROM is one example.
 */
//#define SYS_SPI_BUS

/*
 * SYS_CAN Definitions
 */
/**
 * @brief SYS_CAN Bus Enable
 *
 * Switch to enable CAN Bus functionality. There are more specific switches for
 * various aspects of the CAN Bus functionality, of the libesoup, but this is
 * the global switch for CAN Bus.
 */
#define SYS_CAN                    // Default Disabled

#ifdef SYS_CAN
/**
 * @brief Number of CAN Bus Frame handlers.
 *
 * Code registers an interest in certain CAN Identifiers received by the libesoup
 * CAN Bus frame processing layer, (Layer 2). If your application is only
 * interested in one CAN Identifier then this array size can be limited to a
 * single entry.
 */
#define SYS_CAN_L2_HANDLER_ARRAY_SIZE 5

/**
 * @brief Size of the MCP2515 Rx circular buffer.
 *
 */
#define SYS_CAN_RX_CIR_BUFFER_SIZE    5


/**
 * @brief CAN BUS Baud Auto Detection
 *
 * Depending on the application it may be usefull to be able to automatically
 * detect CAN Bus Baud rate of the bus. Some networks have a specified Baud
 * rate, for example NMEA2000 is defined to operate at 250KB/S. In this case
 * there is no point in enabling Buad Auto Detection. In other cases where 
 * you are creating your own private CAN Bus Network, this may be of use.
 * Especially if your Network can dynamically change it's baud rate.
 *
 * There are a finite number of available Baud rates defined in an ennumerated
 * type (can_baud_rate_t) in libesoup/can/es_can.h  
 * 
 *     typedef enum {
 *             baud_10K   = 0x00,
 *             baud_20K   = 0x01,
 *             baud_50K   = 0x02,
 *             baud_125K  = 0x03,
 *             baud_250K  = 0x04,
 *             baud_500K  = 0x05,
 *             baud_800K  = 0x06,
 *             baud_1M    = 0x07,
 *             no_baud    = 0x08
 *     } can_baud_rate_t;
 * 
 */
//#define SYS_CAN_BAUD_AUTO_DETECT            // Default Disabled

#ifdef SYS_CAN_BAUD_AUTO_DETECT
/**
 * @brief  CAN Baud auto detech listen period
 *
 * If the CAN Layer 2 is required to listen for the Connected Baud rate this
 * macro defines the listening period. The default is 20 Seconds.
 *
 * Basically if Auto Detect is enabled then the code will listen for this
 * period of seconds for valid CAN Layer 2 traffic. If no valid CAN Frames 
 * or Errors are detected during this listening period then the code moves on
 * to the next Baud rate and listens again for this period of time.
 *
 * If on the other hand Valid Frames are received and no Errors then we've
 * found the network's Baud rate.
 */
#define SYS_CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD    20       // Default 20 Seconds
#endif // SYS_CAN_BAUD_AUTO_DETECT

/**
 * @brief Enable the CAN Ping Protocol
 *
 * The Ping Protocol.
 *
 * If your CAN Bus network will rely on auto detection of the Baud rate,
 * then that auto detection depends on valid traffic on the network. 
 * This "Ping Protocol", when enabled, will have your CAN Node send a spurious
 * CAN Frame onto the network if there has been no activity on the network for
 * SYS_CAN_PING_PROTOCOL_PERIOD +/- 0.5 Seconds.
 *
 * The actual exact period defined by SYS_CAN_PING_PROTOCOL_PERIOD is not used but
 * rather a random duration within one second of that duration is used. For
 * that reason if you're code uses the Ping Protcol then it should include 
 * libesoup/utils/rand.c and during startup initialise a random seed with a call
 * to the function:
 * 
 * random_init();
 *
 * The reason for the random duration being used is so that you can enable 
 * all your nodes to include the Ping Protocol. If a number of nodes were all
 * using the same duration for the Ping Protocol then they would all flood the
 * network at regular intervals. With the random shift in duration one of the 
 * nodes will have the shorter period and will jump first. The other nodes will
 * see that Ping Message as valid network traffic and reset their timers.
 *
 * You might see the Ping Protocol as an overhead, but if there's no traffic on
 * the network then nothing is happening anyhow.
 */
//#define SYS_CAN_PING_PROTOCOL        // Default Disabled

/**
 * @brief Ping Protocol idle period
 *
 * See the comments for SYS_CAN_PING_PROTOCOL above. 
 * 
 * Basically if the SYS_CAN Layer 2 functionality detects no activity on the SYS_CAN 
 * Bus for this duration +/- 1 Second then a Ping Frame will be transmitted
 * on the network.
 *
 * Value is a duration in Seconds. 
 *
 * Only used if SYS_CAN_PING_PROTOCOL is enabled.
 *
 */
#ifdef SYS_CAN_PING_PROTOCOL
#define SYS_CAN_PING_PROTOCOL_PERIOD     5    // Seconds
#endif  // SYS_CAN_PING_PROTOCOL

/**
 * @brief Enable the Dynamic CAN Node Configuration Protocol (DCNCP)
 *
 * As the name suggests DCNCP is a collection of messages, both
 * Layer 2 and Layer 3, which can be used to dunamically configure a CAN Node
 * across the CAN Bus Network. (Think DHCP in the TCP/IP World)
 *
 * There are various parameters which can be configured including, for example,
 * the Baud rate of the network. In the case of a Global Configuration, like
 * Baud rate, all Nodes on the network must have DCNCP enabled. 
 */
//#define SYS_CAN_DCNCP               // Default Disabled

#ifdef SYS_CAN_DCNCP

/**
 * @brief DCNCP Network Baud Rate
 *
 * Enabling this switch enables code for dynamically changing the Baud rate of
 * the CAN Bus network. 
 * 
 * WARNING !!!  For this option to be viable *ALL* nodes on the network
 * must have this ability enabled. If a single node does not comply with this
 * Protocol it will be left behind, on the old Baud Rate, and will effectively 
 * broadcast noise on the network.
 *
 * Functionality: If this mechanism is invoked a series of 3 count down messages
 * are broadcast on the network. Each message contains a countdown timer and 
 * the new Buad rate to be used by the network. The new baud rate is specified
 * as a member of the can_baud_rate_t enumerated type (can/es_can.h). So the series
 * of countdown messages are, in the case of moving to baud_250K:
 *
 * 3 Seconds, baud_250K
 * 2 Seconds, baud_250K
 * 1 Second,  baud_250K
 *
 * When a node receives the message it starts a timer for the specified duration.
 * If the node misses the following two countdown messages it will still change
 * to the new Baud rate regardless. 
 * 
 * When one of the following countdown messages is received the existing timer
 * is canceled and re-started with the new countdown timer value.
 *
 * It is extremely unlikely that a node should miss All three countdown 
 * messages. If a node misses all three then Baud rate is the least of your 
 * worries.
 *
 * Once the timer finally expires the node disconnects from the CAN Node, 
 * changes its Baud rate and waits for 10 Seconds before rejoining the Network.
 * If all nodes comply with the DCNCP Baud Rate Protocol the network will now
 * be operating at the new network Baud Rate.
 */
//#define SYS_CAN_DCNCP_BAUDRATE      // Default Disabled

/**
 * @brief Include SYS_ISO15765 Functionality
 *
 * SYS_ISO15765 has a dependance on random! Add libesoup/utils/rand.c to your project 
 * and, to initialise a seed, on startup call:
 * 
 * init_rand();
 *
 * SYS_ISO15765 defines a Layer 3 Protocol which sits on top of the CAN Bus,
 * Later 2, protocol. CAN Bus transmits frames with up to 8 Bytes of Data. That
 * is perfect in a field bus where status data is being send across the network.
 * However depending on your applicaiton 8 bytes of data can be a limiting
 * factor. The SYS_ISO15765 protocol adds the ability to send larger messages
 * across the Bus. The larger messages are segmented into short frames for 
 * transmission across the bus and reassembly on the other side.
 *
 * The libesoup implementation of SYS_ISO15765 is not a strict implementation of the
 * standard. For example the standard enables a maximum message size for 
 * transmission of 4095 Bytes whilst libesoup defines SYS_ISO15765_MAX_MSG as 270
 * in libesoup/core.h. Seperate from this size is a Protocol or Layer 3 message
 * type. The libesoup implementation defines the first byte of the transmitted
 * message as a Protocol Identifier which specifies the type of the message.
 *
 * The libesoup CAN implementation reserves two SYS_ISO15765 Protocol ID's for
 * specific use in libesoup/can/es_can.h:
 * 
 * #define SYS_ISO15765_LOGGER_PROTOCOL_ID    0x01
 * #define SYS_ISO15765_DCNCP_PROTOCOL_ID     0x02
 *
 * If a SYS_ISO15765 Message is received with a frist Byte of 0x01 then the message
 * is passed to the network logger functionality. Likewise 0x02 is passed to 
 * Layer 3 of DCNCP Handler.
 *
 * Unlike Layer 2 SYS_ISO15765, Layer 3, is an addressed protocol. Messages are 
 * sent from a node with a specific address across the network to a node 
 * with a destination address. Therefore a CAN Node using SYS_ISO15765 must have
 * a BYTE value defining the Layer 3 address of the node.
 *
 * The SYS_ISO15765 functionality in the libesoup code does not dictate how a Layer
 * 3 node address is obtained. If your project is including SYS_ISO15765 you will
 * have to add a function to the project which the library code can call to 
 * obtain it's layer 3 node address.
 *
 *     uint8_t node_get_address(void)
 *     {
 *             return(0x01);
 *     }
 *
 * The above example, whilst valid, is too simplistic to be used in producton.
 * Whilst the libesoup SYS_ISO15765 functionality will request a Layer 3 node address
 * from your higher level code a DCNCP peer to peer based protocol is used to 
 * confirm that the address is not already in use on the network. If the Layer
 * 3 node address you have specified is already in use your "node_get_address()
 * function will be recalled!
 *
 * The CAN Node OS stores the Layer 3 node address in EEPROM and uses that
 * address as its "first choice". If that address is rejected it sleects another
 * address at random and stores the new address in EEPROM:
 *  
 *     uint8_t node_get_address(void)
 *     {
 *             static uint8_t retry_count = 0;
 *	       result_t rc;
 *	       uint8_t address;
 *
 *	       if(retry_count == 0) {
 *		       rc = eeprom_read(EEPROM_NODE_ADDRESS, &address);
 *		       if (rc != SUCCESS) {
 * #if (SYS_LOG_LEVEL <= LOG_ERROR)
 *		               LOG_E("address Failed to read from eeprom return code 0x%x\n\r", rc);
 * #endif
 *			       do {
 *				       address = (uint8_t) (rand() & 0x0ff);
 *			       } while (address == BROADCAST_NODE_ADDRESS);
 *
 *			       rc = eeprom_write(EEPROM_NODE_ADDRESS, address);
 *			       if (rc != SUCCESS) {
 * #if (SYS_LOG_LEVEL <= LOG_ERROR)
 *			       	       LOG_E("Failed to write from eeprom return code 0x%x\n\r", rc);
 * #endif
 *			       }
 *		       } else {
 *			       LOG_D("address value read back from EEPROM address 0x02 = 0x%x\n\r", address);
 *		       }
 *	       } else {
 *		       do {
 *			       address = (uint8_t) (rand() & 0x0ff);
 *		       } while (address == BROADCAST_NODE_ADDRESS);
 *		
 *		       rc = eeprom_write(EEPROM_NODE_ADDRESS, address);
 *		       if (rc != SUCCESS) {
 * #if (SYS_LOG_LEVEL <= LOG_ERROR)
 *			       LOG_E("Failed to write from eeprom return code 0x%x\n\r", rc);
 * #endif
 *		       }
 *	       }
 *
 *	       retry_count++;
 *
 *	       LOG_D("node_get_address() Retry %d try address 0x%x\n\r", retry_count, address);
 *	       return(address);
 *     }
 *
 * The above node_get_address() function can be called multiple times by the
 * libesoup code until it finds an address which is not being used.
 *
 * If your network is your own private network and it only has limited nodes
 * then you can hardcode an address into each node on the network.
 *
 */
//#define SYS_ISO15765                 // Default Disabled

#ifdef SYS_ISO15765
/**
 * @brief Size of SYS_ISO15765 register array.
 *
 * The libesoup implementation of SYS_ISO15765 uses the first byte of each message
 * as a message type, or Layer 3 Protocol Identifier. So in theory you can send 
 * 256 different message types between nodes. This number is reduced as libesoup 
 * reserves two layer 3 protocol numbers for specific uses.
 *
 * Default is 2
 */
#define SYS_ISO15765_REGISTER_ARRAY_SIZE 2

/**
 * @brief Enable SYS_ISO15765 Logging functionality
 * 
 * Enable this switch if your project uses the SYS_ISO15765 Logging functionality
 * to send information across the CAN Bus network. The code for this 
 * functionality is in files libesoup/logger/iso15765_log.c and iso15765_log.h.
 * your code can send debug messages with a call to:
 * 
 *     iso15765_log(log_level_t level, char *msg);
 * 
 * So for example:
 *
 *     iso15765_log(Debug, "Hello World");
 *
 * SYS_ISO15765 Layer 3 Protocol ID 0x01 is reserved for this functionality.
 */
//#define SYS_ISO15765_LOGGING           // Default Disabled

/**
 * @brief Enable SYS_ISO15765 Logger functionality
 * 
 * If this node can act as the CAN Bus Network logger enable this swith.
 *
 * The above switch (SYS_ISO15765_LOGGING) allows your firmware to send debug
 * messages across the CAN Bus Network to an SYS_ISO15765 Network Logger. This 
 * switch enables the code to allow this node to be the Network Logger.
 * 
 * This switch should only be enabled in one single node on the network as
 * the SYS_ISO15765 Protocol only sends to a specific Layer 3 node addres. 
 *
 */
//#define SYS_ISO15765_LOGGER             // Default Disabled

/**
 * @brief SYS_ISO15765 Network logger Ping Period (Seconds)
 *
 * If this node has enabled Network Logger functionality this switch defines 
 * the period at which register request messages are repeated.
 *
 * Nodes on the network can make calls to log messages across the network with
 * calls to:
 *
 *     iso15765_log(Debug, "Hello World");
 *
 * The libesoup SYS_ISO15765 logging code will actually totally ignore these requests
 * if it does not know of a registered SYS_ISO15765 Network Logger. An SYS_ISO15765
 * message is sent to a specific destination so if no destination address is
 * known then there is nowhere to send the messages.
 *
 * To ensure that the various nodes on the network know where to send SYS_ISO15765
 * logging messages the Network Logger will periodically transmist a message
 * basically meaning "I am the network logger". Because we're on a network 
 * nodes can power up and down in different orders and different time so this
 * Network Logger Registration message is repeated periodically so that all
 * nodes where to send logging info.  
 *
 * The default is 60 Seconds.
 */
#ifdef SYS_ISO15765_LOGGER
#define SYS_ISO15765_LOGGER_PING_PERIOD 60        // Seconds 
#endif // SYS_ISO15765_LOGGER

/**
 * @brief Enable Dynamic Can Node Configuration Protocol in SYS_ISO15765 layer.
 *
 * We've previously discussed SYS_ISO15765 Network logging functionality. The other
 * reserved SYS_ISO15765 Layer 3 protocol, 0x02 (libesoup/can/es_can.h), is for the
 * Dynamic CAN Node Configuration Protocol. 
 */
#define SYS_ISO15765_DCNCP

#endif // SYS_ISO15765

/**
 * @brief Enable the SYS_ISO11783 Protocol
 *
 * SYS_ISO15765 is just one Layer 3 protocol which sits on top of SYS_CAN Bus. SYS_ISO11783
 * also piggy backs on top of SYS_CAN Bus. SYS_ISO11783 calls itself a Layer 2 Protocol
 * so things might get a bit confusing if we're talking in terms of layers.
 * 
 * SYS_ISO11783 is the Protocol used by the NMEA2000 Protocol in Marine
 * applicaiton and by SAE J1939 in the Automotice applications.
 *
 * 
 */
#define SYS_ISO11783                                 // Default Disabled

#ifdef SYS_ISO11783
/**
 * @brief Size of SYS_ISO11783 register array size
 *
 * The number of different SYS_ISO11783 GRP handlers that can be registered
 * in the system or applicaiton.
 */
#define SYS_ISO11783_REGISTER_ARRAY_SIZE 2

#endif // SYS_ISO11783

#endif // SYS_CAN_DCNCP
#endif // SYS_CAN

/*
 * Android Definitions:
 *
 */
/**
 * @brief Enable Android Functionality
 *
 * If your project communicates with an Android Device then enable this switch.
 */
//#define SYS_ANDROID                        // Default Disabled

#ifdef SYS_ANDROID
/**
 * @brief Android App connected state handler.
 *
 * Interaction with a connected Android device is handeled in libesoup by a small
 * state machine. On startup your firmware should initialise the android state 
 * to the idle state:
 *
 *     #include "libesoup/usb/android/state_idle.h"
 *
 *     set_idle_state();
 *
 * The Android state structure is defined in libesoup/usb/android/state.h
 *
 *     typedef struct android_state_t
 *     {
 *         void (*process_msg)(BYTE, void *, UINT16);
 *         void (*main)(void);
 *         void (*process_usb_event)(USB_EVENT event);
 *     } android_state_t;
 *
 * Basically an Android state contains three pointers to functions. One function
 * processes messages from the Android Device, one is the main loop processing 
 * which is called regularly and the final function pointer is called to process
 * USB Events.
 *
 * The current android state is stored in a variable defined in
 * libesoup/usb/android/state.h:
 * 
 *     extern android_state_t android_state;
 * 
 * libesoup contains only two android states, namely the idle state and the
 * Android device connected state. As has been said on startup your firmware
 * should set the idle state. Once an Android device is physically connected
 * the libesoup changes state to the device connected state. Now once an Android
 * Application starts up which communicates with your firmware libesoup's Android
 * device connected state will trasition to the App Connected state.
 * 
 * What your firmware does with the connected Android Application is up to you
 * you simply have to define a function which will set the correct Android state
 * functions and tell libesoup what function to call with the 
 * SYS_ANDROID_SET_APPLICATION_CONNECTED_STATE switch:
 *  
 *     extern void example_set_app_connected_state(void);
 *     #define SYS_ANDROID_SET_APPLICATION_CONNECTED_STATE example_set_app_connected_state();
 *
 * With this in place the your function will be called to set the correct state
 * when an Android App accepts responsibility for your firmware.
 */
#define SYS_ANDROID_SET_APPLICATION_CONNECTED_STATE set_app_connected_state();
extern void set_app_connected_state(void);

/**
 * @brief Android no App connected functionality.
 *
 * If your firmware is designed to interact with a connected Android device
 * what does it do when there is no Android device connected? The answer to
 * that question is in the SYS_ANDROID_NO_APP_FN switch.
 *
 * This is NOT Mandatory. If you project does not require it then don't
 * define the MACRO.
 *
 */
#define SYS_ANDROID_NO_APP_FN example_no_android_app_function();
extern void example_no_android_app_function(void);

#endif // SYS_ANDROID

/**
 * @brief Use Bootloader EEPROM space.
 *
 * By default libesoup assumes that your firmware is using the bootloader and as
 * a result the lower section of EEPROM is reserved for the Bootloader. The 
 * number of bytes reserved for the Bootloader is defined in libesoup/core.h
 *
 *     #define SYS_EEPROM_BOOT_PAGE_SIZE   0x04
 *
 * If you intend to directly program your project into the Flash with a Flash
 * programmer, like the PICkit-3, in what I refer to as "Raw" mode then you
 * can use the area of EEPROM normally reserved for use by the Bootloader.
 */
//#define SYS_EEPROM_USE_BOOT_PAGE         // Default disabled

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



#endif //SYSTEM_H
