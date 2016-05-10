/**
 *
 * \file es_lib/timers/hw_timers.c
 *
 * Hardware Timer functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************
 *
 */
#define DEBUG_FILE
#define TAG "HW_TIMERS"

#include "system.h"
#include "es_lib/timers/hw_timers.h"
#include "es_lib/logger/serial_log.h"

#define TIMER_UNUSED  0b00
#define TIMER_RUNNING 0b01
#define TIMER_PAUSED  0b10

struct hw_timer_data {
	u8            status:2;
	u8            repeat:1;
	ty_time_units units;
	u16           time;
	u16           repeats;
	u16           remainder;
	void        (*expiry_function)(u8 data);
	u8            data;
};

static struct hw_timer_data timers[NUMBER_HW_TIMERS];

static result_t  start_timer(u8 timer, ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(u8), u8 data);
static void      set_clock_divide(u8 timer, u16 clock_divide);
static void      check_timer(u8 timer);

void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
	TMR1 = 0x00;

	if(!timers[TIMER_1 - TIMER_1].repeat) {
		/*
		 * Timer is off for the moment.
		 */
		IEC0bits.T1IE = 0;
		T1CONbits.TON = 0;
	}

	check_timer(TIMER_1);
}

void _ISR __attribute__((__no_auto_psv__)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;
	TMR2 = 0x00;

	if(!timers[TIMER_2 - TIMER_1].repeat) {
		/*
		 * Timer is off for the moment.
		 */
		IEC0bits.T2IE = 0;
		T2CONbits.TON = 0;
	}

	check_timer(TIMER_2);
}

void _ISR __attribute__((__no_auto_psv__)) _T3Interrupt(void)
{
	IFS0bits.T3IF = 0;
	TMR3 = 0x00;

	if(!timers[TIMER_3 - TIMER_1].repeat) {
		/*
		 * Timer is off for the moment.
		 */
		IEC0bits.T3IE = 0;
		T3CONbits.TON = 0;
	}
	check_timer(TIMER_3);
}

void _ISR __attribute__((__no_auto_psv__)) _T4Interrupt(void)
{
	IFS1bits.T4IF = 0;
	TMR4 = 0x00;

	if(!timers[TIMER_4 - TIMER_1].repeat) {
		/*
		 * Timer is off for the moment.
		 */
		IEC1bits.T4IE = 0;
		T4CONbits.TON = 0;
	}
	check_timer(TIMER_4);
}

void _ISR __attribute__((__no_auto_psv__)) _T5Interrupt(void)
{
	IFS1bits.T5IF = 0;
	TMR5 = 0x00;

	if(!timers[TIMER_5 - TIMER_1].repeat) {
		/*
		 * Timer is off for the moment.
		 */
		IEC1bits.T5IE = 0;
		T5CONbits.TON = 0;
	}
	check_timer(TIMER_5);
}

void hw_timer_init(void)
{
	u8 loop;

	for(loop = 0; loop < NUMBER_HW_TIMERS; loop++) {
		timers[loop].status = TIMER_UNUSED;
		timers[loop].repeat = 0;
		timers[loop].time = 0;
		timers[loop].expiry_function = (void (*)(u8))NULL;
		timers[loop].data = 0;
		timers[loop].repeats = 0;
		timers[loop].remainder = 0;
	}

	/*
	 * Initialise the timers to 16 bit and their clock source
	 */
	T1CONbits.TCS = 0;      // Internal FOSC/2
	T2CONbits.T32 = 0;      // 16 Bit Timer
	T2CONbits.TCS = 0;      // Internal FOSC/2
	T3CONbits.TCS = 0;      // Internal FOSC/2
	T4CONbits.T32 = 0;      // 16 Bit Timer
	T4CONbits.TCS = 0;      // Internal FOSC/2
	T5CONbits.TCS = 0;      // Internal FOSC/2
}

