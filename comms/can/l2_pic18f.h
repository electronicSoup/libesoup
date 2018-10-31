#ifndef L2_PIC18F_H
#define L2_PIC18F_H

#include "libesoup_config.h"
#include <stdint.h>

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
 * CAN Modes of Operation
 */
#define MODE_MASK 0xe0

typedef enum { 
    normal      = 0b000,
    disabled    = 0b001,
    loopback    = 0b010,
    listen_only = 0b011,
    config      = 0b100,
} ty_can_mode;


#define SIDL_SRTR   0x10
#define SIDL_EXIDE  0x08

#define DCL_ERTR    0x40

typedef struct
{
    uint8_t ctrl;
    uint8_t sidh;
    uint8_t sidl;
    uint8_t eid8;
    uint8_t eid0;
    uint8_t dcl;
    uint8_t data[8];
} canBuffer_t;

#ifdef L2_CAN_INTERRUPT_DRIVEN

#else
//typedef struct can_buffer
//{
//    uint8_t   *control;
//    uint8_t   *sidh;
//    uint8_t   *sidl;
//    uint8_t   *eidh;
//    uint8_t   *eidl;
//    uint8_t   *dcl;
//    uint8_t   *data;
//} can_buffer;


#endif

typedef struct can_mask
{
    uint8_t   *sidh;
    uint8_t   *sidl;
    uint8_t   *eidh;
    uint8_t   *eidl;
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
