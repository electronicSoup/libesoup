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

//#include "libesoup/comms/spi/spi.h"
#include "libesoup/hardware/eeprom.h"
#include "libesoup/timers/sw_timers.h"

static struct timer_req timer_request;

/*
 * Forward declaration of the timer expiry function
 */
void exp_func(timer_id timer, union sigval);

/*
 * Local variables, static to file
 */
static uint8_t eeprom_address = 0;
static uint8_t eeprom_test_rd = 0;

int main(void)
{
        result_t      rc;
        uint8_t       data;
        uint8_t       loop;
	timer_id      timer;

	rc = libesoup_init();
	if(rc != SUCCESS) {
		while(1){}
	}

	eeprom_address = 0;
	eeprom_test_rd = 0;
	
	TIMER_INIT(timer)
	timer_request.units = Seconds;
	timer_request.duration = 1;
	timer_request.type = repeat;
	timer_request.data.sival_int = 0;
	timer_request.exp_fn = exp_func;
	
	rc = sw_timer_start(&timer, &timer_request);
	
	LOG_D("Entering main loop\n\r");

        while(1) {
		CHECK_TIMERS()
        }
        return 0;
}

void exp_func(timer_id timer, union sigval data)
{
	uint8_t  read;
	result_t rc = SUCCESS;

	if(!eeprom_test_rd) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("write to address %d\n\r", eeprom_address);
#endif
                rc = eeprom_write((uint16_t)eeprom_address, (uint8_t)eeprom_address);
		if(rc != SUCCESS) {
			LOG_E("Failed to Write\n\r");
		}
		eeprom_test_rd = 1;
	} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("read from address %d\n\r", eeprom_address);
#endif
                rc = eeprom_read((uint16_t)eeprom_address, &read);
		if(rc != SUCCESS) {
			LOG_E("Failed to Read\n\r");
		} else {
			LOG_D("Read back a value of %d\n\r", read)
		}
		eeprom_address++;
		if(eeprom_address == EEPROM_MAX_ADDRESS) eeprom_address = 0;
		eeprom_test_rd = 0;
	}
}	
