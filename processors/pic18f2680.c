#include "system.h"
#include "es_lib/timers/hw_timers.h"

extern void pic18f2680_timer_isr(u8 timer);

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
                TMR0IF=0;
                pic18f2680_timer_isr(TIMER_0);
        }

        if (TMR1IE && TMR1IF) {
                TMR1IF=0;
                pic18f2680_timer_isr(TIMER_1);
        }

	if(PIR1bits.TXIF) {
                serial_isr();
        }
        // process other interrupt sources here, if required
}

void interrupt low_priority tc_clr(void) {
// process any other low priority sources here
}
