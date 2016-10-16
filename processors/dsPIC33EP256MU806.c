#if defined(__dsPIC33EP256MU806__)

#include "system.h"

/*
 * Set up the configuration words of the processor:
 */
_FOSCSEL( FNOSC_FRC & IESO_OFF ) // Start up with FRC Clock source
// Enable Clock Switching and Configure POSC in HS mode
_FOSC( FCKSM_CSECMD & IOL1WAY_OFF & OSCIOFNC_ON & POSCMD_HS )

_FWDT(FWDTEN_OFF)	//watchdog timer off
_FICD(JTAGEN_OFF & ICS_PGD1);//JTAG debugging off, debugging on PG1 pins enabled


#if 0
#ifdef FRC
_FOSCSEL( FNOSC_FRC & IESO_OFF ) // Start up with FRC Clock source
_FOSC( FCKSM_CSDCMD & IOL1WAY_OFF & OSCIOFNC_ON & POSCMD_HS )
#endif

#ifdef PRIMARY
_FOSCSEL( FNOSC_PRI & IESO_OFF )
_FOSC( FCKSM_CSDCMD & IOL1WAY_OFF & OSCIOFNC_ON & POSCMD_HS )
#endif

#ifdef SECONDARY
_FOSCSEL( FNOSC_SOSC & IESO_OFF )
#endif
#endif


#endif // defined(__dsPIC33EP256MU806__)
