#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#define DEBUG_BUILD
#define SYS_TEST_CONTROL_CHAIN
#define SYS_TEST_BUILD

#define SYS_CLOCK_FREQ 60000000

#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL LOG_DEBUG
#define SYS_UART
#define SYS_SERIAL_LOGGING_BAUD           19200
#define SYS_UART_TX_BUFFER_SIZE            1024

#endif // defined(SYS_SERIAL_LOGGING)

#include "libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h"

/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */

#endif // _LIBESOUP_CONFIG_H