u8 hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(u8), u8 data)
{
	result_t rc;
	u8       loop;

//	LOG_D("start_hw_timer()\n\r");

	/*
	 * Find a free timer
	 */
	loop = 0;

	while(loop < NUMBER_HW_TIMERS) {
		if(timers[loop].status == TIMER_UNUSED) {
			rc = start_timer(loop, units, time, repeat, expiry_function, data);
			if(rc == SUCCESS) {
				return(loop);
			} else {
				LOG_E("Failed to start HW Timer rc 0x%x\n\r", rc);
			}
		}
		loop++;
	}

	LOG_E("Failed to start the HW Timer\n\r");
	return(BAD_TIMER);
}

result_t hw_timer_restart(u8 timer, ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(u8), u8 data)
{
	if(timer >= NUMBER_HW_TIMERS) {
		LOG_E("Bad time passed to hw_timer_restart()\n\r!");
		return(ERR_BAD_INPUT_PARAMETER);
	}

	return(start_timer(timer, units, time, repeat, expiry_function, data));
}

result_t hw_timer_pause(u8 timer)
{
	if(timer >= NUMBER_HW_TIMERS) {
		LOG_E("Bad timer passed to hw_timer_pause(0x%x)\n\r!", timer);
		return(ERR_BAD_INPUT_PARAMETER);
	}

	if(timers[timer].status == TIMER_UNUSED) {
		LOG_E("Timer passed to hw_timer_pause() is NOT in use\n\r");
		return(ERR_BAD_INPUT_PARAMETER);
	}

	switch (timer) {
		case TIMER_1:
			TMR1 = 0x00;
			IEC0bits.T1IE = 0;
			T1CONbits.TON = 0;
			break;

		case TIMER_2:
			TMR2 = 0x00;
			IEC0bits.T2IE = 0;
			T2CONbits.TON = 0;
			break;

		case TIMER_3:
			TMR3 = 0x00;
			IEC0bits.T3IE = 0;
			T3CONbits.TON = 0;
			break;

		case TIMER_4:
			TMR4 = 0x00;
			IEC1bits.T4IE = 0;
			T4CONbits.TON = 0;
			break;

		case TIMER_5:
			TMR5 = 0x00;
			IEC1bits.T5IE = 0;
			T5CONbits.TON = 0;
			break;

		default:
			LOG_E("Unknown Timer\n\r");
			break;
	}

	timers[timer].status = TIMER_PAUSED;
	return(SUCCESS);
}

void hw_timer_cancel(u8 timer)
{
	hw_timer_pause(timer);

	timers[timer].status = TIMER_UNUSED;
	timers[timer].repeat = 0;
	timers[timer].time = 0;
	timers[timer].expiry_function = (void (*)(u8))NULL;
	timers[timer].data = 0;
	timers[timer].repeats = 0;
	timers[timer].remainder = 0;
}

void hw_timer_cancel_all()
{
	IEC0bits.T1IE = 0;
	T1CONbits.TON = 0;

	IEC0bits.T2IE = 0;
	T2CONbits.TON = 0;

	IEC0bits.T3IE = 0;
	T3CONbits.TON = 0;

	IEC1bits.T4IE = 0;
	T4CONbits.TON = 0;

	IEC1bits.T5IE = 0;
	T5CONbits.TON = 0;
}

result_t start_timer(u8 timer, ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(u8), u8 data)
{
	u32 ticks;

//	LOG_D("start_timer()\n\r");

	switch(units) {
		case uSeconds:
			set_clock_divide(timer, 8);
			ticks = (u32) ((u32) (((u32) CLOCK_FREQ) / 8000000) * time);
			break;

		case mSeconds:
			set_clock_divide(timer, 64);
			ticks = (u32) ((u32) (((u32) CLOCK_FREQ) / 64000 ) * time);
			break;

		case Seconds:
			set_clock_divide(timer, 256);
			ticks = (u32) ((u32) (((u32) CLOCK_FREQ) / 256) * time);
			break;

		default:
			LOG_E("Unknown Timer Units\n\r");
			return(ERR_BAD_INPUT_PARAMETER);
			break;
	}

	if(ticks) {
		timers[timer].status          = TIMER_RUNNING;
		timers[timer].repeat          = repeat;
		timers[timer].units           = units;
		timers[timer].time            = time;
		timers[timer].expiry_function = expiry_function;
		timers[timer].data            = data;

		timers[timer].repeats         = (u16) (ticks / 0xffff);
		timers[timer].remainder       = (u16) (ticks % 0xffff);

		check_timer(timer);
	}

	return(SUCCESS);
}

