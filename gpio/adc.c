/**
 * @file libesoup/gpio/adc.c
 *
 * @author John Whitmore
 * 
 * @brief functions for working with ADC inputs
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
#if defined(__XC16)
__attribute__((unused)) static const char *TAG = "ADC";
#elif defined(__XC8)
static const char *TAG = "SPI";
#endif
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

#ifndef SYS_ADC_CHANNELS
#error libesoup_config.h file should define number of required ADC Channels in build
#endif

#include "libesoup/errno.h"
#include "libesoup/gpio/adc.h"
#include "libesoup/gpio/gpio.h"

struct adc_channel {
	enum gpio_pin    pin;
	uint16_t      last_reported;
	uint16_t      required_delta;
	uint16_t      sample;
	adc_handler_t handler;
};

struct adc_channel channels[SYS_ADC_CHANNELS];

#if defined(__dsPIC33EP256MU806__)
#endif

#if defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _AD1Interrupt(void)
{
	IFS0bits.AD1IF   = 0;    // Clear the ISR Flag
	IEC0bits.AD1IE   = 0;    // Disable the ISR	
	AD1CON1bits.ADON = 0;  // Turn off the ADC
	
//	if(sample_handler) sample_handler(RB0, ADC1BUF0);
	
//	sample_handler = NULL;
}
#endif // (__dsPIC33EP256MU806__)

result_t adc_init(void)
{
	uint16_t loop;
	
	for(loop = 0; loop < SYS_ADC_CHANNELS; loop++) {
		channels[loop].pin = INVALID_GPIO_PIN; 
		channels[loop].last_reported = 0;
		channels[loop].required_delta = 0;
		channels[loop].sample = 0;
		channels[loop].handler = NULL;
	}

	/*
	 * Clear all ADC Channel from Scan
	 */
	AD1CSSL = 0x0000;

	return(0);
}

result_t adc_monitor_channel(enum gpio_pin gpio_pin, uint16_t delta)
{
	uint16_t loop;
	enum adc_pin adc_pin;

	/*
	 * Check the input gpio pin
	 */
	adc_pin = get_adc_from_gpio(gpio_pin);
	
	if(adc_pin == INVALID_ADC_PIN) {
		return(ERR_BAD_INPUT_PARAMETER);
	}

	/*
	 * Find a free slot in the channels array
	 */
	for(loop = 0; loop < SYS_ADC_CHANNELS; loop++) {
		if (channels[loop].pin == INVALID_GPIO_PIN) {
			channels[loop].pin = gpio_pin;
			channels[loop].last_reported = 0;
			channels[loop].required_delta = delta;
			channels[loop].sample = 0;
		}
	}

	if (loop >= SYS_ADC_CHANNELS) {
		return(ERR_NO_RESOURCES);
	}

	/*
	 * Enable ADC pin in the list to sample.
	 */
	switch(adc_pin) {
	case AN0:
		AD1CSSLbits.CSS0 = 1;
		break;
	case AN1:
		AD1CSSLbits.CSS1 = 1;
		break;
	case AN2:
		AD1CSSLbits.CSS2 = 1;
		break;
	case AN3:
		AD1CSSLbits.CSS3 = 1;
		break;
	case AN4:
		AD1CSSLbits.CSS4 = 1;
		break;
	case AN5:
		AD1CSSLbits.CSS5 = 1;
		break;
	case AN6:
		AD1CSSLbits.CSS6 = 1;
		break;
	case AN7:
		AD1CSSLbits.CSS7 = 1;
		break;
	case AN8:
		AD1CSSLbits.CSS8 = 1;
		break;
	case AN9:
		AD1CSSLbits.CSS9 = 1;
		break;
	case AN10:
		AD1CSSLbits.CSS10 = 1;
		break;
	case AN11:
		AD1CSSLbits.CSS11 = 1;
		break;
	case AN12:
		AD1CSSLbits.CSS12 = 1;
		break;
	case AN13:
		AD1CSSLbits.CSS13 = 1;
		break;
	case AN14:
		AD1CSSLbits.CSS14 = 1;
		break;
	case AN15:
		AD1CSSLbits.CSS15 = 1;
		break;
	default:
		return(ERR_GENERAL_ERROR);
	}
	
	return(0);
}

#if defined(__dsPIC33EP256MU806__)
result_t adc_sample(enum gpio_pin pin, adc_handler_t handler)
{
	result_t  rc;
	
	if(pin != RB0) return(-ERR_NOT_CODED);
	
//	sample_handler = handler;
	
	rc = gpio_set(pin, GPIO_MODE_ANALOG_INPUT, 0);
	RC_CHECK
		
	AD1CSSL = 0x00;
	AD1CSSH = 0x00;
	AD1CSSLbits.CSS0 = 1;
	
	AD1CON1bits.AD12B = 0;    // 10 Bit Operation
	AD1CON1bits.FORM = 0b00;  // Output Integer result
	AD1CON1bits.SSRCG = 0;
	AD1CON1bits.SSRC = 0b111; // Internal counter ends sample starts conversion
	AD1CON1bits.ASAM = 0;     // Don't auto sample
	
	AD1CON2bits.VCFG = 0b100;  // Avdd and Avss are reference voltages
	AD1CON2bits.CHPS = 0b00;   // Sample only channel 0
	
	AD1CON3bits.ADRC = 1;      // Internal RC Clock
	
	AD1CON4bits.ADDMAEN = 0;   // Don't use DMA
	
	AD1CHS0bits.CH0NA = 0;     // Negative input is Vrefl
	AD1CHS0bits.CH0SA = 0;     // Positive input is AN0
	AD1CHS0bits.CH0NB = 0;     // Negative input is Vrefl
	
	AD1CON1bits.ADON = 1;     // Turn on the ADC
	AD1CON1bits.DONE = 0;     // Clear the Done bit
	AD1CON1bits.SAMP = 1;     // Set Sample bit to start sample

	IFS0bits.AD1IF = 0;    // Clear the ISR Flag
	IEC0bits.AD1IE = 1;    // Enable the ISR

	return(0);
}
#endif  // (__dsPIC33EP256MU806__)

#endif // SYS_ADC
