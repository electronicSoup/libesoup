#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#define DEBUG_BUILD
#define SYS_TEST_SLAVE_24LC64
        
#define SYS_TEST_BUILD

#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL LOG_DEBUG
#define SYS_UART
#define SYS_SERIAL_LOGGING_BAUD           19200
#define SYS_UART_TX_BUFFER_SIZE             512

#endif // defined(SYS_SERIAL_LOGGING)

#define SYS_I2C3
#define SYS_SLV_24LC64

/*
 * Include a board file
 */
#include "libesoup/boards/cinnamonBun/pic24FJ/cb-PIC24FJ256GB106.h"

/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */

#endif // _LIBESOUP_CONFIG_H
