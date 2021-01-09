#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#if defined(__dsPIC33EP128GS702__)
#define SYS_CLOCK_FREQ (7600000/2)
#elif defined(__dsPIC33EP256MU806__)
#define SYS_CLOCK_FREQ 60000000
#endif

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
//#define SYS_SLV_24LCxx
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
#if defined(__dsPIC33EP128GS702__)
#include "libesoup/boards/stereo_guitar.h"
#elif defined(__dsPIC33EP256MU806__)
#include "libesoup/boards/cinnamonBun/dsPIC33/cb-dsPIC33EP256MU806.h"
#endif
/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */

#if defined(__dsPIC33EP128GS702__)
/*
 * SD Card
 * Clk RB11
 * WriteP  RB15
 * CMD RB13
 * DAT3/CD (SS) RB14
 * DAT0 RA0
 * DAT1 RA1

 */
#define SD_CARD_DETECT  RB5
#define SD_CARD_WRITE_P RB15

#define SD_CARD_SCK     RB11   // GREY    Idle High Works
#define SD_CARD_MOSI    RB13   // RED     Idle Low no activity
#define SD_CARD_SS      RB14   // ORANGE  Idle low no activity
#define SD_CARD_MISO    RA0    // YELLOW  Idle Hight no Activity

#elif defined(__dsPIC33EP256MU806__)

#define SD_CARD_DETECT  RD0
#define SD_CARD_WRITE_P RD1

#define SD_CARD_SCK     RD3   // GREY    Idle High Works
#define SD_CARD_MOSI    RD4   // RED     Idle Low no activity
#define SD_CARD_SS      RD5   // ORANGE  Idle low no activity
#define SD_CARD_MISO    RD2    // YELLOW  Idle Hight no Activity
#endif

#endif // _LIBESOUP_CONFIG_H
