#if defined(__dsPIC33EP256MU806__)

#include "system.h"

/*
 * Set up the configuration words of the processor:
 *
 * file:///opt/microchip/xc16/v1.26/docs/config_docs/33EP256MU806.html
 */
#pragma config GWRP = OFF
#pragma config GSS = OFF
#pragma config GSSK = OFF
#pragma config FNOSC = FRC
//#pragma config FNOSC = PRI   // Primary oscillator
//#pragma config FNOSC = SOSC  // Secondary oscillator
#pragma config IESO = OFF
#pragma config POSCMD = HS
#pragma config IOL1WAY = OFF
#pragma config FCKSM = CSECMD
#pragma config FWDTEN = OFF
#pragma config BOREN = OFF
#pragma config JTAGEN = OFF

void cpu_init(void)
{
        clock_init();
        spi_init();        
}

#endif // defined(__dsPIC33EP256MU806__)
