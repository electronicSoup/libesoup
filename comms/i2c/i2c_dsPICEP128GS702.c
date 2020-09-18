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
#if defined(__dsPIC33EP128GS702__) && (defined(SYS_I2C1) || defined(SYS_I2C2))

#define DEBUG_FILE
#define TAG "I2C"

#include "libesoup/logger/serial_log.h"


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

struct i2c_channel_data {
	enum i2c_channel chan;
	enum state       state;
	result_t         error;
	uint8_t          finished;
	uint8_t          sent;
	uint8_t         *tx_buf;
	uint8_t          num_tx_bytes;
	uint8_t         *rx_buf;
	uint16_t         num_rx_bytes;
	uint8_t          read_count;
	void           (*callback)(result_t);
	void           (*read_callback)(result_t, uint8_t);
};

struct i2c_channel_data i2c_channels[NUM_I2C_CHANNELS];


/*
 * Prototype
 */
static void i2c3_send_next(enum i2c_channel);



#if defined(SYS_I2C1)
#define I2C1_STARTED         I2C1STATbits.S
#define I2C1_STOPPED         I2C1STATbits.P
#define I2C1_BYTE_READ       I2C1STATbits.RBF
#define I2C1_TRANSMITTING    I2C1STATbits.TBF
#define I2C1_WRITE_COLLISION I2C1STATbits.IWCOL
#define I2C1_ACKSTAT         I2C1STATbits.ACKSTAT

#define I2C1_START           I2C1CONLbits.SEN   = 1
#define I2C1_STOP            I2C1CONLbits.PEN   = 1
#define I2C1_RESTART         I2C1CONLbits.RSEN  = 1
#define I2C1_READ            I2C1CONLbits.RCEN  = 1
#define I2C1_RX_ACK          I2C1CONLbits.ACKDT = 0; I2C1CONLbits.ACKEN =1
#define I2C1_RX_NACK         I2C1CONLbits.ACKDT = 1; I2C1CONLbits.ACKEN =1
#endif

#if defined(SYS_I2C2)
#define I2C2_STARTED         I2C2STATbits.S
#define I2C2_STOPPED         I2C2STATbits.P
#define I2C2_BYTE_READ       I2C2STATbits.RBF
#define I2C2_TRANSMITTING    I2C2STATbits.TBF
#define I2C2_WRITE_COLLISION I2C2STATbits.IWCOL
#define I2C2_ACKSTAT         I2C2STATbits.ACKSTAT

#define I2C2_START           I2C2CONLbits.SEN   = 1
#define I2C2_STOP            I2C2CONbits.PEN    = 1
#define I2C2_RESTART         I2C2CONbits.RSEN   = 1
#define I2C2_READ            I2C2CONbits.RCEN   = 1
#define I2C2_RX_ACK          I2C2CONbits.ACKDT  = 0; I2C2CONbits.ACKEN =1
#define I2C2_RX_NACK         I2C2CONbits.ACKDT  = 1; I2C2CONbits.ACKEN =1
#endif // defined(SYS_I2C2)

