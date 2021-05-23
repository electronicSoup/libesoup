#if defined(__dsPIC33EP256GP502__)
#ifndef _MINI_LOOPER_H
#define _MINI_LOOPER_H

#include "libesoup/processors/dsPIC33/es-dsPIC33EP256GP502.h"

/**
 * @brief Crystal Frequency of the Hardware Device.
 *
 * The dsPIC33 contains a Phase Locked Loop so this crystal frequency is not
 * necessarily the instruction clock used by the device. The libesoup_config.h
 * file defines the requested instruction clock frequency, which will be 
 * configured, as part of libesoup_init() on startup.
 */
#define BRD_CRYSTAL_FREQ 7370000

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
        #define BRD_SERIAL_LOGGING_TX_PIN  RA4
        #define BRD_SERIAL_LOGGING_RX_PIN  RB4
#elif defined(SYS_SERIAL_PORT_GndRxTx)
        #define BRD_SERIAL_LOGGING_TX_PIN  RB4
        #define BRD_SERIAL_LOGGING_RX_PIN  RA4
#else
#error Serial Logging pin orientation not defined!
#endif
#endif // SYS_SERIAL_LOGGING

#define BRD_MCP49x2_CS RB12
#include "libesoup/core.h"

#endif // _MINI_LOOPER_H

#endif // defined(__dsPIC33EP256GP502__)
