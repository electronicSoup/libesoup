/**
 *
 * \file es_lib/timers/hw_timers.c
 *
 * Hardware Timer functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
//#define DEBUG_FILE
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
	void        (*expiry_function)(void *data);
	void         *data;
};

static struct hw_timer_data timers[NUMBER_HW_TIMERS];

static result_t  start_timer(u8 timer, ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void *), void *data);
static void      set_clock_divide(u8 timer, u16 clock_divide);

void      check_timer(u8 timer);

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
        IEC0bits.T1IE = 0;
        T1CONbits.TON = 0;
        
	check_timer(TIMER_1);
}
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;
        IEC0bits.T2IE = 0;
        T2CONbits.TON = 0;

	check_timer(TIMER_2);
}
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T3Interrupt(void)
{
	IFS0bits.T3IF = 0;
        IEC0bits.T3IE = 0;
        T3CONbits.TON = 0;

	check_timer(TIMER_3);
}
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T4Interrupt(void)
{
	IFS1bits.T4IF = 0;
        IEC1bits.T4IE = 0;
        T4CONbits.TON = 0;

	check_timer(TIMER_4);
}
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T5Interrupt(void)
{
	IFS1bits.T5IF = 0;
        IEC1bits.T5IE = 0;
        T5CONbits.TON = 0;

	check_timer(TIMER_5);
}
#endif

void hw_timer_init(void)
{
	u8 timer;

	for(timer = 0; timer < NUMBER_HW_TIMERS; timer++) {
                LOG_D("Initialise timer 0x%x\n\r", timer);
		timers[timer].status = TIMER_UNUSED;
		timers[timer].repeat = 0;
		timers[timer].time = 0;
		timers[timer].expiry_function = NULL;
		timers[timer].data = 0;
		timers[timer].repeats = 0;
		timers[timer].remainder = 0;
	}

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
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
#elif defined(__18F2680) || defined(__18F4585)
        T0CONbits.T08BIT = 0;   // 16 Bit timer
        T0CONbits.T0CS   = 0;   // Clock source Instruction Clock
        
        T1CONbits.TMR1CS = 0;   // Internal FOSC/4
        T1CONbits.TMR1ON = 0;   // Clock source Instruction Clock
#endif
}

/**
 * hw_timer_start returns the id of the started timer.
 *
 * @param units
 * @param time
 * @param repeat
 * @param expiry_function
 * @param data
 * @return Timer started or BAD_TIMER on an error (No free timers)
 */
u8 hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void *), void *data)
{
	result_t rc;
	u8       timer;

	LOG_D("hw_timer_start()\n\r");

	/*
	 * Find a free timer
	 */
	timer = 0;
        timer = TIMER_1;

//	while(timer < NUMBER_HW_TIMERS) {
		if(timers[timer].status == TIMER_UNUSED) {
			rc = start_timer(timer, units, time, repeat, expiry_function, data);
			if(rc == SUCCESS) {
				return(timer);
			} else {
				LOG_E("Failed to start HW Timer rc 0x%x\n\r", rc);
                                return(BAD_TIMER);
			}
		} else {
                        LOG_D("Timer Busy\n\r");
                }
		timer++;
//	}

	LOG_E("Failed to start the HW Timer\n\r");
	return(BAD_TIMER);
}

result_t hw_timer_restart(u8 timer, ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void *), void *data)
{
	if(timer == BAD_TIMER) {
		return(hw_timer_start(units, time, repeat, expiry_function, data));
	}
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
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
        case TIMER_1:
                TMR1 = 0x00;
                IEC0bits.T1IE = 0;
                T1CONbits.TON = 0;
                break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
        case TIMER_2:
                TMR2 = 0x00;
                IEC0bits.T2IE = 0;
                T2CONbits.TON = 0;
                break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
        case TIMER_3:
                TMR3 = 0x00;
                IEC0bits.T3IE = 0;
                T3CONbits.TON = 0;
                break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
        case TIMER_4:
                TMR4 = 0x00;
                IEC1bits.T4IE = 0;
                T4CONbits.TON = 0;
                break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
        case TIMER_5:
                TMR5 = 0x00;
                IEC1bits.T5IE = 0;
                T5CONbits.TON = 0;
                break;
#endif

        default:
                LOG_E("Unknown Timer\n\r");
                break;
	}

	timers[timer].status = TIMER_PAUSED;
	return(SUCCESS);
}

void hw_timer_cancel(u8 timer)
{
        if(timer < NUMBER_HW_TIMERS) {
                hw_timer_pause(timer);

                timers[timer].status = TIMER_UNUSED;
                timers[timer].repeat = 0;
                timers[timer].time = 0;
                timers[timer].expiry_function = NULL;
                timers[timer].data = 0;
                timers[timer].repeats = (u16)0;
                timers[timer].remainder = (u16)0;
        } else {
                LOG_E("Bad timer passed to hw_timer_cancel(0x%x)\n\r", timer);
        }
}

