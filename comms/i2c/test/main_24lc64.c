#include "libesoup_config.h"

#ifdef SYS_TEST_24LC64

#define DEBUG_FILE
#define TAG "24LC64_TST"

#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/i2c/i2c.h"
#include "libesoup/comms/i2c/mc24LC64.h"
#include "libesoup/timers/sw_timers.h"

void callback_24lc64(result_t rc, uint8_t *buffer)
{
        LOG_D("callback_24lc64()\n\r");
}

static void expiry(timer_id timer  __attribute__((unused)), union sigval data __attribute__((unused)))
{
	result_t rc;
        uint8_t  buffer[10];

//	gpio_set(RE5, GPIO_MODE_DIGITAL_OUTPUT, ~gpio_get(RE5));
        rc = mc24lc64_read(I2C3, 0x00, 0x00, 0x01, buffer, callback_24lc64);
	if (rc < 0) {
		LOG_E("failed to read - %s\n\r", error_text(rc));
	}
}

int main(void)
{
	result_t rc;
	struct timer_req request;
	timer_id         timer;

	rc = libesoup_init();

	/*
	 * RE5        3rd pin <-> Pin 7 of EEPROM WP
	 * RE6 / SCL3 2nd pin <-> Pin 6 of EEPROM SCL
	 * RE7 / SDA3 1st pin <-> Pin 5 of EEPROM SDA
	 */

	gpio_set(RE5, GPIO_MODE_DIGITAL_OUTPUT, 0);

	request.period.units    = Seconds;
	request.period.duration = 10;
//	request.type            = repeat_expiry;
	request.type            = single_shot_expiry;
	request.exp_fn          = expiry;
	request.data.sival_int  = 0;

        timer = sw_timer_start(&request);

        if(timer < 0) {
		// Error Condition
        }

	LOG_D("Entering main loop\n\r");
	while(1) {
		libesoup_tasks();
	}
	return(1);
}

#endif // SYS_TEST_24LC64
