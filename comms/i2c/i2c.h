/**
 * @file libesoup/comms/i2c/i2c.h
 *
 * @author John Whitmore
 * 
 * @brief  API Interface to I2C. Initial PIC24FJ256GB106
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
#if defined (SYS_I2C1) || defined (SYS_I2C2) || defined (SYS_I2C3)

extern result_t i2c_start(enum i2c_channel chan, void (*callback)(result_t));
extern result_t i2c_restart(enum i2c_channel chan, void (*callback)(result_t));
extern result_t i2c_stop(enum i2c_channel chan);

extern result_t i2c_write(enum i2c_channel chan, uint8_t *tx_buf, uint8_t num_tx_bytes, void (*callback)(result_t));
extern result_t i2c_read(enum i2c_channel chan, uint8_t *rx_buf, uint8_t num_rx_bytes, void (*callback)(result_t));

#endif // SYS_I2C
