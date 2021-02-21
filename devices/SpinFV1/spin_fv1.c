#include "libesoup_config.h"

#define DEBUG_FILE
#define TAG "SpinFV1"

//#define ISR_DRIVEN
#define BIT_BANG

#include "libesoup/logger/serial_log.h"

#ifdef ISR_DRIVEN
#include "libesoup/comms/i2c/i2c.h"
#endif

#if defined(BIT_BANG)
#define SCL_RD     PORTBbits.RB8
#define SCL_INPUT  TRISBbits.TRISB8 = 1; LATBbits.LATB8 = 1;
#define SDA_RD     PORTBbits.RB9
#define SDA_OUTPUT TRISBbits.TRISB9 = 0;
#define SDA_HIGH   TRISBbits.TRISB9 = 0; LATBbits.LATB9 = 1;
#define SDA_LOW    TRISBbits.TRISB9 = 0; LATBbits.LATB9 = 0;
#define SDA_INPUT  TRISBbits.TRISB9 = 1; LATBbits.LATB9 = 1;
#define ACK        TRISBbits.TRISB9 = 0; LATBbits.LATB9 = 0;
#endif

#include "libesoup/gpio/gpio.h"
#include "libesoup/devices/SpinFV1/spin_fv1.h"
#include "libesoup/timers/delay.h"

result_t spin_init_device(struct spin_fv1 *device)
{
	result_t rc;

	LOG_D("spin_init_device()\n\r");

	rc = gpio_set(device->i2c_scl, GPIO_MODE_DIGITAL_INPUT | GPIO_MODE_OPENDRAIN, 1);
	if (rc < 0) {
		LOG_E("Failed to set SCL GPIO Pin\n\r");
		return(rc);
	}

	rc =  delay_mS(5);
	if (rc < 0) {
		LOG_E("Failed to delay\n\r");
		return(rc);
	}

	rc = gpio_set(device->i2c_sda, GPIO_MODE_DIGITAL_INPUT | GPIO_MODE_OPENDRAIN, 1);
	if (rc < 0) {
		LOG_E("Failed to set SDA GPIO Pin\n\r");
		return(rc);
	}

	rc = gpio_set(device->trigger, GPIO_MODE_DIGITAL_OUTPUT, 0);
	if (rc < 0) {
		LOG_E("Failed to set Trigger GPIO Pin\n\r");
	}

	return(rc);
}

#if defined (ISR_DRIVEN)
result_t spin_program(struct spin_fv1 *device, uint8_t *buffer)
{
	struct i2c_device i2c_device;

	spin_fv1.i2c_scl = RB8;
	spin_fv1.i2c_sda = RB9;
	spin_fv1.trigger = RB2;

	i2c_device.channel = I2C1;
	i2c_device.scl_pin = RB8;
	i2c_device.sda_pin = RB9;

	rc = slave_24lcxx(&i2c_device, llrr_buffer, 128 * 4);

//	pulse_T0();


	return(SUCCESS);
}
#endif  // ISR_DRIVEN

#if defined (BIT_BANG)
result_t spin_program(struct spin_fv1 *device, uint8_t *buffer)
{
	result_t rc;
	uint8_t  rx_byte;
	uint8_t  tx_byte;
	uint8_t  master_ack;

	SCL_INPUT;
	SDA_INPUT;

	rc = gpio_set(device->trigger, GPIO_MODE_DIGITAL_OUTPUT, 0);
	if (rc < 0) {
		LOG_E("Failed to set GPIO\n\r");
		return(rc);
	}

	rc = delay_mS(100);
	if (rc < 0) {
		LOG_E("Failed to delay\n\r");
		return(rc);
	}

	rc = gpio_set(device->trigger, GPIO_MODE_DIGITAL_OUTPUT, 1);
	if (rc < 0) {
		LOG_E("Failed to set GPIO\n\r");
		return(rc);
	}

	INTERRUPTS_DISABLED;

	while (SDA_RD && SCL_RD);

	if (!SDA_RD && SCL_RD) {
		/*
		 * STARTED
		 * read First byte
		 */
		rx_byte = 0x00;

		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		ACK;
		while(!SCL_RD);
		while(SCL_RD);
		SDA_INPUT

		// 2nd Byte
		rx_byte = 0x00;

		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		ACK;
		while(!SCL_RD);
		while(SCL_RD);
		SDA_INPUT;

		// 3rd Byte
		rx_byte = 0x00;

		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		ACK;
		while(!SCL_RD);
		while(SCL_RD);
		SDA_INPUT;

		// Need a restart
		while(!(SDA_RD && SCL_RD) );

		// Back to Idle so wait for start
		while(SDA_RD && SCL_RD);

		// Now 4th Byte
		rx_byte = 0x00;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		while(!SCL_RD);
		rx_byte = (rx_byte << 1) | SDA_RD;
		while(SCL_RD);
		ACK;
		while(!SCL_RD);
		while(SCL_RD);
		SDA_INPUT;

		SDA_OUTPUT;

		master_ack = 1;
		while (master_ack) {
			tx_byte = *buffer++;
			SDA_OUTPUT;
			if (tx_byte & 0x80) {
				SDA_HIGH;
			} else {
				SDA_LOW;
			}
			tx_byte = tx_byte << 1;
			while(!SCL_RD);
			while(SCL_RD);
			if (tx_byte & 0x80) {
				SDA_HIGH;
			} else {
				SDA_LOW;
			}
			tx_byte = tx_byte << 1;
			while(!SCL_RD);
			while(SCL_RD);
			if (tx_byte & 0x80) {
				SDA_HIGH;
			} else {
				SDA_LOW;
			}
			tx_byte = tx_byte << 1;
			while(!SCL_RD);
			while(SCL_RD);
			if (tx_byte & 0x80) {
				SDA_HIGH;
			} else {
				SDA_LOW;
			}
			tx_byte = tx_byte << 1;
			while(!SCL_RD);
			while(SCL_RD);
			if (tx_byte & 0x80) {
				SDA_HIGH;
			} else {
				SDA_LOW;
			}
			tx_byte = tx_byte << 1;
			while(!SCL_RD);
			while(SCL_RD);
			if (tx_byte & 0x80) {
				SDA_HIGH;
			} else {
				SDA_LOW;
			}
			tx_byte = tx_byte << 1;
			while(!SCL_RD);
			while(SCL_RD);
			if (tx_byte & 0x80) {
				SDA_HIGH;
			} else {
				SDA_LOW;
			}
			tx_byte = tx_byte << 1;
			while(!SCL_RD);
			while(SCL_RD);
			if (tx_byte & 0x80) {
				SDA_HIGH;
			} else {
				SDA_LOW;
			}
			tx_byte = tx_byte << 1;
			while(!SCL_RD);
			while(SCL_RD);
			SDA_INPUT;
			while(!SCL_RD);
			master_ack = !SDA_RD;
			while(SCL_RD);
		}
		INTERRUPTS_ENABLED;
		return(SUCCESS);
	} else {
		INTERRUPTS_ENABLED;
		LOG_E("scl 0x%x and sda 0x%x\n\r", SCL_RD, SDA_RD);
		return(-ERR_IM_A_TEAPOT);
	}
}
#endif // BIT_BANG
