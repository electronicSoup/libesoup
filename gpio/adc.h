/**
 * @file libesoup/gpio/adc.c
 *
 * @author John Whitmore
 * 
 * @brief API for ADC functions
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
#ifndef _ADC_H
#define _ADC_H

#include "libesoup_config.h"

#ifdef SYS_ADC
typedef void (*adc_handler_t)(enum gpio_pin, uint16_t);

extern result_t adc_monitor_channel(enum gpio_pin pin, uint16_t delta);
extern result_t adc_sample(enum gpio_pin pin, adc_handler_t);

#endif

#endif // _ADC_H
