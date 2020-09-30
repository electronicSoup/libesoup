/**
 *
 * @file libesoup/comms/spi/spi.c
 *
 * @author John Whitmore
 *
 * @brief SPI Interface functionality
 *
 * Copyright 2017-2020 electronicSoup Limited
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
 */
#if 1
#include "libesoup_config.h"

#if defined(SYS_SPI1) || defined(SYS_SPI2) || defined(SYS_SPI3)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
__attribute__ ((unused)) static const char *TAG = "SPI";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/peripheral.h"
#include "libesoup/comms/spi/spi.h"

#if defined(__dsPIC33EP128GS702__)
#define SPI_ENABLE_CS       SPI1CON1Hbits.MSSEN = 1;
#define SPI_DISABLE_CS      SPI1CON1Hbits.MSSEN = 0;
#define SPI1_ENABLE_MISO    SPI1CON1Lbits.DISSDI = 0;
#define SPI1_DISABLE_MISO   SPI1CON1Lbits.DISSDI = 1;
#define SPI1_ENABLE_MOSI    SPI1CON1Lbits.DISSDI = 0;
#define SPI1_DISABLE_MOSI   SPI1CON1Lbits.DISSDI = 1;
#else
#define SPI_ENABLE_CS   SPI1CON1bits.SSEN  = 1;
#define SPI_DISABLE_CS  SPI1CON1bits.SSEN  = 0;
#endif


struct spi_chan {
	boolean                active;
	struct spi_device     *active_device;
} spi_chan;

struct spi_chan channels[NUM_SPI_CHANNELS];

static result_t	channel_init(enum spi_channel ch);


#ifdef SYS_SPI1
void __attribute__((__interrupt__, __no_auto_psv__)) _SPI1TXInterrupt(void)
{
	IFS0bits.SPI1TXIF = 0;
	serial_printf("*SPI1_TX* H0x%x:L0x%x\n\r", SPI1STATH, SPI1STATL);
}
#endif

#ifdef SYS_SPI1
void __attribute__((__interrupt__, __no_auto_psv__)) _SPI1RXInterrupt(void)
{
	IFS0bits.SPI1RXIF = 0;
	serial_printf("*SPI1_RX* H0x%x:L0x%x\n\r", SPI1STATH, SPI1STATL);
}
#endif



result_t spi_init(void)
{
	enum spi_channel channel;

	for(channel = 0; channel < NUM_SPI_CHANNELS; channel++) {
		channels[channel].active        = FALSE;
		channels[channel].active_device = NULL;
	}
	return(SUCCESS);
}

/*
 * Returns the channel number
 */
result_t spi_reserve(struct spi_device *device)
{
	enum spi_channel channel;

	for (channel = 0; channel < NUM_SPI_CHANNELS; channel++) {
		if(channels[channel].active) {
			continue;
		} else {
			break;
		}
	}

	if (channel < NUM_SPI_CHANNELS) {
		channels[channel].active        = TRUE;
		channels[channel].active_device = device;
		device->channel                 = channel;

		return(channel_init(channel));
	}
	return(-ERR_NO_RESOURCES);
}

