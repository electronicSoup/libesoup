/**
 *
 * \file libesoup/can/l2_mcp2515.h
 *
 * CAN L2 Definitions for MCP2515 Chip
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU Lesser General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef L2_MCP2515_H
#define L2_MCP2515_H

/*
 *  CAN Chip Registers
 */
#define BFPCTRL     0x0c
#define TXRTSCTRL   0x0d
#define CANCTRL     0x0f
#define CANSTAT     0x0e
#define TEC         0x1c
#define REC         0x1d
#define RXM0SIDH    0x20
#define RXM1SIDH    0x24
#define CNF3_REG    0x28
#define CNF2_REG    0x29
#define CNF1_REG    0x2a
#define CANINTE     0x2b
#define CANINTF     0x2c
#define EFLG        0x2d
#define TXB0CTRL    0x30
#define TXB0SIDH    0x31
#define TXB1CTRL    0x40
#define TXB1SIDH    0x41
#define TXB2CTRL    0x50
#define TXB2SIDH    0x51
#define RXB0CTRL    0x60
#define RXB0SIDH    0x61
#define RXB1CTRL    0x70
#define RXB1SIDH    0x71

#define SIDL_SRTR   0x10
#define SIDL_EXIDE  0x08

#define DCL_ERTR    0x40


#define IOCD        0x0e
#define ERR         0x02
#define WAK         0x04
#define TX0         0x06
#define TX1         0x08
#define TX2         0x0a
#define RX0         0x0c
#define RX1         0x0e

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

/*
 * Error Flags
 */
#define RX1OVR 0x80
#define RX0OVR 0x40
#define TXBO   0x20
#define TXEP   0x10
#define RXEP   0x08
#define TXWAR  0x04
#define RXWAR  0x02
#define EWARN  0x01

/*
 * TXRTSCTRL Register Mask
 */
#define B2RTS   0x20
#define B1RTS   0x10
#define B0RTS   0x08
#define B2RTSM  0x04
#define B1RTSM  0x02
#define B0RTSM  0x01

/*
 * BFPCTRL Register Mask
 */
#define B1BFS   0x20
#define B0BFS   0x10
#define B1BFE   0x08
#define B0BFE   0x04
#define B1BFM   0x02
#define B0BFM   0x01

/*
 * TXBnCTRL Register Mask
 */
#define ABTF  0x40
#define MLOA  0x20
#define TXERR 0x10
#define TXREQ 0x08
#define TXP1  0x02
#define TXP0  0x01
/*
 *  CNF1 Register Mask
 */
#define SJW_MASK  0xc0
#define BRP_MASK  0x3f

/*
 *  CNF2 Register Mask
 */
#define BTLMODE_MASK  0x80
#define SAM_MASK      0x40
#define PSEG1_MASK    0x38
#define PROPSEG_MASK  0x07

/*
 *  CNF3 Register Mask
 */
#define SOF_MASK     0x80
#define WAKFIL_MASK  0x40
#define PSEG2_MASK   0x07

/*
 * CANCTRL One Shot Mode
 */
#define OSM 0x08

/*
 * CAN Modes of Opperation
 */
#define MODE_MASK     0xe0

#define NORMAL_MODE   0x00
#define SLEEP_MODE    0x20
#define LOOPBACK_MODE 0x40
#define LISTEN_MODE   0x60
#define CONFIG_MODE   0x80

#define SJW        3
#define BRP        1
#define PHASE_SEG1 4
#define PHASE_SEG2 4
#define PROP_SEG   7

/*
 * MCP2515 SPI Commands
 */
#define CAN_RESET           0xc0
#define CAN_READ_REG        0x03
#define CAN_READ_RX_BUFFER  0x90    // Buffer number -> Lower Nibble
#define CAN_WRITE_REG       0x02
#define CAN_LOAD_TX_BUFFER  0x40    // Buffer number -> Lower Nibble
#define CAN_RTS             0x80    // Buffer number -> Lower Nibble
#define CAN_READ_STATUS     0xa0
#define CAN_RX_STATUS       0xb0
#define CAN_BIT_MODIFY      0x05

#endif // L2_MCP2515_H
