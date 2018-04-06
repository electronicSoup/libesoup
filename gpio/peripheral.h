/**
 *
 * @file libesoup/gpio/peripheral.h
 *
 * @author John Whitmore
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

/**
 * @brief Peripheral Input functions for the dsPIC33EP256MU806
 */
#if defined(__dsPIC33EP256MU806__)
  #define PPS_I_EX_INT_1      RPINR0bits.INT1      ///< External Interrupt 1
  #define PPS_I_EX_INT_2      RPINR1bits.INT2      ///< External Interrupt 2
  #define PPS_I_EX_INT_3      RPINR1bits.INT3      ///< External Interrupt 3
  #define PPS_I_EX_INT_4      RPINR2bits.INT4      ///< External Interrupt 4
  #define PPS_I_T2CK          RPINR3bits.T2CK      ///< Timer 2 clock input source
  #define PPS_I_T3CK          RPINR3bits.T3CK      ///< Timer 3 clock input source
  #define PPS_I_T4CK          RPINR4bits.T4CK      ///< Timer 4 clock input source
  #define PPS_I_T5CK          RPINR4bits.T5CK      ///< Timer 5 clock input source
  #define PPS_I_T6CK          RPINR5bits.T6CK      ///< Timer 6 clock input source
  #define PPS_I_T7CK          RPINR5bits.T7CK      ///< Timer 7 clock input source
  #define PPS_I_T8CK          RPINR6bits.T8CK      ///< Timer 8 clock input source
  #define PPS_I_T9CK          RPINR6bits.T9CK      ///< Timer 9 clock input source
  #define PPS_I_IC1           RPINR7bits.IC1       ///< Input Capture 1
  #define PPS_I_IC2           RPINR7bits.IC2       ///< Input Capture 2
  #define PPS_I_IC3           RPINR8bits.IC3       ///< Input Capture 3
  #define PPS_I_IC4           RPINR8bits.IC4       ///< Input Capture 4
  #define PPS_I_IC5           RPINR9bits.IC5       ///< Input Capture 5
  #define PPS_I_IC6           RPINR9bits.IC6       ///< Input Capture 6
  #define PPS_I_IC7           RPINR10bits.IC7      ///< Input Capture 7
  #define PPS_I_IC8           RPINR10bits.IC8      ///< Input Capture 8
  #define PPS_I_OCFA          RPINR11bits.OCFA     ///< Output Compare Fault A
  #define PPS_I_OCFB          RPINR11bits.OCFB     ///< Output Compare Fault B
  #define PPS_I_FLT1          RPINR12bits.FLT1     ///< PMW Fault 1
  #define PPS_I_FLT2          RPINR12bits.FLT2     ///< PMW Fault 2
  #define PPS_I_FLT3          RPINR13bits.FLT3     ///< PMW Fault 3
  #define PPS_I_FLT4          RPINR13bits.FLT4     ///< PMW Fault 4
  #define PPS_I_QEA1          RPINR14bits.QEA1     ///< QEI1 Phase A
  #define PPS_I_QEB1          RPINR14bits.QEB1     ///< QEI1 Phase B
  #define PPS_I_INDX1         RPINR15bits.INDX1    ///< QEI1 Index
  #define PPS_I_HOME1         RPINR15bits.HOME1    ///< QEI1 Home
  #define PPS_I_QEA2          RPINR16bits.QEA2     ///< QEI2 Phase A
  #define PPS_I_QEB2          RPINR16bits.QEB2     ///< QEI2 Phase B
  #define PPS_I_INDX2         RPINR17bits.INDX2    ///< QEI2 Index
  #define PPS_I_HOME2         RPINR17bits.HOME2    ///< QEI2 Home
  #define PPS_I_UART_1_RX     RPINR18bits.U1RXR    ///< UART 1 Recieve
  #define PPS_I_U1CTS         RPINR18bits.U1CTS    ///< UART 1 Clear To Send
  #define PPS_I_UART_2_RX     RPINR19bits.U2RXR    ///< UART 2 Recieve
  #define PPS_I_U2CTS         RPINR19bits.U2CTS    ///< UART 2 Clear To Send
  #define PPS_I_SPI_1_DI      RPINR20bits.SDI1R    ///< SPI 1 Data Input
  #define PPS_I_SPI_1_CLK     RPINR20bits.SCK1R    ///< SPI 1 Clock
  #define PPS_I_SPI_1_SS      RPINR21bits.SS1R     ///< SPI 1 Slave Select
  #define PPS_I_SPI_2_SS      RPINR23bits.SS2R     ///< SPI 2 Slave Select
  #define PPS_I_CSDI          RPINR24bits.CSDIR    ///< DCI Data Input
  #define PPS_I_CSCKIN        RPINR24bits.CSCKIN   ///< DCI Clock Input
  #define PPS_I_COFSIN        RPINR25bits.COFSR    ///< DCI FSYNC Input
  #define PPS_I_CAN1_RX       RPINR26bits.C1RXR    ///< CAN 1 Recieve
  #define PPS_I_CAN2_RX       RPINR26bits.C2RXR    ///< CAN 2 Recieve
  #define PPS_I_UART_3_RX     RPINR27bits.U3RXR    ///< UART 3 Recieve
  #define PPS_I_UART_3_CTS    RPINR27bits.U3CTSR   ///< UART 3 CTS
  #define PPS_I_UART_4_RX     RPINR28bits.U4RXR    ///< UART 4 Recieve
  #define PPS_I_UART_4_CTS    RPINR28bits.U4CTSR   ///< UART 4 Clear To Send
  #define PPS_I_SPI_3_DI      RPINR29bits.SDI3R    ///< SPI 3 Data Input
  #define PPS_I_SPI_3_CLK     RPINR29bits.SCK3R    ///< SPI 3 Clock Input
  #define PPS_I_SPI_3_SS      RPINR30bits.SS3R     ///< SPI 3 Slave Select Input
  #define PPS_I_SPI_4_DI      RPINR31bits.SDI4R    ///< SPI 4 Data Input
  #define PPS_I_SPI_4_CLK     RPINR31bits.SCK4R    ///< SPI 4 Clock Input
  #define PPS_I_SPI_4_SS      RPINR32bits.SS4R     ///< SPI 4 Slave Select Input
  #define PPS_I_IC9           RPINR33bits.IC9R     ///< Input Capture 9
  #define PPS_I_IC10          RPINR33bits.IC10R    ///< Input Capture 10
  #define PPS_I_IC11          RPINR34bits.IC11R    ///< Input Capture 11
  #define PPS_I_IC12          RPINR34bits.IC12R    ///< Input Capture 12
  #define PPS_I_IC13          RPINR35bits.IC13R    ///< Input Capture 13
  #define PPS_I_IC14          RPINR35bits.IC14R    ///< Input Capture 14
  #define PPS_I_IC15          RPINR36bits.IC15R    ///< Input Capture 15
  #define PPS_I_IC16          RPINR36bits.IC16R    ///< Input Capture 16
  #define PPS_I_OCFC          RPINR37bits.OCFCR    ///< Output Compare Fault C
  #define PPS_I_SYNCI1        RPINR37bits.SYNCI1R  ///< PWM Synch Input 1
  #define PPS_I_DTCMP1        RPINR38bits.DTCMP1R  ///< PWM Dead-Time Compensation 1
  #define PPS_I_SYNCI2        RPINR38bits.SYNCI2R  ///< PWM Synch Input 2
  #define PPS_I_DTCMP2        RPINR39bits.DTCMP2R  ///< PWM Dead-Time Compensation 2
  #define PPS_I_DTCMP3        RPINR39bits.DTCMP3R  ///< PWM Dead-Time Compensation 3
  #define PPS_I_DTCMP4        RPINR40bits.DTCMP4R  ///< PWM Dead-Time Compensation 4
  #define PPS_I_DTCMP5        RPINR40bits.DTCMP5R  ///< PWM Dead-Time Compensation 5
  #define PPS_I_DTCMP6        RPINR41bits.DTCMP6R  ///< PWM Dead-Time Compensation 6
  #define PPS_I_DTCMP7        RPINR41bits.DTCMP7R  ///< PWM Dead-Time Compensation 7
  #define PPS_I_FLT5          RPINR42bits.FLT5R    ///< PWM Fault 5
  #define PPS_I_FLT6          RPINR42bits.FLT6R    ///< PWM Fault 6
  #define PPS_I_FLT7          RPINR43bits.FLT7R    ///< PWM Fault 7
