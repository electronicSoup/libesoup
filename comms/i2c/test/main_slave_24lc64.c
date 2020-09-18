#include "libesoup_config.h"

#ifdef SYS_TEST_SLAVE_24LC64

#define DEBUG_FILE
#define TAG "SLV_24LC64_TST"

#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/i2c/i2c.h"
#include "libesoup/comms/i2c/devices/24lcxx/slave_24LCxx.h"

#include "libesoup/comms/i2c/test/Spin_Fv-1_progs.h"

#define I2C3_STOPPED         I2C3STATbits.P
#define I2C3_STARTED         I2C3STATbits.S
#define I2C3_RX_BYTE         I2C3STATbits.RBF

#define I2C3_READ            I2C3CONbits.RCEN = 1

enum state {
	IDLE,
	RX,
};

static enum state current_state = IDLE;

int main(void)
{
	result_t rc;
	uint8_t  byte_read;
	uint16_t stat = I2C3STAT;
	uint16_t new_stat;

	rc = libesoup_init();

	/*
	 * RE5        3rd pin <-> Pin 7 of EEPROM WP
	 * RE6 / SCL3 2nd pin <-> Pin 6 of EEPROM SCL
	 * RE7 / SDA3 1st pin <-> Pin 5 of EEPROM SDA
	 */

	I2C3ADD = 0x05;
	I2C3MSK = 0x00;  // Couldn't care less

	LOG_D("Entering main loop\n\r");
	while(1) {
//		libesoup_tasks();
		new_stat = I2C3STAT;
		if (stat != new_stat) {
			serial_printf("0x%x > 0x%x\n\r", stat, new_stat);
			stat = new_stat;
		}
		switch (current_state) {
		case IDLE:
			if (I2C3_STARTED) {
				serial_printf("S");
				current_state = RX;
				I2C3_READ;
			}
			break;
		case RX:
			if(I2C3_STOPPED) {
				current_state = IDLE;
				serial_printf("P\n\r");
			}
			if (I2C3_RX_BYTE) {
				byte_read = I2C3RCV;
				serial_printf("0x%x\n\r", byte_read);
			}
			break;
		default:
			if(I2C3_STOPPED) {
				serial_printf("?P\n\r");
			}
			break;
		}
	}
	return(1);
}

#endif // SYS_TEST_24LC64
