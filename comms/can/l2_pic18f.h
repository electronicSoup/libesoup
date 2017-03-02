#ifndef L2_PIC18F_H
#define L2_PIC18F_H

#include "system.h"

/*
 * Interrupt Flags
 */
#define MERRE 0x80
#define WAKIE 0x40
#define ERRIE 0x20
#define TX2IE 0x10
#define TX1IE 0x08
#define TX0IE 0x04
#define RX1IE 0x02
#define RX0IE 0x01

#define CNTL_RXFUL 0x80
#define TXREQ 0x08
#define DCL_RNR 0x40
#define SIDL_EXIDE 0x08

#define TX_CON_TXPRI0   0x01
#define TX_CON_TXPRI1   0x02
#define TX_CON_TXREQ    0x08
#define TX_CON_TXERR    0x10
#define TX_CON_TXLARB   0x20
#define TX_CON_TXABT    0x40
#define TX_CON_TXBIF    0x80

/*
 * CAN Modes of Opperation
 */
#define MODE_MASK     0xe0

#define NORMAL_MODE   0x00
#define SLEEP_MODE    0x20
#define LOOPBACK_MODE 0x40
#define LISTEN_MODE   0x60
#define CONFIG_MODE   0x80


#define SIDL_SRTR   0x10
#define SIDL_EXIDE  0x08

#define DCL_ERTR    0x40

typedef struct
{
    u8 ctrl;
    u8 sidh;
    u8 sidl;
    u8 eid8;
    u8 eid0;
    u8 dcl;
    u8 data[8];
} canBuffer_t;

#ifdef L2_CAN_INTERRUPT_DRIVEN

#else
//typedef struct can_buffer
//{
//    u8   *control;
//    u8   *sidh;
//    u8   *sidl;
//    u8   *eidh;
//    u8   *eidl;
//    u8   *dcl;
//    u8   *data;
//} can_buffer;


#endif

typedef struct can_mask
{
    u8   *sidh;
    u8   *sidl;
    u8   *eidh;
    u8   *eidl;
} can_mask;


//#define CAN_SJW     3
//#define CAN_BRP     1
//#define CAN_PHSEG1  4
//#define CAN_PHSEG2  4
//#define CAN_PROPSEG 7

/*
 * from above:
 *     TQ = 2/16MHz = 0.125 uS
 *     NBT = 16 TQ = 2uS
 *     500,000 baud
 *
 *  When BRP = 3
 *     TQ = 2*3/16 = 0.375 uS
 *     NBT = 16 TQ = 6uS
 *     166,666 baud
 */

#endif
