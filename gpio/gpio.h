/**
 *
 * \file libesoup/gpio/gpio.c
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

#define OUTPUT_PIN  0
#define INPUT_PIN   1

#define DIGITAL_PIN 0
#define ANALOG_PIN  1

#define GPIO_MODE_DIGITAL_INPUT     0x01
#define GPIO_MODE_DIGITAL_OUTPUT    0x02
#define GPIO_MODE_ANALOG_INPUT      0x04
#define GPIO_MODE_ANALOG_OUTPUT     0x08
#define GPIO_MODE_OPENDRAIN_INPUT   0x10
#define GPIO_MODE_OPENDRAIN_OUTPUT  0x20
#define GPIO_MODE_PULLUP            0x40
#define GPIO_MODE_PULLDOWN          0x80

extern result_t gpio_set(enum pin_t pin, uint16_t mode, uint8_t value);
extern result_t gpio_get(enum pin_t pin);
extern result_t pin_to_port_bit(enum pin_t pin, uint8_t **prt, uint8_t *bt);
