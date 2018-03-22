/**
 *
 * \file libesoup/gpio/peripheral.h
 *
 * Copyright 2018 electronicSoup Limited
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
#ifndef _PERIPHERAL_H
#define _PERIPHERAL_H

/*
 * Peripheral Input functions
 */
#define PPS_I_EX_INT_1                    RPINR0bits.INT1
#define PPS_I_EX_INT_2                    RPINR1bits.INT2
#define PPS_I_EX_INT_3                    RPINR1bits.INT3
#define PPS_I_I_EX_INT_4                    RPINR2bits.INT4
#define PPS_I_T2CK                        RPINR3bits.T2CK
#define PPS_I_T3CK                        RPINR3bits.T3CK
#define PPS_I_T4CK                        RPINR4bits.T4CK
#define PPS_I_T5CK                        RPINR4bits.T5CK
#define PPS_I_T6CK                        RPINR5bits.T6CK
#define PPS_I_T7CK                        RPINR5bits.T7CK
#define PPS_I_T8CK                        RPINR6bits.T8CK
#define PPS_I_T9CK                        RPINR6bits.T9CK
#define PPS_I_I_IC1                         RPINR7bits.IC1
#define PPS_I_IC2                         RPINR7bits.IC2
#define PPS_I_IC3                         RPINR8bits.IC3
#define PPS_I_IC4                         RPINR8bits.IC4
#define PPS_I_IC5                         RPINR9bits.IC5
#define PPS_I_IC6                         RPINR9bits.IC6
#define PPS_I_IC7                         RPINR10bits.IC7
#define PPS_I_IC8                         RPINR10bits.IC8
#define PPS_I_OCFA                        RPINR11bits.OCFA
#define PPS_I_OCFB                        RPINR11bits.OCFB
#define PPS_I_FLT1                        RPINR12bits.FLT1
#define PPS_I_FLT2                        RPINR12bits.FLT2
#define PPS_I_FLT3                        RPINR13bits.FLT3
#define PPS_I_FLT4                        RPINR13bits.FLT4
#define PPS_I_QEA1                        RPINR14bits.QEA1
#define PPS_I_QEB1                        RPINR14bits.QEB1
#define PPS_I_INDX1                       RPINR15bits.INDX1
#define PPS_I_HOME1                       RPINR15bits.HOME1
#define PPS_I_QEA2                        RPINR16bits.QEA2
#define PPS_I_QEB2                        RPINR16bits.QEB2
#define PPS_I_INDX2                       RPINR17bits.INDX2
#define PPS_I_HOME2                       RPINR17bits.HOME2
#define PPS_I_UART_1_RX                   RPINR18bits.U1RXR
#define PPS_I_U1CTS                       RPINR18bits.U1CTS
#define PPS_I_UART_2_RX                   RPINR19bits.U2RXR
#define PPS_I_U2CTS                       RPINR19bits.U2CTS
#define PPS_I_SPI_1_DI                    RPINR20bits.SDI1R 
#define PPS_I_SPI_1_CLK                   RPINR20bits.SCK1R
#define PPS_I_SPI_1_SS                    RPINR21bits.SS1R
#define PPS_I_SPI_2_SS                    RPINR23bits.SS2R
#define PPS_I_CSDI                        RPINR24bits.CSDIR
#define PPS_I_CSCKIN                      RPINR24bits.CSCKIN
#define PPS_I_COFSIN                      RPINR25bits.COFSR
#define PPS_I_CAN1_RX                     RPINR26bits.C1RXR
#define PPS_I_CAN2_RX                     RPINR26bits.C2RXR
#define PPS_I_UART_3_RX                   RPINR27bits.U3RXR
#define PPS_I_UART_3_CTS                  RPINR27bits.U3CTSR
#define PPS_I_UART_4_RX                   RPINR28bits.U4RXR
#define PPS_I_UART_4_CTS                  RPINR28bits.U4CTSR
#define PPS_I_SPI3DI                      RPINR29bits.SDI3R 
#define PPS_I_SPI3CLK                     RPINR29bits.SCK3R
#define PPS_I_SPI3SS                      RPINR30bits.SS3R
#define PPS_I_SPI4DI                      RPINR31bits.SDI4R 
#define PPS_I_SPI4CLK                     RPINR31bits.SCK4R
#define PPS_I_SPI4SS                      RPINR32bits.SS4R
#define PPS_I_IC9                         RPINR33bits.IC9R
#define PPS_I_IC10                        RPINR33bits.IC10R
#define PPS_I_IC11                        RPINR34bits.IC11R
#define PPS_I_IC12                        RPINR34bits.IC12R
#define PPS_I_IC13                        RPINR35bits.IC13R
#define PPS_I_IC14                        RPINR35bits.IC14R
#define PPS_I_IC15                        RPINR36bits.IC15R
#define PPS_I_IC16                        RPINR36bits.IC16R
#define PPS_I_OCFC                        RPINR37bits.OCFCR
#define PPS_I_SYNCI1                      RPINR37bits.SYNCI1R
#define PPS_I_DTCMP1                      RPINR38bits.DTCMP1R
#define PPS_I_SYNCI2                      RPINR38bits.SYNCI2R
#define PPS_I_DTCMP2                      RPINR39bits.DTCMP2R
#define PPS_I_DTCMP3                      RPINR39bits.DTCMP3R
#define PPS_I_DTCMP4                      RPINR40bits.DTCMP4R
#define PPS_I_DTCMP5                      RPINR40bits.DTCMP5R
#define PPS_I_DTCMP6                      RPINR41bits.DTCMP6R
#define PPS_I_DTCMP7                      RPINR41bits.DTCMP7R
#define PPS_I_FLT5                        RPINR42bits.FLT5R
#define PPS_I_FLT6                        RPINR42bits.FLT6R
#define PPS_I_FLT7                        RPINR43bits.FLT7R

