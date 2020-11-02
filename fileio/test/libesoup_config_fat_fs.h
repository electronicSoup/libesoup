#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#if defined(__dsPIC33EP128GS702__)
#define SYS_CLOCK_FREQ (7600000/2)
#elif defined(__dsPIC33EP256GP502__)
#define SYS_CLOCK_FREQ (7370000/2)
#elif defined(__dsPIC33EP256MU806__)
#define SYS_CLOCK_FREQ 60000000
#endif

//#define FAT_FS_TEST_MAIN

#define SYS_TEST_BUILD

#define SYS_CHANGE_NOTIFICATION

#ifdef SYS_CHANGE_NOTIFICATION
  #define SYS_CHANGE_NOTIFICATION_MAX_PINS      5
#endif

//#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL LOG_DEBUG
#define SYS_UART1
#define SYS_SERIAL_LOGGING_BAUD           19200
#define SYS_UART_TX_BUFFER_SIZE            1024 //3072

#endif // defined(SYS_SERIAL_LOGGING)


#define  SYS_SW_TIMERS

#ifdef SYS_SW_TIMERS
  #define  SYS_HW_TIMERS
  #define  SYS_NUMBER_OF_SW_TIMERS              5
  #define  SYS_SW_TIMER_TICK_ms                 5
#endif

/*
 * SD Card switches
 */
#if defined(__dsPIC33EP128GS702__)
//#define SYS_SPI1
#elif defined(__dsPIC33EP256GP502__)
#define SYS_SPI2
#endif
#define SYS_SD_CARD
#define SYS_FAT_FS

//#define I2C_TEST_MAIN
#define FAT_FS_TEST_MAIN

/*
 * Include a board file
 */
#if defined(__dsPIC33EP128GS702__)
#include "libesoup/boards/stereo_guitar/stereo_guitar_dsPIC33EP128GS702.h"
#elif defined(__dsPIC33EP256GP502__)
#include "libesoup/boards/stereo_guitar/stereo_guitar_dsPIC33EP256GP502.h"
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