#ifdef SYS_I2C1
void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C1Interrupt(void)
{
	uint8_t  rx_char;

//	serial_printf("*%d*\n\r", current_state);
	while (IFS1bits.MI2C1IF) {
		IFS1bits.MI2C1IF    = 0;  // Clear Master ISR Flag
//		serial_printf("+0x%x", I2C3STAT);

                if (I2C1_WRITE_COLLISION) {
                        LOG_E("IWCOL\n\r");
                        i2c_channels[I2C1].error = -ERR_NOT_READY;
                }

                switch(i2c_channels[I2C1].state) {
		case IDLE_STATE:
			LOG_D("Idle State???\n\r");
			break;
		case STARTING_STATE:
			if (I2C1_STARTED) {
				i2c_channels[I2C1].state = STARTED_STATE;
				LOG_D("Started\n\r");
				if (i2c_channels[I2C1].callback) {
					i2c_channels[I2C1].callback(SUCCESS);
				}
			} else {
				LOG_E("Failed to start\n\r");
				i2c_channels[I2C1].error = ERR_BAD_STATE;
				I2C1_STOP;
				i2c_channels[I2C1].state = STOPPING_STATE;
			}
			break;
		case STARTED_STATE:
			LOG_D("Started ???\n\r");
			break;
		case TX_STATE:
			if(I2C1_ACKSTAT) {
				serial_printf("NACK\n\r");
				i2c_channels[I2C1].error = -ERR_NO_RESPONSE;
				I2C1_STOP;
				i2c_channels[I2C1].state = STOPPING_STATE;
			} else {
				serial_printf("ACK");
				i2c3_send_next(I2C1);
			}
			break;
		case RESTARTING_STATE:
			if (I2C1_STARTED) {
				i2c_channels[I2C1].state = STARTED_STATE;
				if (i2c_channels[I2C1].callback) {
					i2c_channels[I2C1].callback(SUCCESS);
				}
			} else {
				LOG_E("Failed to restart\n\r");
				i2c_channels[I2C1].error = ERR_BAD_STATE;
				I2C1_STOP;
//				next_state = STOPPING_STATE;
				i2c_channels[I2C1].state = STOPPING_STATE;
			}
			break;
		case RX_STATE:
			if (I2C1_BYTE_READ) {
				rx_char = I2C1RCV;
				*i2c_channels[I2C1].rx_buf++ = rx_char;
				i2c_channels[I2C1].read_count++;
				if(i2c_channels[I2C1].read_count % 10 == 0) {
					serial_printf("0");
				}

//				LOG_D("%d:%x", read_count, rx_char);
				if (i2c_channels[I2C1].read_count >= i2c_channels[I2C1].num_rx_bytes) {
					I2C1_RX_NACK;
					LOG_D("Finished\n\r");
					i2c_channels[I2C1].finished = 1;
					i2c_channels[I2C1].state = RX_DONE_STATE;
				} else {
					I2C1_RX_ACK;
					I2C1_READ;
				}
			} else {
				I2C1_READ;
			}
			break;
		case RX_DONE_STATE:
			LOG_D("RX_DONE_STATE\n\r")
			break;
		case STOPPING_STATE:
			if (I2C1_STOPPED) {
				i2c_channels[I2C1].state = IDLE_STATE;
			} else {
				serial_printf("Failed to stop\n\r");
				i2c_channels[I2C1].error = -ERR_IM_A_TEAPOT;
			}
			break;
		default:
			LOG_D("Unknown State ???");
			break;
                }
	}
}
#endif

#ifdef SYS_I2C2
void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C2Interrupt(void)
{
}
#endif // SYS_I2C2

#ifdef SYS_I2C1
void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C1Interrupt(void)
{
	serial_printf("iS");
}
#endif

#ifdef SYS_I2C2
void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C2Interrupt(void)
{
	serial_printf("S");
}
#endif

