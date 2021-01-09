#include "libesoup_config.h"

#if defined(SYS_I2C_1) || defined(SYS_I2C_2) || defined(SYS_I2C_3)

#define DEBUG_FILE
#define TAG "I2C"
#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/i2c/i2c.h"


#define STAT_START_BIT     0x0008
#define STAT_OVERFLOW_BIT  0x0040
#define STAT_RBF_BIT       0x0002
#define STAT_STOP_BIT      0x0010

struct i2c_channel_data i2c_channels[NUM_I2C_CHANNELS];

/*
 * Implemented by device specific I2C Code
 */
extern result_t i2c_py_reserve(struct i2c_device *device);
extern result_t i2c_py_release(struct i2c_device *device);
extern result_t i2c_py_start(struct i2c_device *device);
extern result_t i2c_py_restart(struct i2c_device *device);
extern result_t i2c_py_stop(struct i2c_device *device);
extern result_t i2c_py_write(struct i2c_device *device);
extern result_t i2c_py_read(struct i2c_device *device);

/*
 * State Funcitons
 */
void    idle(enum i2c_chan_id channel, uint16_t stat_reg);
void started(enum i2c_chan_id channel, uint16_t stat_reg);

result_t i2c_init(void)
{
	enum i2c_chan_id i;
	LOG_D("i2c_init()\n\r");

	for(i = 0; i < NUM_I2C_CHANNELS; i++) {
		i2c_channels[i].channel       = i;
		i2c_channels[i].active        = FALSE;
		i2c_channels[1].active_device = NULL;
		i2c_channels[i].state         = idle;
	}
	return (SUCCESS);
}

result_t i2c_tasks(void)
{
	uint8_t  i;
        result_t rc = SUCCESS;

	for (i = 0; i < NUM_I2C_CHANNELS; i++) {
		rc = i2c_channels[i].error;

		if(i2c_channels[i].error < 0) {
			LOG_E("i2c_tasks(%d) Error\n\r", i)
			if(i2c_channels[i].callback) {
				i2c_channels[i].callback(i2c_channels[i].error);
				/*
				 * Clear the error condition
				 */
				i2c_channels[i].error = SUCCESS;
			}
		}

		if(i2c_channels[i].finished) {
			i2c_channels[i].read_callback(SUCCESS, i2c_channels[i].read_count);
			i2c_channels[i].finished        = 0;
			i2c_channels[i].callback        = NULL;
			i2c_channels[i].read_callback   = NULL;
		}
	}
        return(rc);
}

result_t i2c_reserve(struct i2c_device *device)
{
	result_t rc;

	LOG_D("i2c_reserve(%d)\n\r", device->channel)
	if (!i2c_channels[device->channel].active) {
		i2c_channels[device->channel].active        = TRUE;
		i2c_channels[device->channel].active_device = device;
		i2c_channels[device->channel].state         = idle;

		rc = i2c_py_reserve(device);
		if (rc < 0) {
			/*
			 * Roll back the reservation;
			 */
			i2c_channels[device->channel].active        = FALSE;
			i2c_channels[device->channel].active_device = NULL;

			return(rc);
		}

		i2c_channels[device->channel].error         = SUCCESS;
		i2c_channels[device->channel].finished      = 0;
		i2c_channels[device->channel].sent          = 0;
		i2c_channels[device->channel].tx_buf        = NULL;
		i2c_channels[device->channel].num_tx_bytes  = 0;
		i2c_channels[device->channel].rx_buf        = NULL;
		i2c_channels[device->channel].num_rx_bytes  = 0;
		i2c_channels[device->channel].read_count    = 0;
		i2c_channels[device->channel].callback      = NULL;
		i2c_channels[device->channel].read_callback = NULL;

		return(SUCCESS);
	} else {
		return(-ERR_BUSY);
	}
}

result_t i2c_release(struct i2c_device *device)
{
	if (i2c_channels[device->channel].active) {
		i2c_channels[device->channel].active        = FALSE;
		i2c_channels[device->channel].active_device = NULL;
		return(i2c_py_release(device));
	} else {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
}

result_t i2c_start(struct i2c_device *device)
{
	LOG_D("i2c_start()\n\r");
	return(i2c_py_start(device));
}

result_t i2c_restart(struct i2c_device *device)
{
	LOG_D("i2c_restart()\n\r");
	return(i2c_py_restart(device));
}

result_t i2c_stop(struct i2c_device *device)
{
	LOG_D("i2c_stop()\n\r");
	return(i2c_py_stop(device));
}

result_t i2c_write(struct i2c_device *device, uint8_t *tx_buf, uint8_t num_tx_bytes, void (*callback)(result_t))
{
	LOG_D("i2c_write()\n\r");
	return(i2c_py_write(device));
}

result_t i2c_read(struct i2c_device *device, uint8_t *rx_buf, uint16_t num_rx_bytes, void (*callback)(result_t))
{
	LOG_D("i2c_read()\n\r");
	return(i2c_py_read(device));
}

/*
 * State Funcitons
 */
void idle(enum i2c_chan_id channel, uint16_t stat_reg)
{
	serial_printf("I");

	if (stat_reg & STAT_START_BIT) {
		serial_printf("s");
		i2c_channels[channel].state = started;
	}
	if (stat_reg & STAT_OVERFLOW_BIT) {
		switch (channel) {
#ifdef SYS_I2C1
		case I2C1:
			I2C1STATbits.I2COV = 0;
			serial_printf("Ov");
			break;
#endif // SYS_I2C1
		default:
			LOG_E("Bad I2C Channel\n\r");
			break;
		}
	}
}

void started(enum i2c_chan_id channel, uint16_t stat_reg)
{
//	uint8_t rx_byte;

	serial_printf("S");
	if (stat_reg & STAT_RBF_BIT) {
		serial_printf("r");
		switch (channel) {
#ifdef SYS_I2C1
		case I2C1:
			rx_byte = I2C1RCV;
			serial_printf("0x%x\n\r", rx_byte);
			break;
#endif // SYS_I2C1
		default:
			LOG_E("Bad I2C Channel\n\r");
			break;
		}
	}

	if (stat_reg & STAT_STOP_BIT) {
		serial_printf("E");
		i2c_channels[channel].state = idle;
	}

	if (stat_reg & STAT_OVERFLOW_BIT) {
		switch (channel) {
#ifdef SYS_I2C1
		case I2C1:
			I2C1STATbits.I2COV = 0;
			serial_printf("Ov");
			break;
#endif // SYS_I2C1
		default:
			LOG_E("Bad I2C Channel\n\r");
			break;
		}
	}
}

#endif // SYS_I2C1 || SYS_I2C2 || SYS_I2C3
