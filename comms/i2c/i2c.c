/**
 * @file libesoup/comms/i2c/i2c.c
 *
 * @author John Whitmore
 *
 * @brief  Implementaiton of the I2C protocol. Initial PIC24FJ256GB106
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
#include "libesoup_config.h"

#define DEBUG_FILE
#define TAG "I2C"

#include "libesoup/logger/serial_log.h"

#if defined(SYS_I2C1) || defined(SYS_I2C2) || defined(SYS_I2C3)

static result_t error = SUCCESS;
static uint8_t  finished = 0;
static uint8_t  sent;
static uint8_t *tx_buf;
static uint8_t  num_tx_bytes;
static uint8_t *rx_buf;
static uint16_t num_rx_bytes;
static uint8_t  read_count;

static void (*i2c3_callback)(result_t) = NULL;
static void (*i2c3_read_callback)(result_t, uint8_t) = NULL;

/*
 * Prototype
 */
static void i2c3_send_next();

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

static enum state current_state;

#define I2C3_STARTED         I2C3STATbits.S
#define I2C3_STOPPED         I2C3STATbits.P
#define I2C3_BYTE_READ       I2C3STATbits.RBF
#define I2C3_TRANSMITTING    I2C3STATbits.TBF
#define I2C3_WRITE_COLLISION I2C3STATbits.IWCOL

#define I2C3_STOP            I2C3CONbits.PEN = 1
#define I2C3_RESTART         I2C3CONbits.RSEN = 1
#define I2C3_READ            I2C3CONbits.RCEN = 1
#define I2C3_RX_ACK          I2C3CONbits.ACKDT = 0; I2C3CONbits.ACKEN =1
#define I2C3_RX_NACK         I2C3CONbits.ACKDT = 1; I2C3CONbits.ACKEN =1

void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C3Interrupt(void)
{
	uint8_t  rx_char;

	serial_printf("*%d*\n\r", current_state);
	while (IFS5bits.MI2C3IF) {
		IFS5bits.MI2C3IF    = 0;  // Clear Master ISR Flag
		serial_printf("+0x%x", I2C3STAT);

                if (I2C3_WRITE_COLLISION) {
                        LOG_E("IWCOL\n\r");
                        error = -ERR_NOT_READY;
                }

                switch(current_state) {
		case IDLE_STATE:
			LOG_D("Idle State???\n\r");
			break;
		case STARTING_STATE:
			if (I2C3_STARTED) {
				current_state = STARTED_STATE;
				LOG_D("Started\n\r");
				if (i2c3_callback) {
					i2c3_callback(SUCCESS);
				}
			} else {
				LOG_E("Failed to start\n\r");
				error = ERR_BAD_STATE;
				I2C3_STOP;
				current_state = STOPPING_STATE;
			}
			break;
		case STARTED_STATE:
			LOG_D("Started ???\n\r");
			break;
		case TX_STATE:
			if(I2C3STATbits.ACKSTAT) {
				serial_printf("NACK\n\r");
				error = -ERR_NO_RESPONSE;
				I2C3_STOP;
				current_state = STOPPING_STATE;
			} else {
				serial_printf("ACK");
				i2c3_send_next();
			}
			break;
		case RESTARTING_STATE:
			if (I2C3_STARTED) {
				current_state = STARTED_STATE;
				if (i2c3_callback) {
					i2c3_callback(SUCCESS);
				}
			} else {
				LOG_E("Failed to restart\n\r");
				error = ERR_BAD_STATE;
				I2C3_STOP;
//				next_state = STOPPING_STATE;
				current_state = STOPPING_STATE;
			}
			break;
		case RX_STATE:
			if (I2C3_BYTE_READ) {
				rx_char = I2C3RCV;
				*rx_buf++ = rx_char;
				read_count++;
				if(read_count % 10 == 0) {
					serial_printf("0");
				}

//				LOG_D("%d:%x", read_count, rx_char);
				if (read_count >= num_rx_bytes) {
					I2C3_RX_NACK;
					LOG_D("Finished\n\r");
					finished = 1;
					current_state = RX_DONE_STATE;
				} else {
					I2C3_RX_ACK;
					I2C3_READ;
				}
			} else {
				I2C3_READ;
			}
			break;
		case RX_DONE_STATE:
			LOG_D("RX_DONE_STATE\n\r")
			break;
		case STOPPING_STATE:
			if (I2C3_STOPPED) {
				current_state = IDLE_STATE;
			} else {
				serial_printf("Failed to stop\n\r");
				error = -ERR_IM_A_TEAPOT;
			}
			break;
		default:
			LOG_D("Unknown State ???");
			break;
                }
	}
}

void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C3Interrupt(void)
{
	serial_printf("IS");
}

