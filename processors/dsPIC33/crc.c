/**
 * @file libesoup/processors/crc/crc.c
 *
 * @author John Whitmore
 * 
 * Copyright 2018 electronicSoup Limited
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
#include "libesoup_config.h"

#ifdef SYS_CRC

#ifdef SYS_SERIAL_LOGGING
//#define DEBUG_FILE
#undef DEBUG_FILE
static const char *TAG      = "CRC";
#include "libesoup/logger/serial_log.h"
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#include "libesoup/errno.h"
#include "libesoup/processors/dsPIC33/crc.h"
#ifdef SYS_TEST_BUILD
#include "libesoup/timers/delay.h"
#endif

static boolean busy         = FALSE;
uint8_t  *crc_data_byte     = (uint8_t *)&CRCDATL;

#ifdef SYS_TEST_BUILD
static uint8_t reverse_byte(uint8_t byte)
{
	union byte_bits {
		struct {
			uint8_t   bt0:1;
			uint8_t   bt1:1;
			uint8_t   bt2:1;
			uint8_t   bt3:1;
			uint8_t   bt4:1;
			uint8_t   bt5:1;
			uint8_t   bt6:1;
			uint8_t   bt7:1;
		} bits;
		uint8_t byte;
	};
	
	union byte_bits  input;
	union byte_bits  output;
	
	input.byte          = byte;
	
	output.bits.bt0     = input.bits.bt7;
	output.bits.bt1     = input.bits.bt6;
	output.bits.bt2     = input.bits.bt5;
	output.bits.bt3     = input.bits.bt4;
	output.bits.bt4     = input.bits.bt3;
	output.bits.bt5     = input.bits.bt2;
	output.bits.bt6     = input.bits.bt1;
	output.bits.bt7     = input.bits.bt0;
	
	return(output.byte);
}
#endif // SYS_TEST_BUILD

#ifdef SYS_TEST_BUILD
result_t crc_test(void)
{
	uint16_t  crc_sum;
 
        CRCCON1bits.CRCEN   = 0;          //DISABLED;
        CRCCON1bits.CSIDL   = 1;          // Stop in Idle mode
	CRCCON1bits.CRCISEL = 0;          // Interrupt when CRC Finished
	CRCCON1bits.LENDIAN = 1;          // One Wire is Little Endian (I think ;-)
        CRCCON2bits.PLEN    = 7;          // 8th Order Polynomial
        CRCCON2bits.DWIDTH  = 7;          // Working on 8 bit bytes        
	
        CRCXORL             = 0x131;      // polynomial X^8 + X^5 + X^4 + X^0  = 0b 1 0011 0001
        CRCXORH             = 0x00;
	
	/*
	 * Test the CRC of a single byte
	 */
        CRCWDATH            = 0;          // Zero the result before we start
        CRCWDATL            = 0;
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = 1;          // Enable the CRC Engine

	*crc_data_byte      = 0x10;       // As a test CRC a single byte
	*crc_data_byte      = 0x00;       // Add a zero byte

	CRCCON1bits.CRCGO   = 1;          // Start CRC

	while (!IFS4bits.CRCIF)           // Spin waiting for completion
		Nop();
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag

        crc_sum = CRCWDATL;
	if (crc_sum == 0xB9) {
		LOG_D("One Byte CRC - Good\n\r");
	} else {
		LOG_E("One Byte CRC - Bad (0x%x != 0xB9) {Reversed-0x%x}\n\r", crc_sum, reverse_byte(crc_sum));
	}
        CRCCON1bits.CRCEN   = 0;          // Disable the CRC Engine

	/*
	 * Test the CRC of a two bytes
	 */
	delay(mSeconds, 1);
        CRCWDATH            = 0;          // Zero the result before we start
        CRCWDATL            = 0;
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = 1;          // Enable the CRC Engine

	*crc_data_byte      = 0x10;       // Load two bytes
	*crc_data_byte      = 0x92;
	*crc_data_byte      = 0x00;       // Add a zero byte
	
	CRCCON1bits.CRCGO   = 1;          // Start CRC

	while (!IFS4bits.CRCIF)           // Spin waiting for completion
		Nop();
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag

        crc_sum = CRCWDATL;
	if(crc_sum == 0x82) {
		LOG_D("Two Byte CRC - Good\n\r");
	} else {
		LOG_E("Two Byte CRC - Bad (0x%x != 0x82) {Reversed-0x%x}\n\r", crc_sum, reverse_byte(crc_sum));
	}
        CRCCON1bits.CRCEN   = 0;          // Disable the CRC Engine

	/*
	 * Test the CRC of a three bytes
	 */
	delay(mSeconds, 1);
        CRCWDATH            = 0;          // Zero the result before we start
        CRCWDATL            = 0;
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = 1;          // Enable the CRC Engine

	*crc_data_byte      = 0x10;       // Load three bytes
	*crc_data_byte      = 0x92;
	*crc_data_byte      = 0xc8;
	*crc_data_byte      = 0x00;       // Add a zero byte
	
	CRCCON1bits.CRCGO   = 1;          // Start CRC

	while (!IFS4bits.CRCIF)           // Spin waiting for completion
		Nop();
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag

        crc_sum = CRCWDATL;
	if(crc_sum == 0x08) {
		LOG_D("Three Byte CRC - Good\n\r");
	} else {
		LOG_E("Three Byte CRC - Bad (0x%x != 0x08) {Reversed-0x%x}\n\r", crc_sum, reverse_byte(crc_sum));
	}
        CRCCON1bits.CRCEN   = 0;          // Disable the CRC Engine

	/*
	 * Test the CRC of a four bytes
	 */
	delay(mSeconds, 1);
        CRCWDATH            = 0;          // Zero the result before we start
        CRCWDATL            = 0;
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = 1;          // Enable the CRC Engine

	*crc_data_byte      = 0x10;       // Load four bytes
	*crc_data_byte      = 0x92;
	*crc_data_byte      = 0xc8;
	*crc_data_byte      = 0x72;
	*crc_data_byte      = 0x00;       // Add a zero byte

	CRCCON1bits.CRCGO   = 1;          // Start CRC

	while (!IFS4bits.CRCIF)           // Spin waiting for completion
		Nop();
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag

        crc_sum = CRCWDATL;
	if(crc_sum == 0x9B) {
		LOG_D("Four Byte CRC - Good\n\r");
	} else {
		LOG_E("Four Byte CRC - Bad (0x%x != 0x9B) {Reversed-0x%x}\n\r", crc_sum, reverse_byte(crc_sum));
	}
        CRCCON1bits.CRCEN   = 0;          // Disable the CRC Engine

	/*
	 * Test the CRC of a five bytes
	 */
	delay(mSeconds, 1);
        CRCWDATH            = 0;          // Zero the result before we start
        CRCWDATL            = 0;
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = 1;          // Enable the CRC Engine

	*crc_data_byte      = 0x10;       // Load five bytes
	*crc_data_byte      = 0x92;
	*crc_data_byte      = 0xc8;
	*crc_data_byte      = 0x72;
	*crc_data_byte      = 0x03;
	*crc_data_byte      = 0x00;       // Add a zero byte

	CRCCON1bits.CRCGO   = 1;          // Start CRC

	while (!IFS4bits.CRCIF)           // Spin waiting for completion
		Nop();
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag

        crc_sum = CRCWDATL;
	if(crc_sum == 0x94) {
		LOG_D("Five Byte CRC - Good\n\r");
	} else {
		LOG_E("Five Byte CRC - Bad (0x%x != 0x94) {Reversed-0x%x}\n\r", crc_sum, reverse_byte(crc_sum));
	}
        CRCCON1bits.CRCEN   = 0;          // Disable the CRC Engine

	/*
	 * Test the CRC of a six bytes
	 */
	delay(mSeconds, 1);
        CRCWDATH            = 0;          // Zero the result before we start
        CRCWDATL            = 0;
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = 1;          // Enable the CRC Engine

	*crc_data_byte      = 0x10;       // Load six bytes
	*crc_data_byte      = 0x92;
	*crc_data_byte      = 0xc8;
	*crc_data_byte      = 0x72;
	*crc_data_byte      = 0x03;
	*crc_data_byte      = 0x08;
	*crc_data_byte      = 0x00;       // Add a zero byte

	CRCCON1bits.CRCGO   = 1;          // Start CRC

	while (!IFS4bits.CRCIF)           // Spin waiting for completion
		Nop();
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag

        crc_sum = CRCWDATL;
	if(crc_sum == 0xBE) {
		LOG_D("Six Byte CRC - Good\n\r");
	} else {
		LOG_E("Six Byte CRC - Bad (0x%x != 0xBE) {Reversed-0x%x}\n\r", crc_sum, reverse_byte(crc_sum));
	}
        CRCCON1bits.CRCEN   = 0;          // Disable the CRC Engine

	/*
	 * Test the CRC of a seven bytes
	 */
	delay(mSeconds, 1);
        CRCWDATH            = 0;          // Zero the result before we start
        CRCWDATL            = 0;
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = 1;          // Enable the CRC Engine

	*crc_data_byte      = 0x10;       // Load seven bytes
	*crc_data_byte      = 0x92;
	*crc_data_byte      = 0xc8;
	*crc_data_byte      = 0x72;
	*crc_data_byte      = 0x03;
	*crc_data_byte      = 0x08;
	*crc_data_byte      = 0x00;
	*crc_data_byte      = 0x00;       // Add a zero byte

	CRCCON1bits.CRCGO   = 1;          // Start CRC

	while (!IFS4bits.CRCIF)           // Spin waiting for completion
		Nop();
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag

        crc_sum = CRCWDATL;
	if(crc_sum == 0xA0) {
		LOG_D("Seven Byte CRC - Good\n\r");
	} else {
		LOG_E("Seven Byte CRC - Bad (0x%x != 0xA0) {Reversed-0x%x}\n\r", crc_sum, reverse_byte(crc_sum));
	}
        CRCCON1bits.CRCEN   = 0;          // Disable the CRC Engine

	/*
	 * Test the CRC of a full OneWire ID Plus CRC (Eight bytes)
	 */
	delay(mSeconds, 1);
        CRCWDATH            = 0;          // Zero the result before we start
        CRCWDATL            = 0;
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = 1;          // Enable the CRC Engine

	*crc_data_byte      = 0x10;       // Load eight bytes
	*crc_data_byte      = 0x92;
	*crc_data_byte      = 0xc8;
	*crc_data_byte      = 0x72;
	*crc_data_byte      = 0x03;
	*crc_data_byte      = 0x08;
	*crc_data_byte      = 0x00;
	*crc_data_byte      = 0x05;
	*crc_data_byte      = 0x00;       // Add a zero byte

	CRCCON1bits.CRCGO   = 1;          // Start CRC

	while (!IFS4bits.CRCIF)           // Spin waiting for completion
		Nop();
        
	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag

        crc_sum = CRCWDATL;
	if(crc_sum == 0x00) {
		LOG_D("OneWire ID CRC - Good\n\r");
	} else {
		LOG_E("OneWire ID CRC - Bad (0x%x != 0x00) {Reversed-0x%x}\n\r", crc_sum, reverse_byte(crc_sum));
	}
        CRCCON1bits.CRCEN   = 0;          // Disable the CRC Engine

	return(0);
}
#endif // SYS_TEST_BUILD
	