#endif

/**
 * @brief Peripheral Input functions for the PIC24FJ256GB106
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
  #define PPS_I_EX_INT_1      RPINR0bits.INT1      ///< External Interrupt 1
  #define PPS_I_EX_INT_2      RPINR1bits.INT2      ///< External Interrupt 2
  #define PPS_I_EX_INT_3      RPINR1bits.INT3      ///< External Interrupt 3
  #define PPS_I_EX_INT_4      RPINR2bits.INT4      ///< External Interrupt 4
  #define PPS_I_T2CK          RPINR3bits.T2CK      ///< Timer2 External Clock
  #define PPS_I_T3CK          RPINR3bits.T3CK      ///< Timer3 External Clock
  #define PPS_I_T4CK          RPINR4bits.T4CK      ///< Timer4 External Clock
  #define PPS_I_T5CK          RPINR4bits.T5CK      ///< Timer5 External Clock
  #define PPS_I_IC1           RPINR7bits.IC1       ///< Input Capture 1
  #define PPS_I_IC2           RPINR7bits.IC2       ///< Input Capture 2
  #define PPS_I_IC3           RPINR8bits.IC3       ///< Input Capture 3
  #define PPS_I_IC4           RPINR8bits.IC4       ///< Input Capture 4
  #define PPS_I_IC5           RPINR9bits.IC5       ///< Input Capture 5
  #define PPS_I_IC6           RPINR9bits.IC6       ///< Input Capture 6
  #define PPS_I_IC7           RPINR10bits.IC7      ///< Input Capture 7
  #define PPS_I_IC8           RPINR10bits.IC8      ///< Input Capture 8
  #define PPS_I_OCFA          RPINR11bits.OCFAR    ///< Output Compare Fault A
  #define PPS_I_OCFB          RPINR11bits.OCFBR    ///< Output Compare Fault B
  #define PPS_I_IC9           RPINR15bits.IC9      ///< Input Capture 9
  #define PPS_I_UART_3_RX     RPINR17bits.U3RXR    ///< UART 3 Recieve
  #define PPS_I_UART_1_RX     RPINR18bits.U1RXR    ///< UART 1 Recieve
  #define PPS_I_U1CTS         RPINR18bits.U1CTS    ///< UART 1 Clear To Send
  #define PPS_I_UART_2_RX     RPINR19bits.U2RXR    ///< UART 1 Recieve
  #define PPS_I_U2CTS         RPINR19bits.U2CTS    ///< UART 2 Clear To Send
  #define PPS_I_SPI_1_CLK     RPINR20bits.SCK1R    ///< SPI 1 Clock Input
  #define PPS_I_SPI_1_DI      RPINR20bits.SDI1R    ///< SPI 1 Data Input
  #define PPS_I_SPI_1_SS      RPINR21bits.SS1R     ///< SPI 1 Slave Select Input
  #define PPS_I_SPI_2_DI      RPINR22bits.SDI2R    ///< SPI 2 Data Input
  #define PPS_I_SPI_2_CLK     RPINR22bits.SCK2R    ///< SPI 2 Clock Input
  #define PPS_I_SPI_2_SS      RPINR23bits.SS2R     ///< SPI 2 Slave Select Input
  #define PPS_I_SPI_3_CLK     RPINR23bits.SCK3R    ///< SPI 3 Clock Input
  #define PPS_I_UART_4_RX     RPINR27bits.U4RXR    ///< UART 3 Recieve
  #define PPS_I_UART_4_CTS    RPINR27bits.U4CTS    ///< UART 3 Clear To Send
  #define PPS_I_SPI_3_DI      RPINR28bits.SDI3R    ///< SPI 3 Data Input
  #define PPS_I_SPI_3_SS      RPINR29bits.SS3R     ///< SPI 3 Slave Select Input
#endif


/**
 * @brief Peripheral Output functions for the dsPIC33EP256MU806
 */