void hw_timer_cancel_all()
{
	u8 timer;

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
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
#endif

	for(timer = 0; timer < NUMBER_HW_TIMERS; timer++) {
		timers[timer].status = TIMER_UNUSED;
		timers[timer].repeat = 0;
		timers[timer].time = 0;
		timers[timer].expiry_function = NULL;
		timers[timer].data = 0;
		timers[timer].repeats = (u16)0;
		timers[timer].remainder = (u16)0;
	}
}

static result_t start_timer(u8 timer, ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void *), void *data)
{
	u32 ticks = 0;

//	LOG_D("start_timer()\n\r");
	if(timer >= NUMBER_HW_TIMERS) {
                LOG_E("Bad time passed to start_timer(0x%x)\n\r", timer);
                return(ERR_BAD_INPUT_PARAMETER);
        }

	switch(units) {
		case uSeconds:
			set_clock_divide(timer, 1);
#if defined(__18F4585) || defined(__18F2680)
                        ticks = (u32) ((u32) (((u32) CLOCK_FREQ / 4) / 1000000) * time);
#else
			ticks = (u32) ((u32) (((u32) CLOCK_FREQ) / 8000000) * time);
#endif
			break;

		case mSeconds:
#if defined(__18F4585)  || defined(__18F2680)
			set_clock_divide(timer, 4);
			ticks = (u32) ((u32) (((u32)(CLOCK_FREQ / 4)) / 4000) * time);
#else
			set_clock_divide(timer, 64);
			ticks = (u32) ((u32) (((u32) CLOCK_FREQ) / 64000 ) * time);
#endif
			break;

		case Seconds:
#if defined(__18F4585)  || defined(__18F2680)
                        if(timer == TIMER_0) {
                                set_clock_divide(timer, 64);
                                ticks = (u32) ((u32) (((u32) CLOCK_FREQ / 4) / 64) * time);
                        } else if(timer == TIMER_1) {
                                set_clock_divide(timer, 8);
                                ticks = (u32) ((u32) (((u32) CLOCK_FREQ / 4) / 8) * time);
                        } else {
                                LOG_E("Unknown Timers\n\r");
                        }
#else
			set_clock_divide(timer, 256);
			ticks = (u32) ((u32) (((u32) CLOCK_FREQ) / 256) * time);
#endif
			break;

		default:
			LOG_E("Unknown Timer Units\n\r");
			return(ERR_BAD_INPUT_PARAMETER);
//			break;
	}

	if(ticks > 0) {
		timers[timer].status          = TIMER_RUNNING;
		timers[timer].repeat          = repeat;
		timers[timer].units           = units;
		timers[timer].time            = time;
		timers[timer].expiry_function = expiry_function;
		timers[timer].data            = data;

		timers[timer].repeats         = (u16)((ticks >> 16) & 0xffff);
		timers[timer].remainder       = (u16)(ticks & 0xffff);

//                LOG_D("Ticks 0x%lx, Repeats 0x%x, remainder 0x%x\n\r", ticks, timers[timer].repeats, timers[timer].remainder);
//                printf("Ticks 0x%lx, Repeats 0x%x, remainder 0x%x\n\r", ticks, timers[timer].repeats, timers[timer].remainder);
		check_timer(timer);

                return(SUCCESS);
	}

	return(ERR_BAD_INPUT_PARAMETER);
}

static void set_clock_divide(u8 timer, u16 clock_divide)
{
//	LOG_D("set_clock_divide()\n\r");

	switch(timer) {
#if defined(__18F4585) || defined(__18F2680)
        case TIMER_0:
                if(clock_divide == 1) {
                        T0CONbits.PSA    = 1;   // No prescaler
                } else {
                        T0CONbits.PSA    = 0;   // Use prescaler

                        if(clock_divide == 2) {
                                T0CONbits.T0PS = 0x00;   // Divide by 2
                        } else if(clock_divide == 4) {
                                T0CONbits.T0PS = 0x01;   // Divide by 4
                        } else if(clock_divide == 8) {
                                T0CONbits.T0PS = 0x02;   // Divide by 8
                        } else if(clock_divide == 16) {
                                T0CONbits.T0PS = 0x03;   // Divide by 16
                        } else if(clock_divide == 32) {
                                T0CONbits.T0PS = 0x04;   // Divide by 32
                        } else if(clock_divide == 64) {
                                T0CONbits.T0PS = 0x05;   // Divide by 64
                        } else if(clock_divide == 128) {
                                T0CONbits.T0PS = 0x06;   // Divide by 128
                        } else if(clock_divide == 256) {
                                T0CONbits.T0PS = 0x07;   // Divide by 256
                        } else {
                                LOG_E("Unknown divide\n\r");
                        }
                }
                break;
#endif

#if defined(__18F4585) || defined(__18F2680)
        case TIMER_1:
                if(clock_divide == 1) {
                        T1CONbits.T1CKPS = 0x00;   // No prescaler
                } else if(clock_divide == 2) {
                        T1CONbits.T1CKPS = 0x01;   // Divide by 2
                } else if(clock_divide == 4) {
                        T1CONbits.T1CKPS = 0x02;   // Divide by 4
                } else if(clock_divide == 8) {
                        T1CONbits.T1CKPS = 0x03;   // Divide by 8
                } else {
                        LOG_E("Unknow divide\n\r");
                }
                break;
#endif
                
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
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
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
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
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
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
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
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
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
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
#endif

        default:
                LOG_E("Unknown Timer\n\r");
                break;

	}
}

