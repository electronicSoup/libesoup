#include "libesoup_config.h"

#ifdef SYS_TEST_SLAVE_24LC64

#define DEBUG_FILE
#define TAG "SLV_24LC64_TST"

#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/i2c/i2c.h"
#include "libesoup/comms/i2c/slave_24LCxx.h"

#include "libesoup/comms/i2c/test/Spin_Fv-1_progs.h"


//#define ADDRESS 0x0000
//#define ADDRESS 0x0080
//#define ADDRESS 0x0100
#define ADDRESS 0x0180

int main(void)
{
	result_t rc;

	rc = libesoup_init();

	/*
	 * RE5        3rd pin <-> Pin 7 of EEPROM WP
	 * RE6 / SCL3 2nd pin <-> Pin 6 of EEPROM SCL
	 * RE7 / SDA3 1st pin <-> Pin 5 of EEPROM SDA
	 */


	LOG_D("Entering main loop\n\r");
	while(1) {
		libesoup_tasks();
	}
	return(1);
}

#endif // SYS_TEST_24LC64
