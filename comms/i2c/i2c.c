#include "libesoup_config.h"

#if defined(SYS_I2C1) || defined(SYS_I2C2) || defined(SYS_I2C3)

#define DEBUG_FILE
#define TAG "I2C"
#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/i2c/i2c.h"

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

result_t i2c_init(void)
{
	enum i2c_channel i;
	LOG_D("i2c_init()\n\r");

	for(i = 0; i < NUM_I2C_CHANNELS; i++) {
		i2c_channels[i].channel       = i;
		i2c_channels[i].active        = FALSE;
		i2c_channels[1].active_device = NULL;
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

	if (!i2c_channels[device->channel].active) {
		i2c_channels[device->channel].active        = TRUE;
		i2c_channels[device->channel].active_device = device;

		rc = i2c_py_reserve(device);
		if (rc < 0) {
			/*
			 * Roll back the reservation;
			 */
			i2c_channels[device->channel].active        = FALSE;
			i2c_channels[device->channel].active_device = NULL;

			return(rc);
		}

		i2c_channels[device->channel].state         = IDLE_STATE;
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

#endif // SYS_I2C1 || SYS_I2C2 || SYS_I2C3
