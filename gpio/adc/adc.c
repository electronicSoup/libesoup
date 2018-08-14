/**
 * @file libesoup/gpio/adc/adc.c
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
__attribute__((unused)) static const char *TAG = "ADC";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

#ifndef SYS_ADC_MAX_CH
#error libesoup_config.h file should define number of required ADC Channels in build
#endif

#include "libesoup/errno.h"
#include "libesoup/gpio/adc/adc.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/timers/sw_timers.h"

#ifdef SYS_ADC_AVERAGE_SAMPLES
struct average_buffer {
	uint16_t    samples[SYS_ADC_AVERAGE_SAMPLES];
	uint8_t     head;
};
#endif // SYS_ADC_AVERAGE_SAMPLES

struct adc_channel {
	enum gpio_pin    pin;
	uint16_t         last_reported;
	uint16_t         required_delta;
	uint16_t         sample;
	adc_handler_t    handler;
#ifdef SYS_ADC_AVERAGE_SAMPLES
	struct average_buffer samples;
#endif // SYS_ADC_AVERAGE_SAMPLES
};

struct   adc_channel channels[SYS_ADC_MAX_CH + 1];
uint8_t  adc_active_count = 0;

static timer_id adc_timer = BAD_TIMER_ID;
static struct timer_req timer_req;

#if defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _AD1Interrupt(void)
{
	uint8_t   loop;
	uint16_t *ptr;
#ifdef SYS_ADC_AVERAGE_SAMPLES
	uint8_t   i;
	uint32_t  total;
	uint8_t   count;
#endif // SYS_ADC_AVERAGE_SAMPLES

	IFS0bits.AD1IF   = 0;    // Clear the ISR Flag
	IEC0bits.AD1IE   = 0;    // Disable the ISR	
	AD1CON1bits.ADON = 0;  // Turn off the ADC

	ptr = (uint16_t *)&ADC1BUF0;
	for (loop = 0; loop < (SYS_ADC_MAX_CH + 1); loop++) {
		if (channels[loop].pin != INVALID_GPIO_PIN) {
#ifdef SYS_ADC_AVERAGE_SAMPLES
			channels[loop].samples.samples[channels[loop].samples.head] = *ptr++;
			channels[loop].samples.head++;
			channels[loop].samples.head %= SYS_ADC_AVERAGE_SAMPLES;
			total = 0;
			count = 0;
			for(i = 0; i < SYS_ADC_AVERAGE_SAMPLES; i++) {
				if (channels[loop].samples.samples[i] != 0xffff) {
					total += channels[loop].samples.samples[i];
					count++;
				}
			}
			channels[loop].sample = total / count;
#else
			channels[loop].sample = *ptr++;
#endif // SYS_ADC_AVERAGE_SAMPLES
		}
	}
}
#endif // (__dsPIC33EP256MU806__)

void adc_scan(timer_id timer, union sigval data)
{
	if (adc_active_count > 0) {
		IFS0bits.AD1IF   = 0;    // Clear the ISR Flag
		IEC0bits.AD1IE   = 1;    // Disable the ISR	
		AD1CON1bits.ADON = 1;    // Turn off the ADC
	}
}

static result_t scan_enable_ch(enum adc_pin adc_pin)
{
	/*
	 * Enable ADC pin in the list to sample.
	 */
	switch (adc_pin) {
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

static result_t scan_disable_ch(enum adc_pin adc_pin)
{
	/*
	 * Enable ADC pin in the list to sample.
	 */
	switch(adc_pin) {
	case AN0:
		AD1CSSLbits.CSS0 = 0;
		break;
	case AN1:
		AD1CSSLbits.CSS1 = 0;
		break;
	case AN2:
		AD1CSSLbits.CSS2 = 0;
		break;
	case AN3:
		AD1CSSLbits.CSS3 = 0;
		break;
	case AN4:
		AD1CSSLbits.CSS4 = 0;
		break;
	case AN5:
		AD1CSSLbits.CSS5 = 0;
		break;
	case AN6:
		AD1CSSLbits.CSS6 = 0;
		break;
	case AN7:
		AD1CSSLbits.CSS7 = 0;
		break;
	case AN8:
		AD1CSSLbits.CSS8 = 0;
		break;
	case AN9:
		AD1CSSLbits.CSS9 = 0;
		break;
	case AN10:
		AD1CSSLbits.CSS10 = 0;
		break;
	case AN11:
		AD1CSSLbits.CSS11 = 0;
		break;
	case AN12:
		AD1CSSLbits.CSS12 = 0;
		break;
	case AN13:
		AD1CSSLbits.CSS13 = 0;
		break;
	case AN14:
		AD1CSSLbits.CSS14 = 0;
		break;
	case AN15:
		AD1CSSLbits.CSS15 = 0;
		break;
	default:
		return(ERR_GENERAL_ERROR);
	}

	return(0);
}

result_t adc_init(void)
{
	uint16_t loop;
#ifdef SYS_ADC_AVERAGE_SAMPLES
	uint8_t  i;
#endif // SYS_ADC_AVERAGE_SAMPLES

	adc_active_count = 0;

	for(loop = 0; loop < (SYS_ADC_MAX_CH + 1); loop++) {
		channels[loop].pin = INVALID_GPIO_PIN;
		channels[loop].last_reported = 0;
		channels[loop].required_delta = 0;
		channels[loop].sample = 0;
		channels[loop].handler = NULL;
#ifdef SYS_ADC_AVERAGE_SAMPLES
		for (i = 0; i < SYS_ADC_AVERAGE_SAMPLES; i++) {
			channels[loop].samples.samples[i] = 0xffff;
		}
		channels[loop].samples.head = 0;
#endif // SYS_ADC_AVERAGE_SAMPLES
	}
	
	AD1CON1bits.ADON = DISABLED;     // Turn off for the moment

	/*
	 * Clear all ADC Channel from Scan
	 */
	AD1CSSL = 0x0000;
	AD1CSSH = 0x0000;
	
	/*
	 * Configure ADC SFR registers
	 */
	AD1CON1bits.AD12B   = ENABLED;   // 12 Bit mode
	AD1CON1bits.ADDMABM = DISABLED;  // 
	AD1CON1bits.FORM    = 0b00;      // Integer result
	AD1CON1bits.ASAM    = 0b1;       // Auto Sample
	AD1CON1bits.SSRCG   = 0b0;
	AD1CON1bits.SSRC    = 0b111;     // Auto Convert
	AD1CON1bits.SIMSAM  = 0;

	AD1CON2bits.VCFG    = 0b000;     // Avdd / Avss
	AD1CON2bits.CSCNA   = 0b1;       // Scan inputs
	AD1CON2bits.CHPS    = 0b00;      // Convert CH0
	AD1CON2bits.BUFM    = 0b0;
	AD1CON2bits.ALTS    = 0b0;
	
	// ADC CLK
	AD1CON3bits.ADRC    = 0b1;       // Use Internal RC for timing
	AD1CON3bits.SAMC    = 0b11111;   // 8 TAD
	
	// ADC DMA
	AD1CON4bits.ADDMAEN = 0b0;       // Don't use DMA
	
	/*
	 * \Initialise a timer request structure
	 */
	timer_req.period.units    = SYS_ADC_PERIOD_UNITS;
	timer_req.period.duration = SYS_ADC_PERIOD_DURATION;
	timer_req.type            = repeat;
	timer_req.exp_fn          = adc_scan;
	
	return(0);
}

result_t adc_tasks(void)
{
	uint16_t loop;
	
	for (loop = 0; loop < (SYS_ADC_MAX_CH + 1); loop++) {
		if ((channels[loop].pin != INVALID_GPIO_PIN)
		  && (channels[loop].last_reported != channels[loop].sample)) {
			if (channels[loop].sample > channels[loop].last_reported) {
				if (channels[loop].last_reported + channels[loop].required_delta < channels[loop].sample) {
					channels[loop].last_reported = channels[loop].sample;
					if (channels[loop].handler) {
						channels[loop].handler(channels[loop].pin, channels[loop].last_reported);
					} else {
						LOG_E("Ch Error\n\r");
					}
				}
			} else {
				if (channels[loop].last_reported - channels[loop].required_delta > channels[loop].sample) {
					channels[loop].last_reported = channels[loop].sample;
					if (channels[loop].handler) {
						channels[loop].handler(channels[loop].pin, channels[loop].last_reported);
					} else {
						LOG_E("Ch Error\n\r");
					}
				}
			}
		}
	}

	return(0);
}

result_t adc_monitor_channel(enum gpio_pin gpio_pin, uint16_t delta, adc_handler_t handler)
{
	result_t     rc;
	enum adc_pin adc_pin;

	/*
	 * Check the input gpio pin
	 */
	adc_pin = get_adc_from_gpio(gpio_pin);
	
	if(adc_pin == INVALID_ADC_PIN) {
		return(ERR_BAD_INPUT_PARAMETER);
	}

	/*
	 * Test if the ADC Channel already in use
	 */
	if (channels[adc_pin].pin != INVALID_GPIO_PIN) {
		return(ERR_BUSY);
	}

	/*
	 * Disable the ADC for the moment
	 */
	AD1CON1bits.ADON = DISABLED;

	channels[adc_pin].pin = gpio_pin;
	channels[adc_pin].last_reported = 0;
	channels[adc_pin].required_delta = delta;
	channels[adc_pin].handler = handler;
	channels[adc_pin].sample = 0;

	rc = gpio_set(gpio_pin, GPIO_MODE_ANALOG_INPUT, 0);
	if (rc < 0)
		return(rc);

	rc = scan_enable_ch(adc_pin);
	RC_CHECK

	adc_active_count++;
	if (adc_active_count > (SYS_ADC_MAX_CH + 1))
		return(ERR_GENERAL_ERROR);

	AD1CON2bits.SMPI = adc_active_count - 1;
	
	if (adc_timer == BAD_TIMER_ID) {
		rc = sw_timer_start(&timer_req);
		RC_CHECK

		adc_timer = (timer_id)rc;
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
