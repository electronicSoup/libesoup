/*
 * File:   main.c
 * Author: john
 *
 * Created on 26 September 2016, 20:12
 */

/*
 * Set up the configuration words of the processor:
 */

#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "Main";

#include "libesoup/logger/serial_log.h"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif
#endif

#include "libesoup/comms/spi/spi.h"
#include "libesoup/hardware/eeprom.h"

/*
 * Forward declaration of the timer expiry function
 */
void exp_func(void *);

int main(void)
{
        result_t      rc;
        uint8_t       data;
        uint8_t       loop;

	rc = libesoup_init();
	if(rc != SUCCESS) {
		while(1){}
	}
	LOG_D("Testing\n\r");
#if 1
        for(loop = 0; loop <= EEPROM_MAX_ADDRESS; loop++) {
                rc = eeprom_write((uint16_t)loop, loop);
                if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		        LOG_E("Failed to write to EEPROM loop %d\n\r", loop);
#endif
                }
        }
#endif // 0
	LOG_D("EEPROM Written\n\r");
        for(loop = 0; loop <= EEPROM_MAX_ADDRESS; loop++) {
#if 1
                rc = eeprom_read((uint16_t)loop, &data);
                if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		        LOG_E("Failed to read to EEPROM\n\r");
#endif
                }
#endif // 0
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//		if((loop % 10) == 0) {
//			LOG_D("Read back 0x%x from %d\n\r", data, loop);
			LOG_D("Read back from 0\n\r");
//		}
#endif
        }
	
	LOG_D("Entering main loop\n\r");

        while(1) {
                Nop();
        }
        return 0;
}
