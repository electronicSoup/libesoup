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
#define BIT_BANG

#if 0

#include "libesoup_config.h"

#if defined(SYS_SLV_24LCxx) && defined(__dsPIC33EP128GS702__)

#define DEBUG_FILE
#define TAG "24LCxx"

#include "libesoup/logger/serial_log.h"

#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/i2c/test/Spin_Fv-1_progs.h"

/*
 * Prototype
 */
//static void i2c3_send_next(enum i2c_channel);

#define SDA1_PIN RB7
#define SCL1_PIN RB6
#define SDA2_PIN RB14
#define SCL2_PIN RB15

#define I2C1
//#define I2C2

static uint8_t *program;
static uint8_t  program_select;
static uint16_t tx_count = 0;

#ifdef I2C1
void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C1Interrupt(void)
{
	serial_printf("*M1*\n\r");
}

void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C1Interrupt(void)
{
	uint8_t  rx_byte;

	while (IFS1bits.SI2C1IF) {
		IFS1bits.SI2C1IF    = 0;
		if (I2C1STATbits.P) {
			program        = LLRR;
			tx_count       = 0;
			program_select = 0;
			serial_printf("!");
		} else if (I2C1STATbits.S) {
			if (!I2C1STATbits.TBF && tx_count < 512) {
				tx_count++;
				I2C1TRN = *program++;
			}
			if(I2C1STATbits.RBF) {
				rx_byte = I2C1RCV;
			}
			if (I2C1STATbits.R_W) {
				while (tx_count < 512) {
					if (!I2C1STATbits.TBF) {
						tx_count++;
						I2C1TRN = *program++;
					}
				}
				serial_printf("E");
			}
		}

		if (I2C1STATbits.IWCOL) {
			serial_printf("W");
		}
		if (I2C1STATbits.I2COV) {
			serial_printf("V");
		}
	}
}

void __attribute__((__interrupt__, __no_auto_psv__)) _I2C1BCInterrupt(void)
{
	LOG_D("B1* ")
}
#elif defined(I2C2)
void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C2Interrupt(void)
{
	serial_printf("*M2*\n\r");
}

void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C2Interrupt(void)
{
	uint8_t  rx_byte;

	while (IFS3bits.SI2C2IF) {
		IFS3bits.SI2C2IF    = 0;
		if (I2C2STATbits.P) {
			serial_printf("!");
		} else if (I2C2STATbits.S) {
			if(I2C2STATbits.RBF) {
				rx_byte = I2C2RCV;
			}
			if (I2C2STATbits.R_W) {
				TRISBbits.TRISB14 = 0;    // Change SDA1 to Output
				while(I2C2STATbits.TBF);
				I2C2TRN = 0x55;
				I2C2STATbits.TBF = 1;
			}
		}

		if (I2C2STATbits.IWCOL) {
			serial_printf("W");
		}
		if (I2C2STATbits.I2COV) {
			serial_printf("V");
		}
	}
}

void __attribute__((__interrupt__, __no_auto_psv__)) _I2C2BCInterrupt(void)
{
	LOG_D("B2* ")
}
#endif

result_t slave_24lcxx_init(void)
{
	serial_printf("init()\n\r");

	program = LLRR;
	program_select = 0;
	/*
	 * SDA1 and SCL1 Inputs with OpenDrain enabled
	 */
#if defined(I2C1)
	ANSELBbits.ANSB6 = 0;
	TRISBbits.TRISB6 = 1;
	ODCBbits.ODCB6   = 1;
	ANSELBbits.ANSB7 = 0;
	TRISBbits.TRISB7 = 1;
	ODCBbits.ODCB7   = 1;

	I2C1CONLbits.I2CSIDL = 0;  // Module continues in Idle.
	I2C1CONLbits.SCLREL  = 1;  // Release clock (Slave mode))
	I2C1CONLbits.A10M    = 0;  // 7 bit mode
	I2C1CONLbits.DISSLW  = 1;  // Disable Slew rate.
	I2C1CONLbits.SMEN    = 0;  // Disable SMBus thresholds
	I2C1CONLbits.GCEN    = 1;  // Enable General call address
	I2C1CONLbits.STREN   = 0;  // Disable clock stretching

	I2C1CONHbits.SCIE    = 1;  // Enable Start ISR
	I2C1CONHbits.PCIE    = 1;  // Enable Stop ISR

	I2C1MSK           = 0x00;  // Don't care about address for moment
	I2C1ADD           = 0x50;

	IFS1bits.MI2C1IF     = 0;  // Clear Master ISR Flag
	IEC1bits.MI2C1IE     = 1;  // Enable Master Interrupts

	IFS1bits.SI2C1IF     = 0;  // Clear Slave ISR Flag
	IEC1bits.SI2C1IE     = 1;  // Enable Slave Interrupts

	IFS10bits.I2C1BCIF   = 0;
	IEC10bits.I2C1BCIE   = 1;

	I2C1CONLbits.I2CEN   = 1;
#elif defined(I2C2)
	TRISBbits.TRISB14 = 1;
	ODCBbits.ODCB14   = 1;
	TRISBbits.TRISB15 = 1;
	ODCBbits.ODCB15   = 1;

	I2C2CONLbits.I2CSIDL = 0;  // Module continues in Idle.
	I2C2CONLbits.SCLREL  = 1;  // Release clock (Slave mode))
	I2C2CONLbits.A10M    = 0;  // 7 bit mode
	I2C2CONLbits.DISSLW  = 1;  // Disable Slew rate.
	I2C2CONLbits.SMEN    = 0;  // Disable SMBus thresholds
	I2C2CONLbits.GCEN    = 1;  // Enable General call address
	I2C2CONLbits.STREN   = 0;  // Disable clock stretching

	I2C2CONHbits.SCIE    = 1;  // Enable Start ISR
	I2C2CONHbits.PCIE    = 1;  // Enable Stop ISR

	I2C2MSK           = 0xff;  // Don't care about address for moment
	I2C2ADD           = 0xa0;

	IFS3bits.MI2C2IF     = 0;  // Clear Master ISR Flag
	IEC3bits.MI2C2IE     = 1;  // Enable Master Interrupts

	IFS3bits.SI2C2IF     = 0;  // Clear Slave ISR Flag
	IEC3bits.SI2C2IE     = 1;  // Enable Slave Interrupts

	IFS10bits.I2C2BCIF   = 0;
	IEC10bits.I2C2BCIE   = 1;

	I2C2CONLbits.I2CEN   = 1;
#endif
        return (SUCCESS);
}

