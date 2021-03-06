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
#if defined(__dsPIC33EP256GP502__) && (defined(SYS_I2C_1) || defined(SYS_I2C_2))

#define DEBUG_FILE
#define TAG "dsPIC33EP256GP502-I2C"

#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/i2c/i2c.h"
#include "libesoup/timers/delay.h"

extern struct i2c_channel_data i2c_channels[NUM_I2C_CHANNELS];


/*
 * Prototype
 */

#if defined(SYS_I2C_1)
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

#if defined(SYS_I2C_2)
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
#endif // defined(SYS_I2C_2)

#ifdef SYS_I2C_1
void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C1Interrupt(void)
{
//	uint8_t  rx_char;

	serial_printf("M*\n\r");
//	serial_printf("*%d*\n\r", current_state);
#if 0
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
#endif // 0
}
#endif

#ifdef SYS_I2C_2
void __attribute__((__interrupt__, __no_auto_psv__)) _MI2C2Interrupt(void)
{
}
#endif // SYS_I2C_2

#ifdef SYS_I2C_1
void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C1Interrupt(void)
{
//	uint16_t stat_reg;

	gpio_set(RB13, GPIO_MODE_DIGITAL_OUTPUT, 0);

	serial_printf("S1* 0x%x\n\r", I2C1ADD);
	while(1) {
		Nop();
	}
#if 0
	while (IFS1bits.SI2C1IF || I2C1STATbits.RBF) {
		IFS1bits.SI2C1IF    = 0;  // Clear Master ISR Flag
		stat_reg = I2C1STAT;
		serial_printf("0x%x", stat_reg);
		i2c_channels[I2C1].state(I2C1, stat_reg);
	}
	serial_printf(" O\n\r");
#endif // 0
}
#endif

#ifdef SYS_I2C_2
void __attribute__((__interrupt__, __no_auto_psv__)) _SI2C2Interrupt(void)
{
	serial_printf("S");
}
#endif

