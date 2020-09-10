/**
 * @file libesoup/comms/i2c/i2c.c
 *
 * @author John Whitmore
 * 
 * @brief  Implementaiton of the I2C protocol. Initial PIC24FJ256GB106
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

#define DEBUG_FILE
#define TAG "I2C"

#include "libesoup/logger/serial_log.h"

#if defined(SYS_I2C1) || defined(SYS_I2C2) || defined(SYS_I2C3)

void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C3Interrupt(void)
{
	static uint8_t txing = 0;
	uint8_t        state;

	serial_printf("*");
	while (IFS5bits.MI2C3IF) {
		serial_printf("+");
		IFS5bits.MI2C3IF    = 0;  // Clear Master ISR Flag
		state = I2C3CON & 0x1f;
		switch (state) {
		case 0x00:
			if (I2C3STATbits.TRSTAT) {
				serial_printf("X");
			} else {
				serial_printf("I");
				if (!txing) {
					I2C3TRN         = 0x55;
					txing = 1;
				} else {
					I2C3CONbits.PEN = 1;
					txing = 0;
				}
			}
			break;
		case 0x01:
			serial_printf("1");
			break;
		case 0x02:
			serial_printf("2");
			break;
		case 0x04:
			serial_printf("4");
			break;
		case 0x08:
			serial_printf("8");
			break;
		case 0x10:
			serial_printf("9");
			break;
		}
		serial_printf("\n\r");
	}
}

void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C3Interrupt(void)
{
	serial_printf("-");
}

void __attribute__((__interrupt__, __no_auto_psv__)) _BI2C3Interrupt(void)
{
	LOG_D("I2C3 Collision ISR\n\r");
}

result_t i2c_init(enum i2c_channel chan)
{
	LOG_D("i2c_init()\n\r");
        switch (chan) {
                case I2C1:
                        LOG_E("Not implemented\n\r");
                        return(-ERR_NOT_CODED);
                        break;
                case I2C2:
                        LOG_E("Not implemented\n\r");
                        return(-ERR_NOT_CODED);
                        break;
                case I2C3:
                        I2C3BRG             = 0x27;

                        I2C3CONbits.I2CSIDL = 0;  // Module continues in Idle.
                        I2C3CONbits.SCLREL  = 1;  // Release clock (Slave mode))
                        I2C3CONbits.IPMIEN  = 0;  // Disable IPMI mode
                        I2C3CONbits.A10M    = 0;  // 7 bit mode
                        I2C3CONbits.DISSLW  = 1;  // Disable Slew rate.
                        I2C3CONbits.SMEN    = 0;  // Disable SMBus thresholds
                        I2C3CONbits.GCEN    = 0;  // Disable General call address
                        I2C3CONbits.STREN   = 0;  // Disable clock stretching
                        I2C3CONbits.ACKDT   = 0;  // Send /ACK to acknowledge

                        IFS5bits.MI2C3IF    = 0;  // Clear Master ISR Flag
                        IFS5bits.SI2C3IF    = 0;  // Clear Slave ISR Flag
                        IEC5bits.MI2C3IE    = 1;  // Enable Master Interrupts
                        IEC5bits.SI2C3IE    = 1;  // Enable Slave Interrupts

                        I2C3CONbits.I2CEN   = 1;
                        break;
                default:
                        LOG_E("No such I2C\n\r");
                        return (-ERR_BAD_INPUT_PARAMETER);
                        break;
        }
        return (SUCCESS);
}

result_t i2c_send(uint8_t data)
{
	LOG_D("i2c_send()\n\r");

//	while (!I2C3STATbits.P) {
//		Nop();
//	}
//	LOG_D("Idle!\n\r");
	I2C3CONbits.SEN = 1;
//	I2C3TRN         = data;

	return(SUCCESS);
}

#endif // _SYS_I2C1 || SYS_I2S2 || SYS_I2S3
