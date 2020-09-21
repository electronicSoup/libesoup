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

/*
 * SD Card
 * Clk RB11
 * WriteP  RB15
 * CMD RB13
 * DAT3 RB14
 * DAT3 RA0
 * DAT1 RA1

 */
#define SD_CARD_DETECT RB5

#ifdef SYS_CHANGE_NOTIFICATION
void sd_card_detect(enum gpio_pin pin)
{
	LOG_D("SD Card Detect\n\r");
}
#endif

int main(void)
{
	result_t rc;

	rc = libesoup_init();

	rc = slave_24lcxx_init();
	RC_CHECK_PRINT_CONT("24LCxx init failed\n\r");


	gpio_set(SD_CARD_DETECT, GPIO_MODE_DIGITAL_INPUT, 0);
#ifdef SYS_CHANGE_NOTIFICATION
	rc = change_notifier_register(SD_CARD_DETECT, sd_card_detect);
#endif
//	gpio_set(RA4, GPIO_MODE_DIGITAL_OUTPUT, 0);
	/*
	 * RE5        3rd pin <-> Pin 7 of EEPROM WP
	 * RE6 / SCL3 2nd pin <-> Pin 6 of EEPROM SCL
	 * RE7 / SDA3 1st pin <-> Pin 5 of EEPROM SDA
	 */

	LOG_D("Entering main loop\n\r");
	while(1) {
		libesoup_tasks();

//		gpio_toggle_output(RA3);
//		gpio_toggle_output(RA4);

	}
	return(1);
}

#endif // SYS_TEST_24LC64