result_t i2c_init(enum i2c_channel chan)
{
	LOG_D("i2c_init()\n\r");
        switch (chan) {
                case I2C1:
                        LOG_E("Not implemented\n\r");
                        return(-ERR_NOT_CODED);
                        break;
                case I2C2:
                        LOG_E("Not implemented\n\r");
                        return(-ERR_NOT_CODED);
                        break;
                case I2C3:
                        I2C3BRG             = 0x0d;

                        I2C3CONbits.I2CSIDL = 0;  // Module continues in Idle.
                        I2C3CONbits.SCLREL  = 1;  // Release clock (Slave mode))
                        I2C3CONbits.IPMIEN  = 0;  // Disable IPMI mode
                        I2C3CONbits.A10M    = 0;  // 7 bit mode
                        I2C3CONbits.DISSLW  = 1;  // Disable Slew rate.
                        I2C3CONbits.SMEN    = 0;  // Disable SMBus thresholds
                        I2C3CONbits.GCEN    = 0;  // Disable General call address
                        I2C3CONbits.STREN   = 0;  // Disable clock stretching
                        I2C3CONbits.ACKDT   = 0;  // Send /ACK to acknowledge

                        IFS5bits.MI2C3IF    = 0;  // Clear Master ISR Flag
                        IFS5bits.SI2C3IF    = 0;  // Clear Slave ISR Flag
                        IEC5bits.MI2C3IE    = 1;  // Enable Master Interrupts
                        IEC5bits.SI2C3IE    = 1;  // Enable Slave Interrupts

                        I2C3CONbits.I2CEN   = 1;
//                        I2C3CONbits.PEN     = 1;  // Send Stop
                        current_state       = IDLE_STATE;
                        break;
                default:
                        LOG_E("No such I2C\n\r");
                        return (-ERR_BAD_INPUT_PARAMETER);
                        break;
        }
        return (SUCCESS);
}

result_t i2c_tasks(void)
{
        result_t rc;

        rc = error;

        if(error < 0) {
		LOG_E("i2c_tasks() Error\n\r")
                if(i2c3_callback) {
                        i2c3_callback(error);
                        error = SUCCESS;
                }
        }

	if(finished) {
		i2c3_read_callback(SUCCESS, read_count);
		finished           = 0;
		i2c3_callback      = NULL;
		i2c3_read_callback = NULL;
	}
        return(rc);
}

result_t i2c_start(enum i2c_channel chan, void (*p_callback)(result_t))
{
//	static uint8_t count = 0;

	LOG_D("Start\n\r");
        if (chan == I2C3) {
                i2c3_callback = p_callback;

//		if (count == 0) {
//			count++;
//			LOG_D("Starting\n\r");
  //                      I2C3CONbits.SEN = 1;
//			return(SUCCESS);
//		}

                if (I2C3STATbits.P && (current_state == IDLE_STATE)) {
			LOG_D("Starting\n\r");
                        current_state = STARTING_STATE;
                        I2C3CONbits.SEN = 1;
                } else {
			if (!I2C3_STOPPED) {
				LOG_E("Not stopped\n\r");
			}
			if (current_state != IDLE_STATE) {
				LOG_E("Bad state %d\n\r", current_state);
			}
                        return (-ERR_BAD_STATE);
                }
        }

        return(SUCCESS);
}

result_t i2c_restart(enum i2c_channel chan, void (*p_callback)(result_t))
{
	LOG_D("Restart\n\r");
        if (chan == I2C3) {
                i2c3_callback = p_callback;
		if (I2C3_STARTED) {
			current_state = RESTARTING_STATE;
			I2C3_RESTART;
			return(SUCCESS);
		} else {
			LOG_E("Bad State\n\r");
			return (-ERR_BAD_STATE);
		}
        }

        return(SUCCESS);
}

result_t i2c_stop(enum i2c_channel chan)
{
	LOG_D("Stop\n\r");
	current_state = STOPPING_STATE;
        if (chan == I2C3) {
		I2C3_STOP;
        }

        return(SUCCESS);
}

result_t i2c_write(enum i2c_channel chan, uint8_t *p_tx_buf, uint8_t p_num_tx_bytes, void (*p_callback)(result_t))
{
	i2c3_callback      = p_callback;
	LOG_D("i2c_write()\n\r");
	if (current_state == STARTED_STATE) {
		sent          = 0;
		tx_buf        = p_tx_buf;
		num_tx_bytes  = p_num_tx_bytes;

		i2c3_send_next();

		return(SUCCESS);
	} else {
		LOG_E("Bad State %d\n\r", current_state);
		return(-ERR_BAD_STATE);
	}
}

result_t i2c_read(enum i2c_channel chan, uint8_t *p_rx_buf, uint16_t p_num_rx_bytes, void (*p_callback)(result_t, uint8_t))
{
	LOG_D("i2c_read()\n\r");
	i2c3_read_callback   = p_callback;
	rx_buf               = p_rx_buf;
	num_rx_bytes         = p_num_rx_bytes;
	current_state        = RX_STATE;
	read_count           = 0;
	I2C3_READ;
        return(SUCCESS);
}

static void i2c3_send_next(void)
{
	if (sent < num_tx_bytes) {
		current_state = TX_STATE;
		I2C3TRN = *tx_buf++;
		sent++;
	} else {
		i2c3_callback(SUCCESS);
	}
}

#endif // _SYS_I2C1 || SYS_I2S2 || SYS_I2S3
