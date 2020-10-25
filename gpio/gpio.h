/**
 * @file libesoup/gpio/gpio.h
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
#include "libesoup_config.h"

#define GPIO_OUTPUT_PIN  0   ///< value used to set a GPIO pin as an Output pin
#define GPIO_INPUT_PIN   1   ///< value used to set a GPIO pin as an Output pin

#define GPIO_DIGITAL_PIN 0   ///< value used to configure a GPIO pin to be a Digital pin
#define GPIO_ANALOG_PIN  1   ///< value used to configure a GPIO pin to be an Analog pin

/**
 * @brief Modes of operation for GPIO Pins
 */
#define GPIO_MODE_DIGITAL_INPUT     0x01  ///< Digital input pin Mode
#define GPIO_MODE_DIGITAL_OUTPUT    0x02  ///< Digital output pin Mode
#define GPIO_MODE_ANALOG_INPUT      0x04  ///< Analog input pin Mode
#define GPIO_MODE_ANALOG_OUTPUT     0x08  ///< Analog output pin Mode
#define GPIO_MODE_OPENDRAIN         0x10  ///< Open Drain Input pin Mode
#define GPIO_MODE_PULLUP            0x20  ///< Enable Pull up resistor on pin Mode
#define GPIO_MODE_PULLDOWN          0x40  ///< Enable pull down resistor on pin Mode

/**
 * @brief Set the mode of a GPIO Pin
 * @param pin The GPIO pin to be configured
 * @param mode The required mode of operation
 * @param value The initial value of the pin, if an output pin
 * @return result_t result (Negative on error)
 */
extern result_t gpio_set(enum gpio_pin pin, uint16_t mode, uint8_t value);

/**
 * @brief Toggle output of GPIO Pin
 * @param pin The GPIO pin to be toggled
 * @return result_t result (Negative on error)
 */
extern result_t gpio_toggle_output(enum gpio_pin pin);

/**
 * @brief Read the current value on a GPIO pin
 * @param pin The pin whose value is to be read.
 * @return result_t The value read or negative on an error
 */
extern result_t gpio_get(enum gpio_pin pin);
