/**
 * @file libesoup/comms/one_wire/one_wire.c
 *
 * @author John Whitmore
 * 
 * Copyright 2017-2018 electronicSoup Limited
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

#ifdef SYS_ONE_WIRE

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "OneWire";
#include "libesoup/logger/serial_log.h"
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#ifndef SYS_ONE_WIRE_MAX_BUS
#error SYS_ONE_WIRE_BUS Should be defined
#endif

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/delay.h"
#include "libesoup/comms/one_wire/one_wire.h"
#include "libesoup/gpio/change_notification.h"

#ifndef SYS_CHANGE_NOTIFICATION
#error SYS_CHANGE_NOTIFICATION Not defined required by OneWire
#endif

#ifndef SYS_ONE_WIRE_MAX_BUS
#error SYS_ONE_WIRE_MAX_BUS should be defined
#endif

#ifndef SYS_ONE_WIRE_MAX_DEVICES
#error SYS_ONE_WIRE_MAX_DEVICES should be defined
#endif

/*
 * Local definitions
 */
#define NO_CHANNEL 0xFF
#define NO_DEVICE  0xFF

/*
 * One Wire Commands
 */
#define READ_ROM      0x33
#define MATCH_ROM     0x55
#define ALARM_SEARCH  0xEC
#define SEARCH        0xF0

/*
 * Local variables
 */
static volatile uint8_t  timer_expired = TRUE;
static          int16_t  bus_changes;

struct one_wire_bus {
	uint8_t                 active:1;
	uint8_t                 sem_device;
	enum pin_t              pin;
} one_wire_bus;

struct one_wire_device {
	uint8_t                 ow_channel;
	uint8_t                 family;
	uint8_t                 serial_number[6];
	uint8_t                 crc;
} one_wire_device;


static struct one_wire_bus    bus[SYS_ONE_WIRE_MAX_BUS];
static struct one_wire_device device[SYS_ONE_WIRE_MAX_DEVICES];

/*
 * Function Prototypes.
 */
static result_t census(int16_t chan);
static void     bus_change(enum pin_t pin);
static result_t reset_pulse(int16_t chan);
static result_t write_byte(int16_t chan, uint8_t byte);
static result_t write_one(int16_t chan);
static result_t write_zero(int16_t chan);
static result_t read_byte(int16_t chan);
static result_t read_bit(int16_t chan);


result_t one_wire_init()
{
	uint8_t loop;

	for(loop = 0; loop < SYS_ONE_WIRE_MAX_BUS; loop++) {
		bus[loop].active     = 0b0;
		bus[loop].sem_device = NO_DEVICE;
	}

	for(loop = 0; loop < SYS_ONE_WIRE_MAX_DEVICES; loop++) {
		device[loop].ow_channel = NO_CHANNEL;
	}
	return(0);
}

/*
 * Create a OneWire bus on the given pin
 * 
 * Returns the created channel number
 */
result_t one_wire_reserve(enum pin_t pin)
{
	result_t rc;
	uint8_t  loop;
	int16_t  chan;
	LOG_D("one_wire_reserve()\n\r");
	/*
	 * check for existing bus on the given pin
	 */
	for(loop = 0; loop < SYS_ONE_WIRE_MAX_BUS; loop++) {
		if(bus[loop].active && (bus[loop].pin == pin)) {
			return(ERR_BAD_INPUT_PARAMETER);
		}
	}

	/*
	 * Find a free slot
	 */
	for(loop = 0; loop < SYS_ONE_WIRE_MAX_BUS; loop++) {
		if(!bus[loop].active) {
			chan = loop;
			break;
		}
	}
	
	if(chan < SYS_ONE_WIRE_MAX_BUS) {
		bus[chan].active     = 0b1;
		bus[chan].sem_device = NO_DEVICE;
		bus[chan].pin        = pin;

		rc = gpio_set(pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 1);
		RC_CHECK

		rc = census(chan);
		RC_CHECK
		return(chan);
	}
	return(ERR_NO_RESOURCES);
}

void set_bit(int8_t index, uint8_t bit)
{
	if(bit < 8) {
		device[index].family |= (0b1 << bit);
	} else if (bit < 16) {
		device[index].serial_number[0] |= (0b1 << (bit - 8));			
	} else if (bit < 24) {
		device[index].serial_number[1] |= (0b1 << (bit - 16));			
	} else if (bit < 32) {
		device[index].serial_number[2] |= (0b1 << (bit - 24));			
	} else if (bit < 40) {
		device[index].serial_number[3] |= (0b1 << (bit - 32));			
	} else if (bit < 48) {
		device[index].serial_number[4] |= (0b1 << (bit - 40));			
	} else if (bit < 56) {
		device[index].serial_number[5] |= (0b1 << (bit - 48));			
	} else if (bit < 64) {
		device[index].crc |= (0b1 << (bit - 56));			
	}
}

/*
 * Returns the number of devices on the OneWire bus
 */
