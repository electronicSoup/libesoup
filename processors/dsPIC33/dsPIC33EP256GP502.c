/**
 * @file libesoup/processors/dsPIC/dsPIC33EP256GP502.c
 *
 * @author John Whitmore
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
 */
#if defined(__dsPIC33EP256GP502__)
#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "dsPIC33";
#endif // SYS_SERIAL_LOGGING
#include "libesoup/logger/serial_log.h"

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"

#if defined(SYS_UART1) || defined(SYS_UART2)
#include "libesoup/comms/uart/uart.h"
#endif

#if defined (SYS_HW_TIMERS)
#include "libesoup/timers/time.h"
#include "libesoup/timers/delay.h"
#endif

/*
 * Check required libesoup_config.h defines are found
 */
//#ifndef SYS_CLOCK_FREQ
//#error libesoup_config.h should define SYS_CLOCK_FREQ (see libesoup/examples/libesoup_config.h)
//#endif

/*
 * Set up the configuration words of the processor:
 *
 * file:///opt/microchip/xc16/v1.26/docs/config_docs/33EP256MU806.html
 */
#pragma config ICS     = NONE
#pragma config JTAGEN  = OFF
#pragma config FNOSC   = FRC
#pragma config ALTI2C1 = OFF
#pragma config ALTI2C2 = OFF
//#pragma config FNOSC   = PRI   // Primary oscillator
//#pragma config FNOSC = SOSC  // Secondary oscillator
#pragma config IESO    = OFF
#pragma config POSCMD  = HS
#pragma config IOL1WAY = OFF
#pragma config FCKSM   = CSECMD
#pragma config WDTPRE  = PR128  // 1:128
#pragma config WDTPOST = PS128  // 1:128
#pragma config WINDIS  = ON     // Watchdog Timer Window disabled
#pragma config FWDTEN  = OFF

static void clock_init(void);

/*
 * Interrupts
 */
void _ISR __attribute__((__no_auto_psv__)) _AddressError(void)
{
	LOG_E("Address error");
	while (1) {
	}
}

void _ISR __attribute__((__no_auto_psv__)) _StackError(void)
{
	LOG_E("Stack error");
	while (1)  {
	}
}

void cpu_init(void)
{
        clock_init();
	CLEAR_WDT

        INTCON2bits.GIE = ENABLED;
}

static void clock_init(void)
{
#if 1
        uint8_t  clock;
	uint8_t  n1;
	uint8_t  n2;
	uint16_t m;
#endif
        /*
         * There's a special case if the required clock frequency is 1/2 the
         * Crystal Frequency then we can simple use Primary Clock.
	 * NO PLL
         */

	/*
	 * N1 = 4
	 * n2 - 2
	 * m = ((60,000,000 � 2) � 4) � 7,370,000
	 * m = 65
	 */
	n1 = 4;
        m  = 64;
	n2 = 2;

	sys_clock_freq = ((BRD_CRYSTAL_FREQ / n1) * m) / n2;  // 59,881,250
//	sys_clock_freq = ((BRD_CRYSTAL_FREQ / 1) * 16) / 2;  // 59,881,250

	clock = dsPIC33_INTERNAL_RC_PLL;
	__builtin_write_OSCCONH(clock);

	CLKDIVbits.PLLPRE  = n1 - 2;
	CLKDIVbits.PLLPOST = 0b00;   // n2
	PLLFBDbits.PLLDIV  = m - 2;

        __builtin_write_OSCCONL(OSCCON | 0x01);

        // Wait for Clock switch to occur
        while (OSCCONbits.COSC!= clock);

	while (OSCCONbits.LOCK!= 1);
}

enum adc_pin get_adc_from_gpio(enum gpio_pin gpio_pin)
{
	enum adc_pin adc_pin;

	switch(gpio_pin) {
	case RA0:
		adc_pin = AN0;
		break;
	case RA1:
		adc_pin = AN1;
		break;
	case RB0:
		adc_pin = AN2;
		break;
	case RB1:
		adc_pin = AN3;
		break;
	case RB2:
		adc_pin = AN4;
		break;
	case RB3:
		adc_pin = AN5;
		break;
	default:
		adc_pin = INVALID_ADC_PIN;
		break;
	}

	return(adc_pin);
}

enum gpio_pin get_gpio_from_adc(enum adc_pin adc_pin)
{
	enum gpio_pin gpio_pin;

	switch(adc_pin) {
	case AN0:
		gpio_pin = RA0;
		break;
	case AN1:
		gpio_pin = RA1;
		break;
	case AN2:
		gpio_pin = RB0;
		break;
	case AN3:
		gpio_pin = RB1;
		break;
	case AN4:
		gpio_pin = RB2;
		break;
	case AN5:
		gpio_pin = RB3;
		break;
	default:
		gpio_pin = INVALID_ADC_PIN;
		break;
	}

