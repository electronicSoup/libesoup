/*
 * libesoup_conffig.h libesoup/comms/i2c/test/main_master_24lc64.c
 *
 * Circuit wired using:
 * RE7 SDA3
 * RD6 SCL3
 */

#include "libesoup_config.h"

#ifdef SYS_TEST_MASTER_24LC64

#define DEBUG_FILE
#define TAG "24LC64_TST"

#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/i2c/i2c.h"
#include "libesoup/comms/i2c/devices/24lcxx/master_24LCxx.h"
#include "libesoup/timers/sw_timers.h"

#define PROGRAM_SIZE   128 //4*128

//#define ADDRESS 0x0000
//#define ADDRESS 0x0080
//#define ADDRESS 0x0100
#define ADDRESS 0x0180

static uint8_t  buffer[PROGRAM_SIZE];
static uint8_t  done = 0;

void callback_24lc64(result_t rc, uint8_t *buffer, uint8_t count)
{
        LOG_D("callback_24lc64()\n\r");

	if(rc == SUCCESS) {
		done = 1;
		LOG_D("SUCCESS 0x%x count %d\n\r", ADDRESS, count);
	}
}

static void print_one(void)
{
	static uint16_t loop = 0;

	if (done) {
		serial_printf("%d    : 0x%x\n\r", loop, buffer[loop]);
		loop++;

		if (loop >= PROGRAM_SIZE) {
			done = 0;
		}
	}
}

static void expiry(timer_id timer  __attribute__((unused)), union sigval data __attribute__((unused)))
{
	result_t rc;

	rc = mc24lc64_read(I2C3, 0x00, ADDRESS, PROGRAM_SIZE, buffer, callback_24lc64);
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

	gpio_set(RE5, GPIO_MODE_DIGITAL_OUTPUT, 1);

	request.period.units    = Seconds;
	request.period.duration = 5;
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
		if (done) {
			print_one();
		}
	}
	return(1);
}

#endif // SYS_TEST_24LC64
