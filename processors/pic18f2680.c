#if defined(__18F2680)

#include "system.h"
#include <stdio.h>
#include "es_lib/timers/hw_timers.h"

extern void check_timer(uint8_t timer);

#pragma config WDT = OFF
#pragma config MCLRE = OFF
#pragma config DEBUG = OFF
#pragma config LVP = OFF
//#pragma config OSC = IRCIO67    // Internal Osc with IO on Ports
#pragma config OSC = HS         //Crystal
#pragma config IESO = OFF
#pragma config XINST = OFF
#pragma config PWRT = OFF
#pragma config BOREN = OFF
#pragma config STVREN = ON
#pragma config PBADEN = OFF
#pragma config CP0 = OFF
#pragma config CP1 = OFF
#pragma config CP2 = OFF
#pragma config CPD = OFF
#pragma config CPB = OFF
#pragma config WRT0 = OFF
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF

extern void pic18f_timer_isr(uint8_t timer);
extern void pic18f_serial_isr(void);

void pic18f_init(void)
{
        RCONbits.IPEN = 0; // No Interrupt priority level
}

/*
 * Two Interrupt Service Routines
 */
#if 0
void interrupt high_priority high_isr(void)
{
//	serial_isr();
//	timer_isr();
}
#endif

#if 0
void interrupt low_priority low_isr(void)
{
	count++;
	if(count > ROLLOVER) {
		HEARTBEAT_LED = ~HEARTBEAT_LED;
		count = 0;
	}
#ifdef SERIAL
	serial_isr();
#endif
#ifdef TIME
	timer_isr();
#endif
}
#endif // 0

void interrupt tc_int(void)
{
        if (TMR0IE && TMR0IF) {
                TMR0IF = 0;
                TMR0IE = 0;
                T0CONbits.TMR0ON = 0;
                check_timer(TIMER_0);
        }

        if (TMR1IE && TMR1IF) {
                TMR1IF=0;
                TMR1IE = 0;
                T1CONbits.TMR1ON = 0;
                check_timer(TIMER_1);
        }

	if(PIR1bits.TXIF) {
                pic18f_serial_isr();
        }
        // process other interrupt sources here, if required
}

void interrupt low_priority tc_clr(void) {
// process any other low priority sources here
}

#endif // __18F2680
