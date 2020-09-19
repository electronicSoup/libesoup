/**
 * @file libesoup/comms/i2c/devices/slave_24LCxx.c
 *
 * @author John Whitmore
 *
 * @brief  Have to do this ISR driven. Need for Speed.
 *
 * Copyright 2020 electronicSoup Limited
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
 *******************************************************************************
 *
 */
#include "libesoup_config.h"

#if defined(SYS_SLV_24LCxx) && defined(__dsPIC33EP128GS702__)

#define DEBUG_FILE
#define TAG "I2C_EP128"

#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/i2c/test/Spin_Fv-1_progs.h"

/*
 * Prototype
 */
//static void i2c3_send_next(enum i2c_channel);

#define SDA1_PIN RB7
#define SCL1_PIN RB6
#if 0
#define I2C1_STARTED         I2C1STATbits.S
#define I2C1_STOPPED         I2C1STATbits.P
#define I2C1_RBF             I2C1STATbits.RBF
#define I2C1_TRANSMITTING    I2C1STATbits.TBF
#define I2C1_WRITE_COLLISION I2C1STATbits.IWCOL
#define I2C1_ACKSTAT         I2C1STATbits.ACKSTAT
#define I2C1_READ            I2C1STATbits.R_W

#define I2C1_START           I2C1CONLbits.SEN   = 1
#define I2C1_STOP            I2C1CONLbits.PEN   = 1
#define I2C1_RESTART         I2C1CONLbits.RSEN  = 1
//#define I2C1_READ            I2C1CONLbits.RCEN  = 1
#define I2C1_RX_ACK          I2C1CONLbits.ACKDT = 0; I2C1CONLbits.ACKEN =1
#define I2C1_RX_NACK         I2C1CONLbits.ACKDT = 1; I2C1CONLbits.ACKEN =1

static uint8_t  *program = LLRR;
static uint16_t tx_index = 0;
static uint16_t rx_count = 0;
static uint16_t address  = 0;
#endif
void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C1Interrupt(void)
{
	serial_printf("*M*\n\r");
}

void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C1Interrupt(void)
{
	uint8_t  rx_byte;

	serial_printf("*S* ");
	while (IFS1bits.SI2C1IF) {    // || I2C1STATbits.S) {
//	while (IFS1bits.SI2C1IF || I2C1STATbits.S) {
		IFS1bits.SI2C1IF    = 0;
		serial_printf(".");
		if (I2C1STATbits.P) {
			serial_printf("!");
			TRISBbits.TRISB7 = 1;
#if 0
			if (program == LLRR) {
				program = LRRL;
			} else {
				program = LLRR;
			}
			tx_index = 0;
			rx_count = 0;
			address  = 0;
#endif
		} else if (I2C1STATbits.S) {
			if(I2C1STATbits.RBF) {
//				serial_printf("R%d", rx_count);
				serial_printf("R");
				rx_byte = I2C1RCV;
#if 0
				if(rx_count == 0) {
					serial_printf("-0x%x", rx_byte);
					rx_count++;
				} else if(rx_count == 1) {
					serial_printf("-0x%x", rx_byte);
					address = rx_byte;
					address = address << 8;
					rx_count++;
				} else if (rx_count == 2) {
					serial_printf("-0x%x", rx_byte);
					address |= rx_byte;
					rx_count++;
					serial_printf(" A0x%x", address);
				} else {
					rx_count++;
					serial_printf("D");
				}
#endif
			}
#if 1
			if (I2C1STATbits.R_W) {
				serial_printf("T");
				TRISBbits.TRISB7 = 0;
//				while(I2C1_READ && I2C1_STARTED) {
//					if(I2C1_STARTED) {
						while(I2C1STATbits.TBF);
						serial_printf("\n\r");
						I2C1TRN = 0x55; //*program++;
//						serial_printf("+");
						I2C1STATbits.TBF = 1;
//					}
//				}
			}
#endif

		}

		if (I2C1STATbits.IWCOL) {
			serial_printf("V");
		}
	}
	serial_printf("\n\r");
}

void __attribute__((__interrupt__, __no_auto_psv__)) _I2C1BCInterrupt(void)
{
	LOG_D("B1* ")
}

result_t slave_24lcxx_init(void)
{
	gpio_set(SCL1_PIN, GPIO_MODE_DIGITAL_INPUT, 0);
	gpio_set(SDA1_PIN, GPIO_MODE_DIGITAL_INPUT, 0);
	I2C1BRG              = 0x0d;

	I2C1CONLbits.I2CSIDL = 0;  // Module continues in Idle.
	I2C1CONLbits.SCLREL  = 1;  // Release clock (Slave mode))
	I2C1CONLbits.A10M    = 0;  // 7 bit mode
	I2C1CONLbits.DISSLW  = 1;  // Disable Slew rate.
	I2C1CONLbits.SMEN    = 0;  // Disable SMBus thresholds
	I2C1CONLbits.GCEN    = 1;  // Enable General call address
	I2C1CONLbits.STREN   = 0;  // Disable clock stretching
	I2C1CONLbits.ACKDT   = 0;  // Send /ACK to acknowledge

	I2C1CONHbits.SCIE    = 1;  // Enable Start ISR
	I2C1CONHbits.PCIE    = 1;  // Enable Stop ISR
	I2C1MSK           = 0xff;  // Don't care
	I2C1ADD           = 0xa0;

	IFS10bits.I2C1BCIF   = 0;
	IFS1bits.MI2C1IF     = 0;  // Clear Master ISR Flag
	IFS1bits.SI2C1IF     = 0;  // Clear Slave ISR Flag
	IEC1bits.MI2C1IE     = 1;  // Enable Master Interrupts
	IEC1bits.SI2C1IE     = 1;  // Enable Slave Interrupts

	I2C1CONLbits.I2CEN   = 1;
//		I2C1CONLbits.PEN     = 1;  // Send Stop

//	program = LLRR;

        return (SUCCESS);
}

#endif // SYS_SLV_24LC64