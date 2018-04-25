// HardwareProfile.h

#ifndef _HARDWARE_PROFILE_H_
#define _HARDWARE_PROFILE_H_

#if defined( __C30__ ) || defined __XC16__

        // Various clock values
        #define GetSystemClock()            32000000UL
        #define GetPeripheralClock()        (GetSystemClock() / 2)
        #define GetInstructionClock()       (GetSystemClock() / 2)
    
    // Clock values
    #define MILLISECONDS_PER_TICK       10
    #define TIMER_PRESCALER             TIMER_PRESCALER_8   // 8MHz: TIMER_PRESCALER_1
    #define TIMER_PERIOD                20000                // 10ms=20000, 1ms=2000

#endif


// Define the baud rate constants
    #define BAUDRATE2       57600UL
    #define BRG_DIV2        4
    #define BRGH2           1

    #include <p24Fxxxx.h>
    #include <uart2.h>


/** TRIS ***********************************************************/
#define INPUT_PIN           1
#define OUTPUT_PIN          0

#endif  