#if 0
result_t i2c_py_reserve(struct i2c_device *device)
{
	uint8_t chan;

	LOG_D("i2c_py_reserve(%d)\n\r", device->channel);
	chan = device->channel;

	switch (chan) {
#if defined(SYS_I2C_1)
	case I2C1:
		/*
		 * uC has fixed pins RB8 & RB9
		 */
		if ((device->sda_pin != RB9) || (device->scl_pin != RB8)) {
			return(ERR_BAD_INPUT_PARAMETER);
		}

		gpio_set(device->sda_pin, GPIO_MODE_DIGITAL_INPUT | GPIO_MODE_OPENDRAIN, 0);
		gpio_set(device->scl_pin, GPIO_MODE_DIGITAL_INPUT | GPIO_MODE_OPENDRAIN, 0);

#if 0
		I2C1BRG              = 0x0d;

		I2C1CONLbits.I2CSIDL = 0;  // Module continues in Idle.
		I2C1CONLbits.SCLREL  = 1;  // Release clock (Slave mode))
		I2C1CONLbits.A10M    = 0;  // 7 bit mode
		I2C1CONLbits.DISSLW  = 1;  // Disable Slew rate.
		I2C1CONLbits.SMEN    = 0;  // Disable SMBus thresholds
		I2C1CONLbits.GCEN    = 1;  // Enable General call address
		I2C1CONLbits.STREN   = 0;  // Disable clock stretching
		I2C1CONLbits.ACKDT   = 0;  // Send /ACK to acknowledge

		I2C1CONHbits.SCIE    = 1;  // Enable Start ISR
		I2C1CONHbits.PCIE    = 1;  // Enable Stop ISR
		I2C1MSK           = 0xff;  // Don't care
		I2C1ADD           = 0xa0;

#endif //0
		IFS1bits.MI2C1IF     = 0;  // Clear Master ISR Flag
		IFS1bits.SI2C1IF     = 0;  // Clear Slave ISR Flag
		IEC1bits.MI2C1IE     = 1;  // Enable Master Interrupts
		IEC1bits.SI2C1IE     = 1;  // Enable Slave Interrupts

		I2C1CONbits.I2CEN    = 1;
//		I2C1CONLbits.PEN     = 1;  // Send Stop
		break;
#endif
#if defined(SYS_I2C_2)
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
        return (SUCCESS);
}
#else
result_t i2c_py_reserve(struct i2c_device *device)
{
	result_t          rc;
	struct period     period;
	period.units    = mSeconds;
	period.duration = 500;

	/*
	 * Set out watch pin high
	 */
	TRISBbits.TRISB13 = 0;
	LATBbits.LATB13 = 1;

	/*
	 * Pin RB8 is Clock, set input/Open Drian
	 */
	ODCBbits.ODCB8   = 1;
 	TRISBbits.TRISB8 = 1;

	/*
	 * Pin RB9 is out Data pin SDA, Set input/Open Drain
	 */
	ODCBbits.ODCB9   = 1;
	TRISBbits.TRISB9 = 1;

	/*
	 * Enable I2C1 Interrupts
	 */
	IFS1bits.MI2C1IF     = 0;   // Clear Master ISR Flag
	IFS1bits.SI2C1IF     = 0;   // Clear Slave ISR Flag
	IEC1bits.MI2C1IE     = 1;   // Enable Master Interrupts
	IEC1bits.SI2C1IE     = 1;   // Enable Slave Interrupts
        INTCON2bits.GIE      = 1;   // Global Interrup Enable

	I2C1ADD              = 0x00; // I2C Address

	I2C1CONbits.I2CEN    = 1;   // Enable the I2C

	/*
	 * Pin RB2 is our EEPROM Trigger. trigger read with positive edge
	 */
 	TRISBbits.TRISB2 = 0;

	while(1) {
		LOG_D("I2C1ADD:0x%x\n\r", I2C1ADD);
		rc = delay(&period);
		LATBbits.LATB2    = 0;
		rc = delay(&period);
		LATBbits.LATB2    = 1;
		I2C1CONbits.I2CEN = 0;            // Disable the I2C
		I2C1ADD           = I2C1ADD + 1;
		I2C1CONbits.I2CEN = 1;            // Enable the I2C
	}
}
#endif

result_t i2c_py_release(struct i2c_device *device)
{
	LOG_D("i2c_py_release\n\r");
	return(SUCCESS);
}

result_t i2c_py_start(struct i2c_device *device)
{
//	enum i2c_chan_id chan = device->channel;
//	static uint8_t count = 0;

	LOG_D("i2c_py_start\n\r");
#if 0
	switch(chan) {
#if defined (SYS_I2C_1)
	case I2C1:
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
#if defined (SYS_I2C_2)
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
	default:
		return (-ERR_BAD_INPUT_PARAMETER);
		break;
	}
#endif
        return(SUCCESS);
}

result_t i2c_py_restart(struct i2c_device *device)
{
//	enum i2c_chan_id chan = device->channel;

	LOG_D("i2c_py_restart\n\r");
#if 0
	switch(chan) {
#if defined(SYS_I2C_1)
	case I2C1:
		if (I2C1_STARTED) {
			i2c_channels[chan].state = RESTARTING_STATE;
			I2C1_RESTART;
			return(SUCCESS);
		} else {
			LOG_E("Bad State\n\r");
			return (-ERR_BAD_STATE);
		}
		break;
#endif // SYS_I2C_1
	default:
		return (-ERR_BAD_INPUT_PARAMETER);
		break;
	}
#endif
        return(SUCCESS);
}

result_t i2c_py_stop(struct i2c_device *device)
{
//	enum i2c_chan_id chan = device->channel;
	LOG_D("i2c_py_stop\n\r");
#if 0
	switch(chan) {
#if defined(SYS_I2C_1)
	case I2C1:
		i2c_channels[chan].state = STOPPING_STATE;
		I2C1_STOP;
		break;
#endif // SYS_I2C_1
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
#endif
        return(SUCCESS);
}

result_t i2c_py_write(struct i2c_device *device, uint8_t *p_tx_buf, uint8_t p_num_tx_bytes, void (*p_callback)(result_t))
{
	LOG_D("i2c_py_write()\n\r");
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

result_t i2c_py_read(struct i2c_device *device, uint8_t *p_rx_buf, uint16_t p_num_rx_bytes, void (*p_callback)(result_t, uint8_t))
{
	LOG_D("i2c_py_read()\n\r");
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

static void i2c_py_send_next(enum i2c_chan_id chan)
{
	LOG_E("CODE?");
#if 0
	if (i2c_channels[chan].sent < i2c_channels[chan].num_tx_bytes) {
		i2c_channels[chan].state = TX_STATE;
		switch(chan) {
#if defined(SYS_I2C_1)
		case I2C1:
			I2C1TRN = *i2c_channels[chan].tx_buf++;
			break;
#endif
#if defined (SYS_I2C_2)
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
#endif
}

#endif // _SYS_I2C_1 || SYS_I2S2 || SYS_I2S3
