#if defined(__PIC24FJ256GB106__)

#include <xc.h>
#include "libesoup/utils/spi.h"
#include "libesoup/utils/clock.h"

#pragma config FWDTEN   = OFF
#pragma config JTAGEN   = OFF
#pragma config WDTPS    = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA    = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS   = OFF              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config ICS      = PGx2             // Comm Channel Select (Emulator functions are shared with PGEC2/PGED2)
#pragma config FNOSC    = PRI              // Oscillator Select (Primary oscillator (XT, HS, EC) with PLL module (XTPLL,HSPLL, ECPLL))
#pragma config POSCMOD  = HS               // Primary Oscillator Select (HS oscillator mode selected)
#pragma config DISUVREG = OFF              // Internal USB 3.3V Regulator Disable bit (Regulator is disabled)

void cpu_init(void)
{
        clock_init();
#ifdef SYS_SPI_BUS
        spi_init();
#endif
}
        
#endif // defined(__PIC24FJ256GB106__)
