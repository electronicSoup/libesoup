#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#define SYS_CLOCK_FREQ (7600000/2)

//#define DEBUG_BUILD
#define SYS_TEST_SLAVE_24LC64
        
#define SYS_TEST_BUILD

#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL LOG_DEBUG
#define SYS_UART1
#define SYS_SERIAL_LOGGING_BAUD           19200
#define SYS_UART_TX_BUFFER_SIZE             512

#endif // defined(SYS_SERIAL_LOGGING)

#define SYS_I2C1
#define SYS_SLV_24LC64
//#define SYS_TEST_MASTER_24LC64
#define SYS_TEST_SLAVE_24LC64
/*
 * Include a board file
 */
#include "libesoup/boards/stereo_guitar.h"

/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */

#endif // _LIBESOUP_CONFIG_H