#if defined(__dsPIC33EP256MU806__)
  #define PPS_O_DEFAULRT_PORT             0x00     ///< Defauly Pin
  #define PPS_O_UART_1_TX                 0x01     ///< UART 1 Transmit
  #define PPS_O_U1RTS                     0x02     ///< UART 1 Request To Send
  #define PPS_O_UART_2_TX                 0x03     ///< UART 2 Transmit
  #define PPS_O_U2RTS                     0x04     ///< UART 2 Request To Send
  #define PPS_O_SPI1DO                    0x05     ///< SPI 1 Data Output
  #define PPS_O_SPI1CLK                   0x06     ///< SPI 1 Clock Output
  #define PPS_O_SPI1SS                    0x07     ///< SPI 1 Slave Select Output
  #define PPS_O_SPI2SS                    0x0A     ///< SPI 2 Slave Select Output
  #define PPS_D_DCIDO                     0x0B     ///< DCI Data Output
  #define PPS_D_DCICLK                    0x0C     ///< DCI Clock Output
  #define PPS_D_DCIFSYNC                  0x0D     ///< DCI FSYNC Output
  #define PPS_O_CAN1_TX                   0x0E     ///< CAN 1 Transmit
  #define PPS_O_CAN2_TX                   0x0F     ///< CAN 2 Transmit
  #define PPS_O_OC1                       0x10     ///< Output Compare 1 Output
  #define PPS_O_OC2                       0x11     ///< Output Compare 2 Output
  #define PPS_O_OC3                       0x12     ///< Output Compare 3 Output
  #define PPS_O_OC4                       0x13     ///< Output Compare 4 Output
  #define PPS_O_OC5                       0x14     ///< Output Compare 5 Output
  #define PPS_O_OC6                       0x15     ///< Output Compare 6 Output
  #define PPS_O_OC7                       0x16     ///< Output Compare 7 Output
  #define PPS_O_OC8                       0x17     ///< Output Compare 8 Output
  #define PPS_O_C1OUT                     0x18     ///< Comparator Output 1
  #define PPS_O_C2OUT                     0x19     ///< Comparator Output 2
  #define PPS_O_C3OUT                     0x1A     ///< Comparator Output 3
  #define PPS_O_UART_3_TX                 0x1B     ///< UART 3 Transmit
  #define PPS_O_U3RTS                     0x1C     ///< UART 3 Request To Send
  #define PPS_O_UART_4_TX                 0x1D     ///< UART 4 Transmit
  #define PPS_O_U4RTS                     0x1E     ///< UART 4 Request To Send
  #define PPS_O_SPI3DO                    0x1F     ///< SPI 3 Data Output
  #define PPS_O_SPI3CLK                   0x20     ///< SPI 3 Clock Output
  #define PPS_O_SPI3SS                    0x21     ///< SPI 3 Slave Select Output
  #define PPS_O_SPI4DO                    0x22     ///< SPI 4 Data Output
  #define PPS_O_SPI4CLK                   0x23     ///< SPI 4 Clock Output
  #define PPS_O_SPI4SS                    0x24     ///< SPI 4 Slave Select
  #define PPS_O_OC9                       0x25     ///< Output Compare 9 Output
  #define PPS_O_OC10                      0x26     ///< Output Compare 10 Output
  #define PPS_O_OC11                      0x27     ///< Output Compare 11 Output
  #define PPS_O_OC12                      0x28     ///< Output Compare 12 Output
  #define PPS_O_OC13                      0x29     ///< Output Compare 13 Output
  #define PPS_O_OC14                      0x2A     ///< Output Compare 14 Output
  #define PPS_O_OC15                      0x2B     ///< Output Compare 15 Output
  #define PPS_O_OC16                      0x2C     ///< Output Compare 16 Output
  #define PPS_O_SYNCO1                    0x2D     ///< PWM Primary Time Base Sync Output
  #define PPS_O_SYNCO2                    0x2E     ///< PWM Secondary Time Base Sync Output
  #define PPS_O_QEI1CCMP                  0x2F     ///< QEI 1 Counter Comparator Output
  #define PPS_O_QEI2CCMP                  0x30     ///< QEI 2 Counter Comparator Output
  #define PPS_O_REFCLK                    0x31     ///< Reference Clock Output
