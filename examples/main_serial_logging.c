#include "libesoup_config.h"
#include "stdio.h"

#define DEBUG_FILE TRUE
#define TAG "Main"

#include "libesoup/utils/clock.h"
#include "libesoup/comms/uart/uart.h"
#include "libesoup/logger/serial_log.h"

/*
 *
 */
int main() 
{
        result_t rc;
        uint8_t  x = 0x12;
        uint16_t y = 0x1234;
        uint32_t z = 0x12345678;
        
        cpu_init();

        /*
         * Initialise the UART management data structures. Needed for Serial
         * Logging.
         */
        
#if (SYS_LOG_LEVEL != NO_LOGGING)
        uart_init();
        rc = serial_logging_init();
        if (rc != SUCCESS) {
                /*
                 * What to do?
                 */
                return(1);
        }
#endif

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("Debug logging statement uint_8 x is 0x%x\n\r", x);
#endif
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_INFO))
        LOG_I("Info logging statement uin16_t y is 0x%x\n\r", y);
#endif
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
        LOG_W("Warning logging statement uin32_t z is 0x%lx\n\r", z);
#endif
#if (SYS_LOG_LEVEL <= LOG_ERROR)
        LOG_W("ERROR logging statement\n\r");
#endif
        while(1) {
        }
}
