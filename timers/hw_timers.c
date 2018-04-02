/*
 *
 * file libesoup/timers/hw_timers.c
 *
 * author John Whitmore
 *
 * Hardware Timer functionality for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017-2018 electronicSoup Limited
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
#if defined(XC16) || defined(__XC8)

/*
 * XC8 Compiler warns about unused functions
 */
#if defined (__XC8)
#pragma warning disable 520
#endif


#include "libesoup_config.h"

#ifdef SYS_HW_TIMERS

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "HW_TIMERS";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

#include "libesoup/errno.h"
#include "libesoup/timers/hw_timers.h"

/*
 * Local definitions
 */

/*
 * constants:
 */
/*
 * Definitions for the status of a hardware timer
 */
#define TIMER_UNUSED  0b00
#define TIMER_RUNNING 0b01
#define TIMER_PAUSED  0b10

/*
 * Data structures:
 */
/*
 * Private structure to manage the hardware timers of the micro controller
 * should not be used outside this file.
 */
struct hw_timer_data {
	uint8_t          status;
	struct timer_req request;
	uint16_t         repeats;
	uint16_t         remainder;
};

/*
 * Private array managing the hardware timers in the micro-controller. The size
 * is defined by the constant NUMBER_OF_TIMERS which should be defined in a
 * processor header file, located in libesoup/processors.
 * 
 * The processor header file should be included in a board header file 
 * "libesoup/boards/..." which is included by the libesoup_config.h configuration header
 * file. So the libesoup_config.h file defines the board being used and the board file
 * defines the micro controller being used. The microcontroller ultimately 
 * dictates the number of hardware timers.
 */
static struct hw_timer_data timers[NUMBER_HW_TIMERS];

/*
 * Function definitions:
 */
static timer_id start_timer(timer_id timer, struct timer_req *request);
static void     set_clock_divide(timer_id timer, uint16_t clock_divide);

/*
 * Called from external scope by pic18f4585.c
 */
void check_timer(timer_id timer);

/*
 * The Code:
 */

/*
 * The function simply clears the Register flags associated with the interrupt
 * and calls the local static function check_timer() passing it the identifier
 * for the expired timer. The check_timer() function does the main processing
 * for all expired timers.
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
        IEC0bits.T1IE = 0;
        T1CONbits.TON = 0;
        
	check_timer(TIMER_1);
}
#endif

/*
 * The function simply clears the Register flags associated with the interrupt
 * and calls the local static function check_timer() passing it the identifier
 * for the expired timer. The check_timer() function does the main processing
 * for all expired timers.
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;
        IEC0bits.T2IE = 0;
        T2CONbits.TON = 0;

	check_timer(TIMER_2);
}
#endif

/*
 * The function simply clears the Register flags associated with the interrupt
 * and calls the local static function check_timer() passing it the identifier
 * for the expired timer. The check_timer() function does the main processing
 * for all expired timers.
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T3Interrupt(void)
{
	IFS0bits.T3IF = 0;
        IEC0bits.T3IE = 0;
        T3CONbits.TON = 0;

	check_timer(TIMER_3);
}
#endif

/*
 * The function simply clears the Register flags associated with the interrupt
 * and calls the local static function check_timer() passing it the identifier
 * for the expired timer. The check_timer() function does the main processing
 * for all expired timers.
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T4Interrupt(void)
{
	IFS1bits.T4IF = 0;
        IEC1bits.T4IE = 0;
        T4CONbits.TON = 0;

	check_timer(TIMER_4);
}
#endif

/*
 * The function simply clears the Register flags associated with the interrupt
 * and calls the local static function check_timer() passing it the identifier
 * for the expired timer. The check_timer() function does the main processing
 * for all expired timers.
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void __attribute__((__interrupt__, __no_auto_psv__)) _T5Interrupt(void)
{
	IFS1bits.T5IF = 0;
        IEC1bits.T5IE = 0;
        T5CONbits.TON = 0;

	check_timer(TIMER_5);
}
#endif

/*
 */