static result_t census(int16_t chan)
{
	result_t  rc;
	uint8_t   loop;
	boolean   first_read;
	boolean   second_read;
	uint8_t   last_discrepency;
	uint8_t   device_index;

	delay(mSeconds, 100);	
	
	rc = reset_pulse(chan);
	RC_CHECK
	if(rc == 0) return(-ERR_NO_RESPONSE);

	rc = write_byte(chan, SEARCH);
//	rc = write_byte(bus, READ_ROM);
//	LOG_D("write_byte returned %d\n\r", rc);
	RC_CHECK

	last_discrepency = 0x00;
	device_index = 0;

	while(last_discrepency != 0xff) {
		last_discrepency = 0xff;  // Assume there will be no discrepency on the pass
		device[device_index].family     = 0x00;
		device[device_index].crc        = 0x00;
		device[device_index].ow_channel = chan;
		for(loop = 0; loop < 6; loop++) device[device_index].serial_number[loop] = 0x00;

		for(loop = 0; loop < 64; loop++) {
//			if(loop % 8 == 0) serial_printf(" ");
			rc = read_bit(chan);
			RC_CHECK
			first_read = (boolean)(rc == 1);

			rc = read_bit(chan);
			RC_CHECK
			second_read = (boolean)(rc == 1);
	
			if( (first_read == 0) && (second_read == 0) ) {
				// Discrepancy at this bit
				last_discrepency = loop;
				LOG_D("D\n\r");
			} else if( (first_read == 1) && (second_read == 1) ) {
				// No devices, must be finished
				LOG_D("No Devices\n\r");
				return(0);
				break;
			} else if(first_read) {
//				serial_printf("1");
				set_bit(device_index, loop);
				rc = write_one(chan);
				RC_CHECK
			} else {
//				serial_printf("0");
				rc = write_zero(chan);
				RC_CHECK
			}
		}
		device_index++;
	}
	//	rc = read_byte(bus);
	//	RC_CHECK
	//	family_code = rc;
#ifdef SYS_SERIAL_LOGGING
	LOG_D("Family Code 0x%x\n\r", device[0].family);
	for(loop = 0; loop < 6; loop++) {
		LOG_D("serial[%d] 0x%x\n\r", loop, device[0].serial_number[loop]);		
	}
	LOG_D("CRC 0x%x\n\r", device[0].crc);	
#endif
	return(0);
}

static void bus_change(enum pin_t pin)
{
	bus_changes++;
}

/*
 * Synchronous Function it will spin
 */
static result_t reset_pulse(int16_t chan)
{
        result_t rc;
        
	/*
	 * Want to receive a presence pulse from the devices on the bus
	 * so request change notification on the bus I/O Pin
	 */
	bus_changes = 0;
	rc = change_notifier_register(bus[chan].pin, bus_change);
	RC_CHECK
	
        delay(mSeconds, 100);
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 0);
	RC_CHECK
        delay(uSeconds, 500);
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 1);
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_INPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 1);
	RC_CHECK
		
	// Give the slaves on the bus time to respond
	delay(uSeconds, 500);
	rc = change_notifier_deregister(bus[chan].pin);
	RC_CHECK

	return(bus_changes);
}

static result_t write_byte(int16_t chan, uint8_t byte)
{
        uint8_t  bit = 0;
        uint8_t  i;
        result_t rc;

        /*
         * Start with the least significant bit. 
         */
        bit = 0x01;
        
        for(i = 0; i < 8; i++, bit = bit << 1) {
                /*
                 * Check is the bit zero or one
                 */
                if(byte & bit) {
			rc = write_one(chan);
			RC_CHECK
		} else {
			rc = write_zero(chan);
			RC_CHECK
		}
	}
        return(0);
}

result_t write_one(int16_t chan)
{
	result_t  rc;

	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 0);
	RC_CHECK
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 1);
	RC_CHECK
		
	// Wait for the end of the write slot
	delay(uSeconds, 100);

	return(0);
}

result_t write_zero(int16_t chan)
{
	result_t  rc;
	
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 0);
	RC_CHECK
        rc = delay(uSeconds, 100);
	RC_CHECK
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 1);
	RC_CHECK
        rc = delay(uSeconds, 20);
	RC_CHECK
		
	return(0);
}

static result_t read_byte(int16_t chan)
{
	result_t  rc;
	uint8_t   loop;
	uint8_t   byte = 0x00;

	for(loop = 0; loop < 8; loop++) {
		rc = read_bit(chan);
		RC_CHECK
			
		byte = byte >> 1;
		if(rc) {
			byte |= 0x80;
		} else {
			byte &= 0x7f;
		}
	}
	return(byte);
}

static result_t read_bit(int16_t chan)
{
	int16_t   bit;
	result_t  rc;
	
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 0);
	RC_CHECK
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_OUTPUT | GPIO_MODE_OPENDRAIN_OUTPUT, 1);
	RC_CHECK
	rc = gpio_set(bus[chan].pin, GPIO_MODE_DIGITAL_INPUT | GPIO_MODE_OPENDRAIN_INPUT, 1);
	RC_CHECK
	Nop();
	Nop();
	Nop();
	Nop();
	rc = gpio_get(bus[chan].pin);
	RC_CHECK
	bit = rc;
//	LOG_D("read bit %d\n\r", rc);
	rc = delay(uSeconds, 100);
	RC_CHECK
	return(bit);
}

#endif // SYS_ONE_WIRE