	return(gpio_pin);
}

/*
 * dsPIC33EP256GP502 had no PWM channels
 */
enum pwm_pin get_pwm_from_gpio(enum gpio_pin gpio_pin)
{
	enum pwm_pin pwm_pin;

	switch (gpio_pin) {
	default:
		pwm_pin = INVALID_PWM_PIN;
		break;
	}

	return(pwm_pin);
}

enum gpio_pin get_gpio_from_pwm(enum pwm_pin pwm_pin)
{
	enum gpio_pin gpio_pin;

	switch(pwm_pin) {
	default:
		gpio_pin = INVALID_PWM_PIN;
		break;
	}

	return(gpio_pin);
}

int16_t set_peripheral_input(enum gpio_pin pin)
{
	int16_t  ppin;

	switch(pin) {
	case RA4:
		ppin = 20;
		break;
	case RB0:
		ppin = 32;
		break;
	case RB1:
		ppin = 33;
		break;
	case RB2:
		ppin = 34;
		break;
	case RB3:
		ppin = 35;
		break;
	case RB4:
		ppin = 36;
		break;
	case RB5:
		ppin = 37;
		break;
	case RB6:
		ppin = 38;
		break;
	case RB7:
		ppin = 39;
		break;
	case RB8:
		ppin = 40;
		break;
	case RB9:
		ppin = 41;
		break;
	case RB10:
		ppin = 42;
		break;
	case RB11:
		ppin = 43;
		break;
	case RB12:
		ppin = 44;
		break;
	case RB13:
		ppin = 45;
		break;
	case RB14:
		ppin = 46;
		break;
	case RB15:
		ppin = 47;
		break;
	default:
		ppin = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(ppin);
}

int16_t set_peripheral_output(enum gpio_pin pin, uint16_t function)
{
	result_t rc = 0;

	switch(pin) {
	case RA4:  // PR20
		RPOR0bits.RP20R = function;
		break;
	case RB3:  // 35
		RPOR0bits.RP35R = function;
		break;
	case RB4:  // 36
		RPOR1bits.RP36R = function;
		break;
	case RB5:  // 37
		RPOR1bits.RP37R = function;
		break;
	case RB6:  // 38
		RPOR2bits.RP38R = function;
		break;
	case RB7:  // 39
		RPOR2bits.RP39R = function;
		break;
	case RB8:  // 40
		RPOR3bits.RP40R = function;
		break;
	case RB9:  // 41
		RPOR3bits.RP41R = function;
		break;
	case RB10: // 42
		RPOR4bits.RP42R = function;
		break;
	case RB11: // 43
		RPOR4bits.RP43R = function;
		break;
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}

	return(rc);
}

#if defined(SYS_UART1) || defined(SYS_UART2)
result_t uart_set_uart_config(struct uart_data *udata)
{
	enum uart_channel channel;

        channel = udata->channel;
	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
		U1MODE = udata->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U1STAbits.UTXISEL1 = 1;
		U1STAbits.UTXISEL0 = 0;

		/*
		 * with BRGH = 0 Slow Speed Mode:
		 *        Baudrate - between  sys_clock_freq /(16 * 65536) and sys_clock_freq / 16
		 *                   For sys_clock_freq = 16,000,000 Min 15bps Max 1Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 3bps Max 250Kbps
		 *
		 *                   Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 *                   UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 *                   UxBRG = ((CLOCK / BAUD)/16) -1
		 *
		 *
		 * with BRGH = 1 : sys_clock_freq /(4 * 65536) <= Baudrate <= sys_clock_freq / 4
		 *                   For sys_clock_freq = 16,000,000 Min 61bps Max 4Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 15bps Max 1Mbps
		 */
		if(udata->uart_mode & BRGH_MASK) {
			/*
			 * Hight Speed Mode:
			 */
			U1BRG = (uint16_t)((uint32_t)((uint32_t)((uint32_t)sys_clock_freq / 2) / (uint32_t)udata->baud) / 4) - 1;
		} else {
			/*
			 * Standard Mode:
			 */
			U1BRG = (uint16_t)((uint32_t)((uint32_t)((uint32_t)sys_clock_freq / 2) / (uint32_t)udata->baud) / 16) - 1;
		}

		U1_RX_ISR_FLAG             = 0;
		U1_TX_ISR_FLAG             = 0;

		if (udata->rx_pin != INVALID_GPIO_PIN) {
                        U1STAbits.URXISEL  = 0b00;
			U1_RX_ISR_ENABLE   = ENABLED;
		} else {
			U1_RX_ISR_ENABLE   = DISABLED;
		}

		if (udata->tx_pin != INVALID_GPIO_PIN) {
			U1_TX_ISR_PRIOTITY = 0x07;
			U1_TX_ISR_ENABLE   = ENABLED;
			U1STAbits.UTXEN    = ENABLED;

		} else {
			U1_TX_ISR_ENABLE   = DISABLED;
			U1STAbits.UTXEN    = DISABLED;
		}

		U1_ENABLE = ENABLED;
		break;
#endif // SYS_UART1

#if defined(SYS_UART2)
	case UART_2:
		U2MODE = udata->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U2STAbits.URXISEL = 0b10;

		/*
		 * with BRGH = 0 Slow Speed Mode:
		 *        Baudrate - between  sys_clock_freq /(16 * 65536) and sys_clock_freq / 16
		 *                   For sys_clock_freq = 16,000,000 Min 15bps Max 1Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 3bps Max 250Kbps
		 *
		 *                   Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 *                   UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 *                   UxBRG = ((CLOCK / BAUD)/16) -1
		 *
		 *
		 * with BRGH = 1 : sys_clock_freq /(4 * 65536) <= Baudrate <= sys_clock_freq / 4
		 *                   For sys_clock_freq = 16,000,000 Min 61bps Max 4Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 15bps Max 1Mbps
		 */
		if(udata->uart_mode & BRGH_MASK) {
			/*
			 * Hight Speed Mode:
			 */
			U2BRG = (uint16_t)((uint32_t)((uint32_t)((uint32_t)sys_clock_freq / 2) / (uint32_t)udata->baud) / 4) - 1;
		} else {
			/*
			 * Standard Mode:
			 */
			U2BRG = (uint16_t)((uint32_t)((uint32_t)((uint32_t)sys_clock_freq / 2) / (uint32_t)udata->baud) / 16) - 1;
		}


		if (udata->rx_pin != INVALID_GPIO_PIN) {
			LOG_D("Enable Rx ISR\n\r");
                        U2STAbits.URXISEL  = 0b00;
			U2_RX_ISR_ENABLE   = ENABLED;
		} else {
			U2_RX_ISR_ENABLE   = DISABLED;
		}

		if (udata->tx_pin != INVALID_GPIO_PIN) {
			U2_TX_ISR_PRIOTITY = 0x07;
			U2_TX_ISR_ENABLE   = ENABLED;
			U2STAbits.UTXEN    = ENABLED;

		} else {
			U2_TX_ISR_ENABLE   = DISABLED;
			U2STAbits.UTXEN    = DISABLED;
		}

		U2_ENABLE                  = ENABLED;
		break;
#endif // SYS_UART2
	default:
 		return(-ERR_BAD_INPUT_PARAMETER);
	}
        return(SUCCESS);
}
#endif //#if defined(SYS_UART1) || defined(SYS_UART2)

