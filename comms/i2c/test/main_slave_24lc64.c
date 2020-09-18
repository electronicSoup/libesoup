#include "libesoup_config.h"

#ifdef SYS_TEST_SLAVE_24LCxx

#define DEBUG_FILE
#define TAG "SLV_24LCxx_TST"

#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/i2c/devices/24lcxx/slave_24LCxx.h"

#include "libesoup/comms/i2c/test/Spin_Fv-1_progs.h"

int main(void)
{
	result_t rc;

	rc = libesoup_init();

	rc = i2c_24lcxx_init(I2C1);
	RC_CHECK_PRINT_CONT("24LCxx init failed\n\r");
//	gpio_set(RA3, GPIO_MODE_DIGITAL_OUTPUT, 0);
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
