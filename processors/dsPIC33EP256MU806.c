#if defined(__dsPIC33EP256MU806__)

#include "libesoup_config.h"

/*
 * Set up the configuration words of the processor:
 *
 * file:///opt/microchip/xc16/v1.26/docs/config_docs/33EP256MU806.html
 */
#pragma config GWRP = OFF
#pragma config GSS = OFF
#pragma config GSSK = OFF
//#pragma config FNOSC = FRC
#pragma config FNOSC = PRI   // Primary oscillator
//#pragma config FNOSC = SOSC  // Secondary oscillator
#pragma config IESO = OFF
#pragma config POSCMD = HS
#pragma config IOL1WAY = OFF
#pragma config FCKSM = CSECMD
#pragma config FWDTEN = OFF
#pragma config WINDIS   = OFF              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config BOREN = OFF
#pragma config JTAGEN = OFF

#include "libesoup/timers/clock.h"

#ifdef SYS_SPI_BUS
#include "libesoup/utils/spi.h"
#endif

#include "libesoup/logger/serial_log.h"

#ifdef SYS_HW_TIMERS
#include "libesoup/timers/hw_timers.h"
#endif

#ifdef SYS_SW_TIMERS
#include "libesoup/timers/sw_timers.h"
#endif

void cpu_init(void)
{
#if (SYS_LOG_LEVEL != NO_LOGGING)
	result_t rc;
#endif	
        clock_init();
        
        INTCON2bits.GIE = ENABLED;
	
#if (SYS_LOG_LEVEL != NO_LOGGING)
	rc = serial_logging_init();
#endif

#ifdef SYS_HW_TIMERS
	hw_timer_init();
#endif
	
#ifdef SYS_SW_TIMERS
	sw_timer_init();
#endif

#ifdef SYS_SPI_BUS
        spi_init();
#endif
}

#endif // defined(__dsPIC33EP256MU806__)