void check_timer(u8 timer)
{
#if defined(__18F4585)
        u16           remainder;
#endif
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
	void        (*expiry)(void *data);
	void         *data;
#endif
//	LOG_D("check_timer(%d) repeats 0x%x, remainder 0x%x\n\r", timer, timers[timer].repeats, timers[timer].remainder);

	if(timers[timer].repeats) {
		switch (timer) {
#if defined(__18F4585)
                case TIMER_0:
                        TMR0H = 0x00;
                        TMR0L = 0x00;
                        INTCONbits.T0IE = 1;
                        T0CONbits.TMR0ON = 1;
                        break;
#endif

#if defined(__18F4585)
                case TIMER_1:
                        TMR1H = 0x00;
                        TMR1L = 0x00;
                        TMR1IE = 1;
                        T1CONbits.TMR1ON = 1;
                        break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_1:
                        TMR1 = 0x00;
                        PR1 = 0xffff;

                        IEC0bits.T1IE = 1;
                        T1CONbits.TON = 1;
                        break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_2:
                        TMR2 = 0x00;
                        PR2 = 0xffff;
                        
                        IEC0bits.T2IE = 1;
                        T2CONbits.TON = 1;
                        break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_3:
                        TMR3 = 0x00;
                        PR3 = 0xffff;
                        
                        IEC0bits.T3IE = 1;
                        T3CONbits.TON = 1;
                        break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_4:
                        TMR4 = 0x00;
                        PR4 = 0xffff;

                        IEC1bits.T4IE = 1;
                        T4CONbits.TON = 1;
                        break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_5:
                        TMR5 = 0x00;
                        PR5 = 0xffff;

                        IEC1bits.T5IE = 1;
                        T5CONbits.TON = 1;
                        break;
#endif

                default:
                        LOG_E("Unknown Timer\n\r");
                        break;
		}
		timers[timer].repeats--;
  //              LOG_D("After Repeats %d\n\r", timers[timer].repeats);
	} else if(timers[timer].remainder) {
                LOG_D("Remainder %d\n\r", timers[timer].remainder);
		switch (timer) {
#if defined(__18F4585)
                case TIMER_0:
                        remainder = 0xffff - timers[timer].remainder;
                        TMR0H = (u8)((remainder >> 8) & 0xff);
                        TMR0L = (u8)(remainder & 0xff);
                        INTCONbits.T0IE = 1;
                        T0CONbits.TMR0ON = 1;
                        break;
#endif

#if defined(__18F4585)
                case TIMER_1:
                        remainder = 0xffff - timers[timer].remainder;
                        TMR1H = (u8)((remainder >> 8) & 0xff);
                        TMR1L = (u8)(remainder & 0xff);
                        TMR1IE = 1;
                        T1CONbits.TMR1ON = 1;
                        break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_1:
                        TMR1 = 0x00;
                        PR1 = timers[timer].remainder;

                        IEC0bits.T1IE = 1;
                        T1CONbits.TON = 1;
                        break;
#endif
                        
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_2:
                        TMR2 = 0x00;
                        PR2 = timers[timer].remainder;

                        IEC0bits.T2IE = 1;
                        T2CONbits.TON = 1;
                        break;
#endif
                        
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_3:
                        TMR3 = 0x00;
                        PR3 = timers[timer].remainder;

                        IEC0bits.T3IE = 1;
                        T3CONbits.TON = 1;
                        break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_4:
                        TMR4 = 0x00;
                        PR4 = timers[timer].remainder;

                        IEC1bits.T4IE = 1;
                        T4CONbits.TON = 1;
                        break;
#endif

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                case TIMER_5:
                        TMR5 = 0x00;
                        PR5 = timers[timer].remainder;

                        IEC1bits.T5IE = 1;
                        T5CONbits.TON = 1;
                        break;
#endif

                default:
                        LOG_E("Unknown Timer\n\r");
                        break;
		}
		timers[timer].remainder = 0;

	} else {
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
                expiry = timers[timer].expiry_function;
                data = timers[timer].data;

		if(timers[timer].repeat) {
			start_timer(timer, timers[timer].units, timers[timer].time, timers[timer].repeat, timers[timer].expiry_function, timers[timer].data);
                        LOG_E("XC8 can't call recursive function, No repeat\n\r");
		} else {
			timers[timer].status = TIMER_UNUSED;
//			timers[timer].repeat = 0;
//			timers[timer].expiry_function = NULL;
//			timers[timer].data = 0;
//			timers[timer].repeats = 0;
//			timers[timer].remainder = 0;
		}

		if(expiry) {
                        LOG_D("Call expiry\n\r");
                        expiry(data);
                }
#elif defined(__18F4585)
                timers[timer].status = TIMER_UNUSED;
                if(timers[timer].expiry_function) {
                        timers[timer].expiry_function(timers[timer].data);
                }
#endif        
	}
}