void set_clock_divide(u8 timer, u16 clock_divide)
{
//	LOG_D("set_clock_divide()\n\r");

	switch(timer) {
		case TIMER_1:
			if(clock_divide == 1) {
				T1CONbits.TCKPS1 = 0; // Divide by 1
				T1CONbits.TCKPS0 = 0;
			} else if(clock_divide == 8) {
				T1CONbits.TCKPS1 = 0; // Divide by 8
				T1CONbits.TCKPS0 = 1;
			} else if(clock_divide == 64) {
				T1CONbits.TCKPS1 = 1; // Divide by 64
				T1CONbits.TCKPS0 = 0;
			} else if(clock_divide == 256) {
				T1CONbits.TCKPS1 = 1; // Divide by 256
				T1CONbits.TCKPS0 = 1;
			} else {
				LOG_E("Bad clock divider!\n\r");
			}
			break;

		case TIMER_2:
			if(clock_divide == 1) {
				T2CONbits.TCKPS1 = 0; // Divide by 1
				T2CONbits.TCKPS0 = 0;
			} else if(clock_divide == 8) {
				T2CONbits.TCKPS1 = 0; // Divide by 8
				T2CONbits.TCKPS0 = 1;
			} else if(clock_divide == 64) {
				T2CONbits.TCKPS1 = 1; // Divide by 64
				T2CONbits.TCKPS0 = 0;
			} else if(clock_divide == 256) {
				T2CONbits.TCKPS1 = 1; // Divide by 256
				T2CONbits.TCKPS0 = 1;
			} else {
				LOG_E("Bad clock divider!\n\r");
			}
			break;

		case TIMER_3:
			if(clock_divide == 1) {
				T3CONbits.TCKPS1 = 0; // Divide by 1
				T3CONbits.TCKPS0 = 0;
			} else if(clock_divide == 8) {
				T3CONbits.TCKPS1 = 0; // Divide by 8
				T3CONbits.TCKPS0 = 1;
			} else if(clock_divide == 64) {
				T3CONbits.TCKPS1 = 1; // Divide by 64
				T3CONbits.TCKPS0 = 0;
			} else if(clock_divide == 256) {
				T3CONbits.TCKPS1 = 1; // Divide by 256
				T3CONbits.TCKPS0 = 1;
			} else {
				LOG_E("Bad clock divider!\n\r");
			}
			break;

		case TIMER_4:
			if(clock_divide == 1) {
				T4CONbits.TCKPS1 = 0; // Divide by 1
				T4CONbits.TCKPS0 = 0;
			} else if(clock_divide == 8) {
				T4CONbits.TCKPS1 = 0; // Divide by 8
				T4CONbits.TCKPS0 = 1;
			} else if(clock_divide == 64) {
				T4CONbits.TCKPS1 = 1; // Divide by 64
				T4CONbits.TCKPS0 = 0;
			} else if(clock_divide == 256) {
				T4CONbits.TCKPS1 = 1; // Divide by 256
				T4CONbits.TCKPS0 = 1;
			} else {
				LOG_E("Bad clock divider!\n\r");
			}
			break;

		case TIMER_5:
			if(clock_divide == 1) {
				T5CONbits.TCKPS1 = 0; // Divide by 1
				T5CONbits.TCKPS0 = 0;
			} else if(clock_divide == 8) {
				T5CONbits.TCKPS1 = 0; // Divide by 8
				T5CONbits.TCKPS0 = 1;
			} else if(clock_divide == 64) {
				T5CONbits.TCKPS1 = 1; // Divide by 64
				T5CONbits.TCKPS0 = 0;
			} else if(clock_divide == 256) {
				T5CONbits.TCKPS1 = 1; // Divide by 256
				T5CONbits.TCKPS0 = 1;
			} else {
				LOG_E("Bad clock divider!\n\r");
			}
			break;

		default:
			LOG_E("Unknown Timer\n\r");
			break;

	}
}

