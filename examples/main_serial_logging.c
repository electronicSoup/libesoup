#include "libesoup_config.h"
#include "stdio.h"

#define DEBUG_FILE TRUE
#define TAG "Main"

#include "es_lib/utils/clock.h"
#include "es_lib/logger/serial_log.h"

/*
 *
 */
int main() 
{
        uint8_t  x = 0x12;
        uint16_t y = 0x1234;
        uint32_t z = 0x12345678;
        
        cpu_init();

#if (SYS_LOG_LEVEL != NO_LOGGING)
        serial_logging_init();
#endif
        printf("Testing\n\r");
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        log_d(TAG, "Debug logging statement uint8_t x is 0x%x\n\r", x);
#endif
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_INFO))
        log_i(TAG, "Info logging statement uin16_t y is 0x%x\n\r", y);
#endif
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
        log_w(TAG, "Warning logging statement uin32_t z is 0x%x\n\r", z);
#endif
#if (SYS_LOG_LEVEL <= LOG_ERROR)
        log_e(TAG, "ERROR logging statement\n\r");
#endif
        while(1) {
        }
}