/*
 * Peripheral Output functions
 */
#define PPS_O_DEFAULRT_PORT             0x00
#define PPS_O_UART_1_TX                 0x01
#define PPS_O_U1RTS                     0x02
#define PPS_O_UART_2_TX                 0x03
#define PPS_O_U2RTS                     0x04
#define PPS_O_SPI1DO                    0x05
#define PPS_O_SPI1CLK                   0x06
#define PPS_O_SPI1SS                    0x07
#define PPS_O_SPI2SS                    0x0A
#define PPS_D_DCIDO                     0x0B
#define PPS_D_DCICLK                    0x0C
#define PPS_D_DCIFSYNC                  0x0D
#define PPS_O_CAN1_TX                   0x0E
#define PPS_O_CAN2_TX                   0x0F
#define PPS_O_OC1                       0x10
#define PPS_O_OC2                       0x11
#define PPS_O_OC3                       0x12
#define PPS_O_OC4                       0x13
#define PPS_O_OC5                       0x14
#define PPS_O_OC6                       0x15
#define PPS_O_OC7                       0x16
#define PPS_O_OC8                       0x17
#define PPS_O_C1OUT                     0x18
#define PPS_O_C2OUT                     0x19
#define PPS_O_C3OUT                     0x1A
#define PPS_O_UART_3_TX                 0x1B
#define PPS_O_U3RTS                     0x1C
#define PPS_O_UART_4_TX                 0x1D
#define PPS_O_U4RTS                     0x1E
#define PPS_O_SPI3DO                    0x1F
#define PPS_O_SPI3CLK                   0x20
#define PPS_O_SPI3SS                    0x21
#define PPS_O_SPI4DO                    0x22
#define PPS_O_SPI4CLK                   0x23
#define PPS_O_SPI4SS                    0x24
#define PPS_O_OC9                       0x25
#define PPS_O_OC10                      0x26
#define PPS_O_OC11                      0x27
#define PPS_O_OC12                      0x28
#define PPS_O_OC13                      0x29
#define PPS_O_OC14                      0x2A
#define PPS_O_OC15                      0x2B
#define PPS_O_OC16                      0x2C
#define PPS_O_SYNCO1                    0x2D
#define PPS_O_SYNCO2                    0x2E
#define PPS_O_QEI1CCMP                  0x2F
#define PPS_O_QEI2CCMP                  0x30
#define PPS_O_REFCLK                    0x31

//#define PPS_RP64                        RPOR0bits.RP64R
//#define PPS_RP100                       RPOR9bits.RP100R
//#define PPS_RP118                       RPOR13bits.RP118R
//#define PPS_RP120                       RPOR14bits.RP120R
    

extern int16_t set_peripheral_input(enum pin_t);
extern int16_t set_peripheral_output(enum pin_t, uint16_t function);

#endif // _PERIPHERAL_H
