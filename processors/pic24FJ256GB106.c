#if defined(__PIC24FJ256GB106__)

#include <xc.h>
#include "es_lib/utils/spi.h"

//_CONFIG1(JTAGEN_OFF & FWDTEN_OFF & FWPSA_PR32 & WDTPS_PS1024 & WINDIS_OFF & ICS_PGx2)   // JTAG off, watchdog timer off
//_CONFIG2(FNOSC_FRCPLL & POSCMOD_NONE & OSCIOFNC_ON & PLL_96MHZ_ON & PLLDIV_NODIV & DISUVREG_OFF)  // CLOCK 16000000

/*
 * /opt/microchip/mla/v2015_08_10/apps/usb/device/hid_joystick/firmware/src/system_config/exp16/pic24fj256gb110_pim/system.c
 */
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
