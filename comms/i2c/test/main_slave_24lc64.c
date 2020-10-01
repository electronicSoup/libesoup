#include "libesoup_config.h"

#ifdef SYS_TEST_SLAVE_24LCxx

#define DEBUG_FILE
#define TAG "SLV_24LCxx_TST"

#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/gpio.h"
#ifdef SYS_CHANGE_NOTIFICATION
#include "libesoup/gpio/change_notification.h"
#endif
#include "libesoup/comms/i2c/devices/24lcxx/slave_24LCxx.h"

#if defined(SYS_SD_CARD)
#include "libesoup/comms/spi/devices/sd_card.h"


#endif // SYS_SD_CARD

#include "libesoup/timers/sw_timers.h"

static uint32_t count = 0;

void expiry(timer_id timer, union sigval data)
{
	count++;
	LOG_D("Count %ld\n\r", count);
}

int main(void)
{
	result_t rc;
	struct timer_req request;

	rc = libesoup_init();
#ifdef SYS_SLV_24LCxx
	rc = slave_24lcxx_init();
	if (rc < 0) {
		LOG_E("24LCxx init failed\n\r");
	} else {
		LOG_D("Returned 0x%x\n\r");
	}
#endif
#ifdef SYS_SD_CARD
	rc = sd_card_init();
	if (rc < 0) {
		LOG_E("SD Card init failed\n\r");
	} else {
//		rc = sd_card_read(0x0000);
	}
#endif
//	gpio_set(RA4, GPIO_MODE_DIGITAL_OUTPUT, 0);
	/*
	 * RE5        3rd pin <-> Pin 7 of EEPROM WP
	 * RE6 / SCL3 2nd pin <-> Pin 6 of EEPROM SCL
	 * RE7 / SDA3 1st pin <-> Pin 5 of EEPROM SDA
	 */

	request.period.units = Seconds;
	request.period.duration = 60;
	request.data.sival_int = 0;
	request.type = repeat_expiry;
	request.exp_fn = expiry;
	sw_timer_start(&request);
	LOG_D("Entering main loop\n\r");
	while(1) {
		libesoup_tasks();
	}
	return(1);
}

#endif // SYS_TEST_24LC64
