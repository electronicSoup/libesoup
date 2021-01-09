/**
 * @file libesoup/comms/i2c/i2c.h
 *
 * @author John Whitmore
 * 
 * @brief  API Interface to I2C.
 *
 * Copyright 2020 electronicSoup Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU Lesser General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************
 *
 */
#ifndef I2C_H_
#define I2C_H_

#include "libesoup_config.h"

#if defined (SYS_I2C_1) || defined (SYS_I2C_2) || defined (SYS_I2C_3)


enum state {
        IDLE_STATE,
        STARTING_STATE,
        STARTED_STATE,
        TX_STATE,
        RESTARTING_STATE,
        RX_STATE,
	RX_DONE_STATE,
        STOPPING_STATE
};

struct i2c_device {
        enum i2c_chan_id  channel;
        enum gpio_pin     scl_pin;
        enum gpio_pin     sda_pin;
        void            (*callback)(result_t);
};

struct i2c_channel_data {
        enum i2c_chan_id   channel;
	uint8_t            active;
	struct i2c_device *active_device;
	void             (*state)(enum i2c_chan_id, uint16_t);
	result_t           error;
	uint8_t            finished;
	uint8_t            sent;
	uint8_t           *tx_buf;
	uint8_t            num_tx_bytes;
	uint8_t           *rx_buf;
	uint16_t           num_rx_bytes;
	uint8_t            read_count;
	void             (*callback)(result_t);
	void             (*read_callback)(result_t, uint8_t);
};

extern result_t i2c_reserve(struct i2c_device *device);
extern result_t i2c_release(struct i2c_device *device);

extern result_t i2c_start(struct i2c_device *device);
extern result_t i2c_restart(struct i2c_device *device);
extern result_t i2c_stop(struct i2c_device *device);

extern result_t i2c_write(struct i2c_device *device, uint8_t *tx_buf, uint8_t num_tx_bytes, void (*callback)(result_t));
extern result_t i2c_read(struct i2c_device *device, uint8_t *rx_buf, uint16_t num_rx_bytes, void (*callback)(result_t));

#endif // SYS_I2C

#endif //  _I2C_H_
