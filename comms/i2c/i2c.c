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

#ifdef SYS_I2C

void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C3Interrupt(void)
{
	
}

void i2c_init(void)
{
	LOG_D("i2c_init()\n\r");

	I2C3BRG           = 255;
	I2C3CONbits.IPMIEN = 0; // Disable IPMI mode
	I2C3CONbits.I2CEN = 1;
}

result_t i2c_send(uint8_t data)
{
	LOG_D("i2c_send()\n\r");

	while (!I2C3STATbits.P) {
		Nop();
	}
	LOG_D("Idle!\n\r");
	I2C3CONbits.SEN = 1;
	I2C3TRN         = data;

	return(SUCCESS);
}

#endif // _SYS_I2C