void hw_timer_init(void)
{
	timer_id timer;

	for(timer = 0; timer < NUMBER_HW_TIMERS; timer++) {
		timers[timer].status = TIMER_UNUSED;
		timers[timer].request.type = single_shot;
		timers[timer].request.duration = 0;
		timers[timer].request.exp_fn = NULL;
		timers[timer].request.data.sival_int = 0;
		timers[timer].repeats = 0;
		timers[timer].remainder = 0;
	}

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
	/*
	 * Initialise the timers to 16 bit and their clock source
	 */
	T1CONbits.TCS = 0;      // Internal FOSC/2
	T1CONbits.TGATE = 0;
        IPC0bits.T1IP = 0x06;   // Higest Priority

	T2CONbits.T32 = 0;      // 16 Bit Timer
	T2CONbits.TCS = 0;      // Internal FOSC/2
        T2CONbits.TGATE = 0;
        IPC1bits.T2IP = 0x06;   // Higest Priority
        
	T3CONbits.TCS = 0;      // Internal FOSC/2
        T3CONbits.TGATE = 0;
        IPC2bits.T3IP = 0x06;   // Higest Priority
        
	T4CONbits.T32 = 0;      // 16 Bit Timer
	T4CONbits.TCS = 0;      // Internal FOSC/2
        T4CONbits.TGATE = 0;
        IPC6bits.T4IP = 0x06;   // Higest Priority
        
	T5CONbits.TCS = 0;      // Internal FOSC/2
        T5CONbits.TGATE = 0;
        IPC7bits.T5IP = 0x06;   // Higest Priority
#elif defined(__18F2680) || defined(__18F4585)
        T0CONbits.T08BIT = 0;         // 16 Bit timer
        T0CONbits.T0CS   = 0;         // Clock source Instruction Clock
	T0CONbits.TMR0ON = DISABLED;  // Timer off for the moment
        
        T1CONbits.TMR1CS = 0;         // Internal FOSC/4
        T1CONbits.TMR1ON = DISABLED;  // Timer off for the moment
	
	/*
	 * PIC18F TImer 2 is an 8 bit timer?
	 */
#endif
}

#if (SYS_LOG_LEVEL != NO_LOGGING)
uint8_t hw_timer_active_count(void)
{
        uint8_t  count = 0;
	timer_id timer;

	for(timer = 0; timer < NUMBER_HW_TIMERS; timer++) {
		if(timers[timer].status != TIMER_UNUSED)
                        count++;
        }
        return(count);
}
#endif

/*
 * hw_timer_start returns the id of the started timer.
 */
timer_id hw_timer_start(struct timer_req *request)
{
	timer_id timer;

	/*
	 * XC8 Compiler don't like recursion have to find a way around this.
	 */
#if defined(__XC8)
	if(request->type == repeat) return(-ERR_BAD_INPUT_PARAMETER);
#endif // 18F4585
	/*
	 * Find a free timer
	 */
	timer = 0;
	while(timer < NUMBER_HW_TIMERS) {
		if(timers[timer].status == TIMER_UNUSED) {
			return(start_timer(timer, request));
		}
		/*
		 * Increment the value pointed at not the address
		 */
		timer++;
	}
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
	LOG_E("Failed to start the HW Timer\n\r");
#endif
	return(-ERR_NO_RESOURCES);
}

