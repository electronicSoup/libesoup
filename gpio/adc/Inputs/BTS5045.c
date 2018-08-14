/**
 * @file libesoup/gpio/adc/Inputs/BTS5045.c
 *
 * @author John Whitmore
 * 
 * @brief ADC Handler for the Infineon BTS5045 Profet with a 665 Ohm resistor
 * on the Is pin.
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
#include "libesoup_config.h"

#ifdef SYS_ADC

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
__attribute__((unused)) static const char *TAG = "BTS5045";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

#include "libesoup/gpio/gpio.h"

void adc_bts5045_665r(enum gpio_pin pin, uint16_t value)
{
	uint32_t  current_ma;
	
	switch(pin) {
	case RB0:
		current_ma = (value * 100) / 57;
		LOG_D("ADC Value 0x%x current %dmA\n\r", value, (uint16_t)current_ma);
		break;
		
	default:
		break;
	}
}

#endif // SYS_ADC