#endif // SYS_SLV_24LC64

#endif //0

#ifdef BIT_BANG
#include "libesoup_config.h"
#include "libesoup/gpio/gpio.h"

#define DEBUG_FILE
#define TAG "24LCxx"

#include "libesoup/logger/serial_log.h"

#define SDA PORTBbits.RB7
#define SCL PORTBbits.RB6

#define SDA_PIN RB7
#define SCL_PIN RB6

result_t slave_24lcxx_init(void)
{
	uint8_t  i;
	uint8_t  rx_byte;
	result_t rc;

	rc = gpio_set(SCL_PIN, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK;
	rc = gpio_set(SDA_PIN, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK;

	while (SDA && SCL);

	if (!SDA && SCL) {
		/*
		 * STARTED
		 * read First byte
		 */
		rx_byte = 0x00;

		for (i = 0; i < 8; i++) {
			while(SCL);
			while(!SCL);
			rx_byte = (rx_byte << 1) | SDA;
		}

		/*
		 * Ack that byte
		 */
		while(SCL);
		gpio_set(SDA_PIN, GPIO_MODE_DIGITAL_OUTPUT, 0);
		while(!SCL);
		while(SCL);
		gpio_set(SDA_PIN, GPIO_MODE_DIGITAL_INPUT, 0);

		return(rx_byte);

		/*
		 * Read byte
		 */
		if (rx_byte && 0x01) {
			
		}

	} else {
		return(-ERR_IM_A_TEAPOT);
	}

}
#endif // BIT_BANG

#ifdef MINIMAL
#include <xc.h>

#define SDA1_PIN RB7
#define SCL1_PIN RB6

void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C1Interrupt(void)
{
//	serial_printf("*M1*\n\r");
}

void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C1Interrupt(void)
{
	uint8_t  rx_byte;

	while (IFS1bits.SI2C1IF) {
		IFS1bits.SI2C1IF    = 0;
		if (I2C1STATbits.P) {
//			serial_printf("!");
		} else if (I2C1STATbits.S) {
			if(I2C1STATbits.RBF) {
				rx_byte = I2C1RCV;
			}
			if (I2C1STATbits.R_W) {
				while (I2C1STATbits.TBF);
				I2C1TRN = 0x55;
			}
		}

		if (I2C1STATbits.IWCOL) {
//			serial_printf("W");
		}
		if (I2C1STATbits.I2COV) {
//			serial_printf("V");
		}
	}
}

void __attribute__((__interrupt__, __no_auto_psv__)) _I2C1BCInterrupt(void)
{
//	serial_printf("B1* ");
}

void slave_24lcxx_init(void)
{
	/*
	 * SDA1 and SCL1 Inputs with OpenDrain enabled
	 */
	ANSELBbits.ANSB6 = 0;
	TRISBbits.TRISB6 = 1;
	ODCBbits.ODCB6   = 1;
	ANSELBbits.ANSB7 = 0;
	TRISBbits.TRISB7 = 1;
	ODCBbits.ODCB7   = 1;

	I2C1CONLbits.I2CSIDL = 0;  // Module continues in Idle.
	I2C1CONLbits.SCLREL  = 1;  // Release clock (Slave mode))
	I2C1CONLbits.A10M    = 0;  // 7 bit mode
	I2C1CONLbits.DISSLW  = 1;  // Disable Slew rate.
	I2C1CONLbits.SMEN    = 0;  // Disable SMBus thresholds
	I2C1CONLbits.GCEN    = 1;  // Enable General call address
	I2C1CONLbits.STREN   = 0;  // Disable clock stretching

	I2C1CONHbits.SCIE    = 1;  // Enable Start ISR
	I2C1CONHbits.PCIE    = 1;  // Enable Stop ISR

	I2C1MSK           = 0x00;  // Don't care about address for moment
	I2C1ADD           = 0x50;

	IFS1bits.MI2C1IF     = 0;  // Clear Master ISR Flag
	IEC1bits.MI2C1IE     = 1;  // Enable Master Interrupts

	IFS1bits.SI2C1IF     = 0;  // Clear Slave ISR Flag
	IEC1bits.SI2C1IE     = 1;  // Enable Slave Interrupts

	IFS10bits.I2C1BCIF   = 0;
	IEC10bits.I2C1BCIE   = 1;

	I2C1CONLbits.I2CEN   = 1;
}

int main(void)
{
	slave_24lcxx_init();

	while(1) {
		Nop();
	}
}
#endif