result_t i2c_init()
{
	uint8_t chan;

	LOG_D("i2c_init()\n\r");
	for ( chan = 0; chan < NUM_I2C_CHANNELS; chan++) {
		switch (chan) {
#if defined(SYS_I2C1)
		case I2C1:
			i2c_channels[I2C1].chan          = chan;
			i2c_channels[I2C1].state         = IDLE_STATE;
			i2c_channels[I2C1].error         = SUCCESS;
			i2c_channels[I2C1].finished      = 0;
			i2c_channels[I2C1].sent          = 0;
			i2c_channels[I2C1].tx_buf        = NULL;
			i2c_channels[I2C1].num_tx_bytes  = 0;
			i2c_channels[I2C1].rx_buf        = NULL;
			i2c_channels[I2C1].num_rx_bytes  = 0;
			i2c_channels[I2C1].read_count    = 0;
			i2c_channels[I2C1].callback      = NULL;
			i2c_channels[I2C1].read_callback = NULL;

                        I2C1BRG              = 0x0d;

                        I2C1CONLbits.I2CSIDL = 0;  // Module continues in Idle.
                        I2C1CONLbits.SCLREL  = 1;  // Release clock (Slave mode))
                        I2C1CONLbits.A10M    = 0;  // 7 bit mode
                        I2C1CONLbits.DISSLW  = 1;  // Disable Slew rate.
                        I2C1CONLbits.SMEN    = 0;  // Disable SMBus thresholds
                        I2C1CONLbits.GCEN    = 0;  // Disable General call address
                        I2C1CONLbits.STREN   = 0;  // Disable clock stretching
                        I2C1CONLbits.ACKDT   = 0;  // Send /ACK to acknowledge

			IFS10bits.I2C1BCIF   = 0;
                        IFS1bits.MI2C1IF     = 0;  // Clear Master ISR Flag
                        IFS1bits.SI2C1IF     = 0;  // Clear Slave ISR Flag
                        IEC1bits.MI2C1IE     = 1;  // Enable Master Interrupts
                        IEC1bits.SI2C1IE     = 1;  // Enable Slave Interrupts

                        I2C1CONLbits.I2CEN   = 1;
                        I2C1CONLbits.PEN     = 1;  // Send Stop
                        break;
#endif
#if defined(SYS_I2C2)
		case I2C2:
			i2c_channels[I2C2].chan          = chan;
			i2c_channels[I2C2].state         = IDLE_STATE;
			i2c_channels[I2C2].error         = SUCCESS;
			i2c_channels[I2C2].finished      = 0;
			i2c_channels[I2C2].sent          = 0;
			i2c_channels[I2C2].tx_buf        = NULL;
			i2c_channels[I2C2].num_tx_bytes  = 0;
			i2c_channels[I2C2].rx_buf        = NULL;
			i2c_channels[I2C2].num_rx_bytes  = 0;
			i2c_channels[I2C2].read_count    = 0;
			i2c_channels[I2C2].callback      = NULL;
			i2c_channels[I2C2].read_callback = NULL;

                        I2C1BRG              = 0x0d;

                        I2C2CONLbits.I2CSIDL = 0;  // Module continues in Idle.
                        I2C2CONLbits.SCLREL  = 1;  // Release clock (Slave mode))
                        I2C2CONLbits.A10M    = 0;  // 7 bit mode
                        I2C2CONLbits.DISSLW  = 1;  // Disable Slew rate.
                        I2C2CONLbits.SMEN    = 0;  // Disable SMBus thresholds
                        I2C2CONLbits.GCEN    = 0;  // Disable General call address
                        I2C2CONLbits.STREN   = 0;  // Disable clock stretching
                        I2C2CONLbits.ACKDT   = 0;  // Send /ACK to acknowledge

			IFS10bits.I2C2BCIF   = 0;
                        IFS3bits.MI2C2IF     = 0;  // Clear Master ISR Flag
                        IFS3bits.SI2C2IF     = 0;  // Clear Slave ISR Flag
                        IEC3bits.MI2C2IE     = 1;  // Enable Master Interrupts
                        IEC3bits.SI2C2IE     = 1;  // Enable Slave Interrupts

                        I2C2CONLbits.I2CEN   = 1;
                        I2C2CONLbits.PEN     = 1;  // Send Stop
                        break;
#endif
		default:
                        LOG_E("No such I2C\n\r");
                        return (-ERR_BAD_INPUT_PARAMETER);
                        break;
		}
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

result_t i2c_start(enum i2c_channel chan, void (*p_callback)(result_t))
{
//	static uint8_t count = 0;

	LOG_D("Start\n\r");
	switch(chan) {
#if defined (SYS_I2C1)
	case I2C1:
                i2c_channels[chan].callback = p_callback;

//		if (count == 0) {
//			count++;
//			LOG_D("Starting\n\r");
  //                      I2C3CONbits.SEN = 1;
//			return(SUCCESS);
//		}

                if (I2C1_STOPPED && (i2c_channels[chan].state == IDLE_STATE)) {
			LOG_D("Starting\n\r");
                        i2c_channels[chan].state = STARTING_STATE;
                        I2C1_START;
                } else {
			if (!I2C1_STOPPED) {
				LOG_E("Not stopped\n\r");
			}
			if (i2c_channels[chan].state != IDLE_STATE) {
				LOG_E("Bad state %d\n\r", i2c_channels[chan].state);
			}
                        return (-ERR_BAD_STATE);
                }
		break;
#endif
#if defined (SYS_I2C2)
	case I2C2:
                i2c_channels[chan].callback = p_callback;

//		if (count == 0) {
//			count++;
//			LOG_D("Starting\n\r");
  //                      I2C3CONbits.SEN = 1;
//			return(SUCCESS);
//		}

                if (I2C2_STOPPED && (i2c_channels[chan].state == IDLE_STATE)) {
			LOG_D("Starting\n\r");
                        i2c_channels[chan].state = STARTING_STATE;
                        I2C2_START;
                } else {
			if (!I2C1_STOPPED) {
				LOG_E("Not stopped\n\r");
			}
			if (i2c_channels[chan].state != IDLE_STATE) {
				LOG_E("Bad state %d\n\r", i2c_channels[chan].state);
			}
                        return (-ERR_BAD_STATE);
                }
		break;
#endif
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
	case I2C3:
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
		break;
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
	default:
		return (-ERR_BAD_INPUT_PARAMETER);
		break;
	}

        return(SUCCESS);
}

result_t i2c_restart(enum i2c_channel chan, void (*p_callback)(result_t))
{
	LOG_D("Restart\n\r");
#if defined(SYS_I2C3)
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
#endif // defined(SYS_I2C3)
        return(SUCCESS);
}

result_t i2c_stop(enum i2c_channel chan)
{
	LOG_D("Stop\n\r");
#if defined(SYS_I2C3)
	current_state = STOPPING_STATE;
        if (chan == I2C3) {
		I2C3_STOP;
        }
#endif // defined(SYS_I2C3)
        return(SUCCESS);
}

result_t i2c_write(enum i2c_channel chan, uint8_t *p_tx_buf, uint8_t p_num_tx_bytes, void (*p_callback)(result_t))
{
	LOG_D("i2c_write()\n\r");
#if defined(SYS_I2C3)
	i2c3_callback      = p_callback;
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
#endif // defined(SYS_I2C3)
	return(SUCCESS);
}

result_t i2c_read(enum i2c_channel chan, uint8_t *p_rx_buf, uint16_t p_num_rx_bytes, void (*p_callback)(result_t, uint8_t))
{
	LOG_D("i2c_read()\n\r");
#if defined(SYS_I2C3)
	i2c3_read_callback   = p_callback;
	rx_buf               = p_rx_buf;
	num_rx_bytes         = p_num_rx_bytes;
	current_state        = RX_STATE;
	read_count           = 0;
	I2C3_READ;
#endif // #if defined(SYS_I2C3)
        return(SUCCESS);
}

static void i2c3_send_next(enum i2c_channel chan)
{
	if (i2c_channels[chan].sent < i2c_channels[chan].num_tx_bytes) {
		i2c_channels[chan].state = TX_STATE;
		switch(chan) {
#if defined(SYS_I2C1)
		case I2C1:
			I2C1TRN = *i2c_channels[chan].tx_buf++;
			break;
#endif
#if defined (SYS_I2C2)
		case I2C2:
			I2C2TRN = *i2c_channels[chan].tx_buf++;
			break;
#endif
		default:
			LOG_E("Bad input\n\r");
		}
		i2c_channels[chan].sent++;
	} else {
		i2c_channels[chan].callback(SUCCESS);
	}
}

#endif // _SYS_I2C1 || SYS_I2S2 || SYS_I2S3