static void check_timer(u8 timer)
{
//	LOG_D("check_timer(%d) repeats 0x%x, remainder 0x%x\n\r", timer, timers[timer].repeats, timers[timer].remainder);

	if(timers[timer].repeats) {
		switch (timer) {
			case TIMER_1:
				TMR1 = 0x00;
				PR1 = 0xffff;

				IEC0bits.T1IE = 1;
				T1CONbits.TON = 1;
				break;

			case TIMER_2:
				TMR2 = 0x00;
				PR2 = 0xffff;

				IEC0bits.T2IE = 1;
				T2CONbits.TON = 1;
				break;

			case TIMER_3:
				TMR3 = 0x00;
				PR3 = 0xffff;

				IEC0bits.T3IE = 1;
				T3CONbits.TON = 1;
				break;

			case TIMER_4:
				TMR4 = 0x00;
				PR4 = 0xffff;

				IEC1bits.T4IE = 1;
				T4CONbits.TON = 1;
				break;

			case TIMER_5:
				TMR5 = 0x00;
				PR5 = 0xffff;

				IEC1bits.T5IE = 1;
				T5CONbits.TON = 1;
				break;

			default:
				LOG_E("Unknown Timer\n\r");
				break;
		}
		timers[timer].repeats--;
	} else if(timers[timer].remainder) {
		switch (timer) {
			case TIMER_1:
				TMR1 = 0x00;
				PR1 = timers[timer].remainder;

				IEC0bits.T1IE = 1;
				T1CONbits.TON = 1;
				break;

			case TIMER_2:
				TMR2 = 0x00;
				PR2 = timers[timer].remainder;

				IEC0bits.T2IE = 1;
				T2CONbits.TON = 1;
				break;

			case TIMER_3:
				TMR3 = 0x00;
				PR3 = timers[timer].remainder;

				IEC0bits.T3IE = 1;
				T3CONbits.TON = 1;
				break;

			case TIMER_4:
				TMR4 = 0x00;
				PR4 = timers[timer].remainder;

				IEC1bits.T4IE = 1;
				T4CONbits.TON = 1;
				break;

			case TIMER_5:
				TMR5 = 0x00;
				PR5 = timers[timer].remainder;

				IEC1bits.T5IE = 1;
				T5CONbits.TON = 1;
				break;

			default:
				LOG_E("Unknown Timer\n\r");
				break;
		}
		timers[timer].remainder = 0;

	} else {
		switch (timer) {
			case TIMER_1:
				TMR1 = 0x00;

				IEC0bits.T1IE = 0;
				T1CONbits.TON = 0;
				break;

			case TIMER_2:
				TMR2 = 0x00;

				IEC0bits.T2IE = 0;
				T2CONbits.TON = 0;
				break;

			case TIMER_3:
				TMR3 = 0x00;

				IEC0bits.T3IE = 0;
				T3CONbits.TON = 0;
				break;

			case TIMER_4:
				TMR4 = 0x00;

				IEC1bits.T4IE = 0;
				T4CONbits.TON = 0;
				break;

			case TIMER_5:
				TMR5 = 0x00;

				IEC1bits.T5IE = 0;
				T5CONbits.TON = 0;
				break;

			default:
				LOG_E("Unknown Timer\n\r");
				break;
		}

		if(timers[timer].expiry_function != NULL) {
			timers[timer].expiry_function(timers[timer].data);
		}

		if(timers[timer].repeat) {
			start_timer(timer, timers[timer].units, timers[timer].time, timers[timer].repeat, timers[timer].expiry_function, timers[timer].data);
		} else {
			timers[timer].status = TIMER_UNUSED;
			timers[timer].repeat = 0;
			timers[timer].expiry_function = (void (*)(u8))NULL;
			timers[timer].data = 0;
			timers[timer].repeats = 0;
			timers[timer].remainder = 0;
		}
	}
}
