/**
 * @file libesoup/gpio/pwm.h
 *
 * @author John Whitmore
 * 
 * @brief API for PWM functions
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
#ifndef _PWM_H
#define _PWM_H

#include "libesoup_config.h"

/*
 * Must be called before the pin is turned on with the pwm_on() API call.
 * but can be called after the pwm_on() API call to change the current
 * configuration of frequency or duty cycle.
 */
extern result_t pwm_config(enum gpio_pin, uint16_t frequency, uint8_t duty);
extern result_t pwm_on(enum gpio_pin);
extern result_t pwm_off(enum gpio_pin);

#endif // _PWM_H