result_t spi_release(struct spi_device *device)
{
	enum spi_channel channel;

	channel = device->channel;
	if(channels[channel].active_device == device) {
		channels[channel].active        = FALSE;
		channels[channel].active_device = NULL;
		device->channel                 = NUM_SPI_CHANNELS;
		return (SUCCESS);
	} else {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
}

/*
 * Local static function to initialise the Peripheral pins of spi channel
 */
static result_t	channel_init(enum spi_channel ch)
{
	result_t           rc;
	struct spi_device *device;

	device = channels[ch].active_device;

	/*
	 * Setup GPIO pins
	 */
	if (device->io.miso != INVALID_GPIO_PIN) {
		gpio_set(device->io.miso, GPIO_MODE_DIGITAL_INPUT, 0);
	}

	if (device->io.mosi != INVALID_GPIO_PIN) {
		gpio_set(device->io.mosi, GPIO_MODE_DIGITAL_OUTPUT, 0);
	}

	/*
	 * Have to have a clock pin.
	 */
	gpio_set(device->io.sck,  GPIO_MODE_DIGITAL_OUTPUT, 0);

	if (device->io.cs != INVALID_GPIO_PIN) {
		gpio_set(device->io.cs,  GPIO_MODE_DIGITAL_OUTPUT, 0);
	}

	/*
	 * Set up the Peripheral pins for the SPI channel
	 */
	switch(ch) {
#if defined(SYS_SPI1)
	case SPI1:
		if (device->io.miso != INVALID_GPIO_PIN) {
			rc = set_peripheral_input(device->io.miso);
			RC_CHECK;
			PPS_I_SPI_1_DI = rc;
			SPI1_ENABLE_MISO;
		} else {
			SPI1_DISABLE_MISO;
		}
		if (device->io.mosi != INVALID_GPIO_PIN) {
			rc = set_peripheral_output(device->io.mosi, PPS_O_SPI1DO);
			RC_CHECK
			SPI1_ENABLE_MOSI;
		} else {
			SPI1_DISABLE_MOSI;
		}
		if (device->io.cs != INVALID_GPIO_PIN) {
			rc = set_peripheral_output(device->io.cs, PPS_O_SPI1SS);
			RC_CHECK
			SPI_ENABLE_CS;
		} else {
			SPI_DISABLE_CS;
		}

		rc = set_peripheral_output(device->io.sck,  PPS_O_SPI1CLK);
		RC_CHECK

		/*
	         * Init the SPI Config
	         */
#if defined(__dsPIC33EP256MU806___)
		SPI1CON1bits.MSTEN = 1;   // Master mode
		SPI1CON1bits.PPRE = 0x02;
		SPI1CON1bits.SPRE = 0x07;

		SPI1CON1bits.CKE = 0;
		SPI1CON1bits.CKP = 1;

		SPI1CON2 = 0x00;
		SPI1STATbits.SPIEN = 1;   // Enable the SPI
#elif defined(__dsPIC33EP128GS702__)
		SPI1CON1Lbits.MSTEN  = 1;   // Master mode
		SPI1CON1Lbits.SMP    = 1;
		SPI1CON1Lbits.ENHBUF = 1;   // Enable Enhanced buffer mode

		switch (device->bus_mode) {
		case bus_mode_0:
			serial_printf("SPI Mode 0\n\r");
			SPI1CON1Lbits.CKP = 0;
			SPI1CON1Lbits.CKE = 0;
			break;
		case bus_mode_1:
			serial_printf("SPI Mode 1\n\r");
			SPI1CON1Lbits.CKP = 0;
			SPI1CON1Lbits.CKE = 1;
			break;
		case bus_mode_2:
			serial_printf("SPI Mode 2\n\r");
			SPI1CON1Lbits.CKP = 1;
			SPI1CON1Lbits.CKE = 0;
			break;
		case bus_mode_3:
			serial_printf("SPI Mode 3\n\r");
			SPI1CON1Lbits.CKP = 1;
			SPI1CON1Lbits.CKE = 1;
			break;
		default:
			break;
		}

		SPI1BRGL = device->brg;

		SPI1CON2 = 0x00;

		SPI1IMSKL = 0xffff;   // Enable all ISRs for the moment
		SPI1IMSKH = 0xffff;   // Enable all ISRs for the moment

		SPI1STATLbits.SPIROV = 0;  // Clear overflow

		IFS0bits.SPI1TXIF = 0;
		IFS0bits.SPI1RXIF = 0;

		IEC0bits.SPI1TXIE = 1;
		IEC0bits.SPI1RXIE = 1;

		SPI1CON1Lbits.SPIEN = 1;   // Enable the SPI
#endif
		break;
#endif // SYS_SPI1
#if defined(SYS_SPI2)
	case SPI2:
		break;
#endif // SYS_SPI2
#if defined(SYS_SPI3)
	case SPI3:
		break;
#endif // SYS_SPI3
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	return(ch);
}

result_t spi_write_byte(struct spi_device *device, uint8_t write)
{
	enum spi_channel channel;

	channel = device->channel;

	if (channels[channel].active_device == device) {
		switch(channel) {
#if defined(SYS_SPI1)
		case SPI1:
#if defined(__dsPIC33EP256MU806__)
			SPI1BUF = write;
			while (!SPI1STATbits.SPIRBF);
			return(SPI1BUF);
#elif defined(__dsPIC33EP128GS702__)
			while (SPI1STATLbits.SPITBF);
			SPI1BUFL = write;
			while (SPI1STATLbits.SPIRBE);
			return(SPI1BUFL);
#endif // Micro-Controller
			break;
#endif // SYS_SPI1
		default:
			return(-ERR_BAD_INPUT_PARAMETER);
			break;
		}
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

#endif // SYS_SPI1 || SYS_SPI2 || SYS_SPI3

#else //0
#include <xc.h>

#define SPI_ENABLE_CS       SPI1CON1Hbits.MSSEN = 1;
#define SPI_DISABLE_CS      SPI1CON1Hbits.MSSEN = 0;
#define SPI1_ENABLE_MISO    SPI1CON1Lbits.DISSDI = 0;
#define SPI1_DISABLE_MISO   SPI1CON1Lbits.DISSDI = 1;
#define SPI1_ENABLE_MOSI    SPI1CON1Lbits.DISSDI = 0;
#define SPI1_DISABLE_MOSI   SPI1CON1Lbits.DISSDI = 1;

//#define SCK     RB11   // GREY    Idle High Works
//#define MOSI    RB13   // RED     Idle Low no activity
//#define SS      RB14   // ORANGE  Idle low no activity
//#define MISO    RA0    // YELLOW  Idle Hight no Activity

void __attribute__((__interrupt__, __no_auto_psv__)) _SPI1TXInterrupt(void)
{
	IFS0bits.SPI1TXIF = 0;
}

void __attribute__((__interrupt__, __no_auto_psv__)) _SPI1RXInterrupt(void)
{
	IFS0bits.SPI1RXIF = 0;
}


static void spi__init(void)
{
	TRISBbits.TRISB11 = 0;   // SCL
	TRISBbits.TRISB13 = 0;   // MOSI
	TRISBbits.TRISB14 = 0;   // SS
	ANSELAbits.ANSA0  = 0;
	TRISAbits.TRISA0  = 1;   // MISO

	RPINR20bits.SDI1R = 16;  // SPI 1 Data Input
	SPI1CON1Lbits.DISSDI = 0;  // Enable DI

	RPOR8bits.RP45R = 0x05;     // SPI 1 Data Output
	SPI1CON1Lbits.DISSDI = 0;   // Enable DI

	RPOR9bits.RP46R = 0x07;     // SPI 1 Slave Select Output
	SPI1CON1Hbits.MSSEN = 1;

	RPOR7bits.RP43R = 0x06;     ///< SPI 1 Clock Output

	SPI1CON1Lbits.MSTEN = 1;   // Master mode


	SPI1BRGL = 0xff;

	SPI1CON1Lbits.SPISIDL = 1;

	SPI1CON1bits.CKE = 0;
	SPI1CON1bits.CKP = 1;

	SPI1CON2 = 0x00;

	SPI1IMSKL = 0xffff;   // Enable all ISRs for the moment
	SPI1IMSKH = 0xffff;   // Enable all ISRs for the moment

	SPI1STATLbits.SPIROV = 0;  // Clear overflow

	IFS0bits.SPI1TXIF = 0;
	IFS0bits.SPI1RXIF = 0;

	IEC0bits.SPI1TXIE = 1;
	IEC0bits.SPI1RXIE = 1;

	SPI1CON1Lbits.SPIEN = 1;   // Enable the SPI
}

int main(void)
{
	uint16_t count = 0;

	spi__init();
	while(1) {

		count++;
		if (count >= 0x8000) {
			count = 0;
			SPI1BUFL = 0x55;
		}
		Nop();
	}
}

#endif // 0