#endif

/**
 * @brief Peripheral Output functions for the PIC24FJ256GB106
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
  #define PPS_O_C1OUT                     1        ///< Comparator 1 Output
  #define PPS_O_C2OUT                     2        ///< Comparator 2 Output
  #define PPS_O_UART_1_TX                 3        ///< UART 1 Transmit
  #define PPS_O_U1RTS                     4        ///< UART 1 Request To Send
  #define PPS_O_UART_2_TX                 5        ///< UART 2 Transmit
  #define PPS_O_U2RTS                     6        ///< UART 2 Request To Send
  #define PPS_O_SPI1DO                    7        ///< SPI 1 Data Output
  #define PPS_O_SPI1CLK                   8        ///< SPI 1 Clock Output
  #define PPS_O_SPI1SS                    9        ///< SPI 1 Slave Select Output
  #define PPS_D_SPI2DO                    10       ///< SPI 2 Data Output
  #define PPS_D_SPI2CLK                   11       ///< SPI 2 Clock Output 
  #define PPS_O_SPI2SS                    12       ///< SPI 2 Slave Slect Output
  #define PPS_O_OC1                       18       ///< Output Compare 1
  #define PPS_O_OC2                       19       ///< Output Compare 2
  #define PPS_O_OC3                       20       ///< Output Compare 3
  #define PPS_O_OC4                       21       ///< Output Compare 4
  #define PPS_O_OC5                       22       ///< Output Compare 5
  #define PPS_O_OC6                       23       ///< Output Compare 6
  #define PPS_O_OC7                       24       ///< Output Compare 7
  #define PPS_O_OC8                       25       ///< Output Compare 8
  #define PPS_O_UART_3_TX                 28       ///< UART 3 Transmit 
  #define PPS_O_U3RTS                     29       ///< UART 3 Request To Send
  #define PPS_O_UART_4_TX                 30       ///< UART 4 Transmit
  #define PPS_O_U4RTS                     31       ///< UART 4 Request To Send
  #define PPS_O_SPI3DO                    32       ///< SPI 3 Data Output
  #define PPS_O_SPI3CLK                   33       ///< SPI 3 Clock Output
  #define PPS_O_SPI3SS                    34       ///< SPI 3 Slave Select
  #define PPS_O_OC9                       35       ///< Output Compare 9
  #define PPS_O_C3OUT                     36       ///< Comparater 3 Output
#endif

/**
 * @brief Set an input pin to a peripheral function
 * @param pin GPIO pin to configure
 * @return result Pin number for assignement to function (Negative on error)
 *
 * For example to set UART 1 Recieve to be on pin RD0 use code:
 *
 * PPS_I_UART_1_RX = set_peripheral_input(RD0);
 *
 * The above line ignores any returned error from the function call so more 
 * correct implementation would check for error before assignment:
 * 
 * rc = set_peripheral_input(RD0);
 * if (rc < 0) {
 *     // Error
 * } else {
 *     PPS_I_UART_1_RX = rc;
 * }
 */
extern result_t set_peripheral_input(enum pin_t pin);

/**
 * @brief Set an Output pin to a peripheral function
 * @param pin - GPIO pin to configure
 * @param function - Function to be configured
 * @return result Negative on error.
 *
 * For example to set UART 1 Transmit to be on pin RD0 use code:
 * rc = set_peripheral_output(RD0, PPS_O_UART_1_TX);
 * if (rc < 0) {
 *     // Error
 * }
 */
extern result_t set_peripheral_output(enum pin_t pin, uint16_t function);

#endif // _PERIPHERAL_H
