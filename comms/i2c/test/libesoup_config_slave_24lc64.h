#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#define SYS_CLOCK_FREQ (7600000/2)

//#define DEBUG_BUILD
#define SYS_TEST_SLAVE_24LC64
        
#define SYS_TEST_BUILD

#define SYS_CHANGE_NOTIFICATION

#ifdef SYS_CHANGE_NOTIFICATION
  #define SYS_CHANGE_NOTIFICATION_MAX_PINS      5
#endif

#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL LOG_DEBUG
#define SYS_UART1
#define SYS_SERIAL_LOGGING_BAUD           19200
#define SYS_UART_TX_BUFFER_SIZE            1024

#endif // defined(SYS_SERIAL_LOGGING)


#define  SYS_SW_TIMERS

#ifdef SYS_SW_TIMERS
  #define  SYS_HW_TIMERS
  #define  SYS_NUMBER_OF_SW_TIMERS              5
  #define  SYS_SW_TIMER_TICK_ms                 5
#endif

//#define SYS_I2C1
#define SYS_SLV_24LCxx
//#define SYS_TEST_MASTER_24LC64
#define SYS_TEST_SLAVE_24LCxx

/*
 * SD Card switches
 */
#define SYS_SPI1
#define SYS_SD_CARD

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
