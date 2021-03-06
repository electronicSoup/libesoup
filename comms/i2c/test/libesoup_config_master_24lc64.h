#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#define DEBUG_BUILD
#define SYS_TEST_24LC64
        
#define SYS_TEST_BUILD

#define SYS_HW_TIMERS
#define SYS_SW_TIMERS
#ifdef SYS_SW_TIMERS
#define SYS_NUMBER_OF_SW_TIMERS    10
#define SYS_SW_TIMER_TICK_ms        5
#endif

#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL LOG_DEBUG
#define SYS_UART1
#define SYS_SERIAL_LOGGING_BAUD           19200
#define SYS_UART_TX_BUFFER_SIZE            (2 * 1024)

#endif // defined(SYS_SERIAL_LOGGING)

#define SYS_I2C3
#define SYS_24LC64

#define SYS_TEST_MASTER_24LC64

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