timer_id hw_timer_restart(timer_id timer, struct timer_req *request)
{
	if(timer == BAD_TIMER_ID) {
		return(hw_timer_start(request));
	}
	if(timer >= NUMBER_HW_TIMERS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Bad time passed to hw_timer_restart()\n\r!");
#endif
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	return(start_timer(timer, request));
}

timer_id hw_timer_pause(timer_id timer)
{
	if(timer >= NUMBER_HW_TIMERS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Bad timer passed to hw_timer_pause(0x%x)\n\r!", timer);
#endif
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	if(timers[timer].status == TIMER_UNUSED) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Timer passed to hw_timer_pause() is NOT in use\n\r");
#endif
		return(-ERR_BAD_INPUT_PARAMETER);
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                LOG_E("Unknown Timer\n\r");
#endif
                break;
	}

	timers[timer].status = TIMER_PAUSED;
	return(timer);
}

timer_id hw_timer_cancel(timer_id timer)
{
        if(timer < NUMBER_HW_TIMERS) {
                hw_timer_pause(timer);

                timers[timer].status = TIMER_UNUSED;
                timers[timer].request.exp_fn = NULL;
                timers[timer].repeats = (uint16_t)0;
                timers[timer].remainder = (uint16_t)0;
		
		return(timer);
        } else {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                LOG_E("Bad timer passed to hw_timer_cancel(0x%x)\n\r", timer);
#endif
		return(-ERR_BAD_INPUT_PARAMETER);
        }
}

void hw_timer_cancel_all()
{
	timer_id timer;

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
		timers[timer].request.exp_fn = NULL;
		timers[timer].repeats = (uint16_t)0;
		timers[timer].remainder = (uint16_t)0;
	}
}

static timer_id start_timer(timer_id timer, struct timer_req *request)
{
	uint32_t ticks = 0;
	uint16_t duration;

	if(timer >= NUMBER_HW_TIMERS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                LOG_E("Bad time passed to start_timer(0x%x)\n\r", timer);
#endif
                return(-ERR_BAD_INPUT_PARAMETER);
        }

	switch(request->units) {
	case uSeconds:
		/*
	         * If the duration is uSeconds compensate for the Instruction overhead of
	         * starting a HW Timer.
	         */
		duration = request->duration;
		
#if defined(__dsPIC33EP256MU806__)
		if(request->duration > HW_TIMER_OVERHEAD) {
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
#endif // defined(__dsPIC33EP256MU806__)
		set_clock_divide(timer, 1);
#if defined(__18F4585) || defined(__18F2680)
		ticks = (uint32_t) ((uint32_t) (((uint32_t) sys_clock_freq / 4) / 1000000) * request->duration);
#elif defined(__dsPIC33EP256MU806__)
		ticks = (uint32_t) ((uint32_t) (((uint32_t) sys_clock_freq) / 1000000) * (duration));
#endif // Target uC
                break;

	case mSeconds:
#if defined(__18F4585)  || defined(__18F2680)
		set_clock_divide(timer, 4);
		ticks = (uint32_t) ((uint32_t) (((uint32_t)(sys_clock_freq / 4)) / 4000) * request->duration);
#else
                /*
                 * Divided by 64 so sys_clock_freq/64 ticks in a Second
                 * 1mS = 1Sec/1,000
                 * Ticks in 1mS = (sys_clock_freq/64)/1,000
                 *              = sys_clock_freq / 64 * 1,000
                 */
		set_clock_divide(timer, 64);
		ticks = (uint32_t) ((uint32_t) (((uint32_t) sys_clock_freq) / 64000 ) * request->duration);
#endif
                break;

	case Seconds:
#if defined(__18F4585)  || defined(__18F2680)
		if(timer == TIMER_0) {
			set_clock_divide(timer, 64);
			ticks = (uint32_t) ((uint32_t) (((uint32_t) sys_clock_freq / 4) / 64) * request->duration);
		} else if(timer == TIMER_1) {
			set_clock_divide(timer, 8);
			ticks = (uint32_t) ((uint32_t) (((uint32_t) sys_clock_freq / 4) / 8) * request->duration);
		} else {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Unknown Timers\n\r");
#endif
		}
#else
                /*
                 * Divide by 256 so 
                 * in 1 Second (sys_clock_freq/256) ticks
                 */
		set_clock_divide(timer, 256);
		ticks = (uint32_t) ((uint32_t) (((uint32_t) sys_clock_freq) / 256) * request->duration);
#endif
                break;

	default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Bad duration Units for Hardware Timer\n\r");
#endif
		return(-ERR_BAD_INPUT_PARAMETER);
#ifndef __XC8
		break;
#endif
	}

	if(ticks > 0) {
		timers[timer].status            = TIMER_RUNNING;
		timers[timer].request.type      = request->type;
		timers[timer].request.units     = request->units;
		timers[timer].request.duration  = request->duration;
		timers[timer].request.exp_fn    = request->exp_fn;
		timers[timer].request.data      = request->data;

		timers[timer].repeats           = (uint16_t)((ticks >> 16) & 0xffff);
		timers[timer].remainder         = (uint16_t)(ticks & 0xffff);

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//                LOG_D("Ticks 0x%lx, Repeats 0x%x, remainder 0x%x\n\r", ticks, timers[timer].repeats, timers[timer].remainder);
#endif
		check_timer(timer);

                return(timer);
	} else {
                /*
                 * Simply call the expiry function
                 */
                request->exp_fn(timer, request->data);
                return(0);
        }
#ifndef __XC8 // X8 Compiler warns about unreachable code
	return(-ERR_BAD_INPUT_PARAMETER);
#endif // __XC8
}

static void set_clock_divide(timer_id timer, uint16_t clock_divide)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//	LOG_D("set_clock_divide()\n\r");
#endif

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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                                LOG_E("Unknown divide\n\r");
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Unknow divide\n\r");
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Bad clock divider!\n\r");
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Bad clock divider!\n\r");
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Bad clock divider!\n\r");
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Bad clock divider!\n\r");
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Bad clock divider!\n\r");
#endif
                }
                break;