#if defined (SYS_HW_TIMERS)
result_t __attribute__((optimize(0))) delay_uS(uint16_t duration)
{
	struct period period;

	if (duration < 5) {
		return(SUCCESS);
	}

	if (duration == 5) {
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		return(SUCCESS);
	}

	if (duration == 10) {
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");

		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		return(SUCCESS);
	}

	period.units 	= uSeconds;
	period.duration = duration;
	delay(&period);
	return(SUCCESS);
}

uint32_t uc_calculate(struct period *period, uint16_t *clk_divisor)
{
	uint16_t duration;

	switch(period->units) {
	case uSeconds:
		*clk_divisor = 1;

		/*
	         * If the duration is uSeconds compensate for the Instruction overhead of
	         * starting a HW Timer.
	         */
		duration = period->duration;

		if(duration == 25) {
			return(85);
		}
		if(duration == 50) {
			return(818);
		}
		if(duration == 100) {
			return(2285);
		}
		if(period->duration > HW_TIMER_OVERHEAD) {
			duration -= HW_TIMER_OVERHEAD;
		} else {
			/*
			 * The delay passed in is too small to accurately calculate
			 * The calculations depend on sys_clock_freq and multiply and
			 * divide calculations will cost more then the timer.
			 * Roll it by hand!
			 */
			return(-ERR_RANGE_ERROR);
		}
		return((uint32_t) ((uint32_t) (((uint32_t) sys_clock_freq) / 1000000) * (duration)));
                break;

	case mSeconds:
		*clk_divisor = 64;

		if (period->duration == 1) {
			return(451);
		}
		if (period->duration == 5) {
			return(2293);
		}
		if (period->duration == 10) {
			return(4592);
		}
		return((uint32_t) ((uint32_t) ((uint32_t) ((uint32_t) sys_clock_freq) / 128000 ) * period->duration));
                break;

	case Seconds:
		*clk_divisor = 64;
		return ((uint32_t) ((uint32_t) (((uint32_t) sys_clock_freq / 4) / 64) * period->duration));
                break;

	default:
		LOG_E("Bad duration Units for Hardware Timer\n\r");
		return(-ERR_BAD_INPUT_PARAMETER);
	}

}

#endif // (SYS_HW_TIMERS)


#endif // defined(__dsPIC33E256GP502__)
