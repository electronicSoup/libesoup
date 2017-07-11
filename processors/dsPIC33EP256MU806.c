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

#include "libesoup/utils/clock.h"
#include "libesoup/utils/spi.h"

void cpu_init(void)
{
        clock_init();
        
        INTCON1bits.NSTDIS = 1; // No nested Interrupts
        INTCON2bits.GIE = ENABLE;
#ifdef SYS_SPI_BUS
        spi_init();
#endif
}

#endif // defined(__dsPIC33EP256MU806__)