#endif

        default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                LOG_E("Unknown Timer\n\r");
#endif
                break;

	}
}

/*
 * Not static as called from pic18f4585.c processor
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
void check_timer(timer_id timer)
{
	expiry_function  expiry;
	union sigval     data;

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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Unknown Timer\n\r");
#endif
                        break;
		}
		timers[timer].repeats--;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
  //              LOG_D("After Repeats %d\n\r", timers[timer].repeats);
#endif
	} else if(timers[timer].remainder) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//                LOG_D("Remainder %d\n\r", timers[timer].remainder);
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Unknown Timer\n\r");
#endif
                        break;
		}
		timers[timer].remainder = 0;

	} else {
                expiry = timers[timer].request.exp_fn;
                data = timers[timer].request.data;

		if(timers[timer].request.type == repeat) {
			start_timer(timer, &timers[timer].request);
		} else {
			timers[timer].status = TIMER_UNUSED;
		}

		if(expiry) {
                        expiry(timer, data);
                }
	}
}
#endif  // #if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)


#if defined(__18F4585)
void check_timer(timer_id timer)
{
        uint16_t           remainder;

	if(timers[timer].repeats) {
		switch (timer) {
                case TIMER_0:
                        TMR0H = 0x00;
                        TMR0L = 0x00;
                        INTCONbits.T0IE = 1;
                        T0CONbits.TMR0ON = 1;
                        break;

                case TIMER_1:
                        TMR1H = 0x00;
                        TMR1L = 0x00;
                        TMR1IE = 1;
                        T1CONbits.TMR1ON = 1;
                        break;

                default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Unknown Timer\n\r");
#endif
                        break;
		}
		timers[timer].repeats--;
	} else if(timers[timer].remainder) {

		switch (timer) {
                case TIMER_0:
                        remainder = 0xffff - timers[timer].remainder;
                        TMR0H = (uint8_t)((remainder >> 8) & 0xff);
                        TMR0L = (uint8_t)(remainder & 0xff);
                        INTCONbits.T0IE = 1;
                        T0CONbits.TMR0ON = 1;
                        break;

                case TIMER_1:
                        remainder = 0xffff - timers[timer].remainder;
                        TMR1H = (uint8_t)((remainder >> 8) & 0xff);
                        TMR1L = (uint8_t)(remainder & 0xff);
                        TMR1IE = 1;
                        T1CONbits.TMR1ON = 1;
                        break;

                default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Unknown Timer\n\r");
#endif
                        break;
		}
		timers[timer].remainder = 0;

	} else {
                timers[timer].status = TIMER_UNUSED;
                if(timers[timer].request.exp_fn) {
                        timers[timer].request.exp_fn(timer, timers[timer].request.data);
                }
	}
}
#endif // defined(__18F4585)

#endif // #ifdef SYS_HW_TIMERS

#endif // defined(XC16) || defined(__XC8)