result_t crc_reserve(uint32_t polynomial, uint8_t polynomial_length, uint8_t data_word_width, boolean little_endian)
{
        result_t rc;
        
        if (busy)
                return(-ERR_BUSY);
        
        CRCCON1bits.CRCEN   = DISABLED;
        CRCCON1bits.CSIDL   = ENABLED;   // Stop in Idle mode
	CRCCON1bits.CRCISEL = 0;         // Interrupt when CRC Finished
        
        if (little_endian)
                CRCCON1bits.LENDIAN = ENABLED;
        else
                CRCCON1bits.LENDIAN = DISABLED;

        CRCCON2bits.PLEN    = polynomial_length;
        CRCCON2bits.DWIDTH  = data_word_width;
        
        CRCXORL = (uint16_t)(polynomial & 0xffff);
        CRCXORH = (uint16_t)((polynomial >> 16) & 0xffff);
        
        rc = crc_sum_reset();
        RC_CHECK

	IFS4bits.CRCIF      = 0;          // Clear the ISR Flag
        CRCCON1bits.CRCEN   = ENABLED;
        return(0);
}

result_t crc_sum_byte(uint8_t data)
{
        if(!CRCCON1bits.CRCFUL) {
                *crc_data_byte    = data;
		CRCCON1bits.CRCGO = 1;    // Start CRC
                return(0);
        } else {
                return(-ERR_BUSY);
        }
}

result_t crc_sum_result(uint32_t *result)
{
        LOG_D("res count %d  CRCWDATL - 0x%x\n\r", CRCCON1bits.VWORD, CRCWDATL);
//        while(CRCCON1bits.CRCGO)
	
	if (CRCCON2bits.DWIDTH == 7) {
                *crc_data_byte = 0x00;
		CRCCON1bits.CRCGO = 1;    // Start CRC

		while (!IFS4bits.CRCIF)
			serial_printf(".");
        
		IFS4bits.CRCIF = 0;

		*result = CRCWDATL;
	} else {
		return(-ERR_NOT_CODED);
	}

        return(0);
}

result_t crc_sum_reset(void)
{
        CRCWDATH = 0;
        CRCWDATL = 0;
        return(0);
}

result_t crc_release(void)
{
        busy = FALSE;
        return(0);
}

#endif //SYS_CRC
