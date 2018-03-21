/**
 *
 * \file libesoup/comms/one_wire/one_wire.c
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

#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/delay.h"
//#include "libesoup/timers/sw_timers.h"
#include "libesoup/comms/one_wire/one_wire.h"
#include "libesoup/processors/dsPIC33/change_notification/change_notification.h"

#ifndef SYS_CHANGE_NOTIFICATION
#error SYS_CHANGE_NOTIFICATION Not defined required by OneWire
#endif

#ifndef SYS_ONE_WIRE_MAX_BUS
#error SYS_ONE_WIRE_MAX_BUS should be defined
#endif

#ifndef SYS_ONE_WIRE_MAX_DEVICES
#error SYS_ONE_WIRE_MAX_DEVICES should be defined
#endif

//static uint32_t d;
//#define DELAY(x) for(d = 0; d < (uint32_t)(((float)x )/NOP_DURATION); d++) Nop();

//static uint8_t read_bit;
//static uint8_t read_count;
//static uint8_t write_byte;
//static uint8_t write_count;
//static     enum   pin_t pin;
//static volatile uint8_t device_present = FALSE;
//static          uint8_t bus_busy = FALSE;
//static          uint8_t bus_level;
//static void (*callback)(void) = NULL;

static volatile uint8_t timer_expired = TRUE;

struct one_wire_bus {
	uint8_t                 active:1;
	uint8_t                 semaphore:1;
	enum pin_t              pin;
} one_wire_bus;

struct one_wire_device {
	uint8_t                 active:1;
	uint16_t                id;
	struct one_wire_bus    *bus;
} one_wire_device;


static struct one_wire_bus    bus[SYS_ONE_WIRE_MAX_BUS];
static struct one_wire_device device[SYS_ONE_WIRE_MAX_DEVICES];

/*
 * Function Prototypes.
 */
static result_t census(struct one_wire_bus *bus);
static void     bus_change(uint8_t *port, uint8_t bit);
static result_t reset_pulse(struct one_wire_bus *bus);
static result_t set_pin(enum pin_t pin, uint8_t direction, uint8_t value);

//static result_t set_pin(enum pin_t pin, uint8_t direction, uint8_t value);
//static result_t get_pin(enum pin_t pin, uint8_t *value);
//static void     expiry_fn(void *data);
//static result_t reset_pulse(enum pin_t pin);
//static result_t tx_byte(enum pin_t pin, uint8_t byte);
//result_t rx_byte(enum pin_t pin, uint8_t *byte);
//static uint8_t  rx_bit(enum pin_t pin);

//static result_t program_pulse(enum pin_t pin);

//static void exp_end_reset(void *data);
//static void exp_end_presence(void *data);

//static void write_one();
//static void exp_end_write_one_pulse(void *data);
//static void exp_end_write_one(void *data);

//static void write_zero();
//static void exp_end_write_zero(void *data);

//static void read();
//static void exp_end_read_pulse(void *data);
//static void exp_read(void *data);
//static void exp_end_read(void *data);

//static void write_byte_fn(uint8_t byte);
//static void write_bit_fn(void);

//static void rom_command(void);
//static void read_rom(void);
//static void bit_read(uint8_t read_bit);

result_t one_wire_init(void)
{
	uint8_t loop;

	for(loop = 0; loop < SYS_ONE_WIRE_MAX_BUS; loop++) {
		bus[loop].active    = 0b0;
		bus[loop].semaphore = 0b0;
	}

	for(loop = 0; loop < SYS_ONE_WIRE_MAX_DEVICES; loop++) {
		device[loop].active = 0b0;
	}
}

result_t one_wire_reserve(enum pin_t pin)
{
	result_t rc;
	uint8_t  b_loop;
	uint8_t  d_loop;
	
	LOG_D("one_wire_reserve()\n\r");
	/*
	 * check for existing bus on the given pin
	 */
	for(b_loop = 0; b_loop < SYS_ONE_WIRE_MAX_BUS; b_loop++) {
		if(bus[b_loop].active && (bus[b_loop].pin == pin)) {
			return(ERR_BAD_INPUT_PARAMETER);
		}
	}
	
	/*
	 * Find a free slot
	 */
	for(b_loop = 0; b_loop < SYS_ONE_WIRE_MAX_BUS; b_loop++) {
		if(!bus[b_loop].active) {
			bus[b_loop].active    = 0b1;
			bus[b_loop].semaphore = 0b1;
			bus[b_loop].pin       = pin;

			return(set_pin(pin, OUTPUT_PIN, 1));

			//return(census(&bus[b_loop]));
		}
	}
	
	return(ERR_NO_RESOURCES);
}

static result_t census(struct one_wire_bus *bus)
{
	result_t  rc;
	uint8_t  *port;
	uint8_t   bit;

	LOG_D("Census\n\r");
	
	/*
	 * Reset pulse
	 */
        delay(mSeconds, 500);
        rc = set_pin(bus->pin, OUTPUT_PIN, 0);
        delay(mSeconds, 500);
        rc = set_pin(bus->pin, OUTPUT_PIN, 1);
	
	/*
	 * Want to receive a presence pulse from the devices on the bus
	 * so request change notification on the bus I/O Pin
	 */
	rc = pin_to_port_bit(bus->pin, &port, &bit);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
                LOG_E("Failed pin to port bit\n\r");
#endif
	}

	rc = change_notifier_register(port, bit, bus_change);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
                LOG_E("Failed notify request\n\r");
#endif
	}
	
	return(rc);
}

static void bus_change(uint8_t *port, uint8_t bit)
{
	LOG_D("Change\n\r");	
}

/*
 * Synchronous Function it will spin
 */
#if 0
static result_t reset_pulse(struct one_wire_bus *bus)
{
        uint8_t  i;
        uint8_t  value;
        uint8_t  hw_timer;
        result_t rc;
        
        /*
         * First reset pulse at least 480uS 
         */
        rc = set_pin(bus->pin, OUTPUT_PIN, 0);

        delay(uSeconds, 500);

        rc = set_pin(bus->pin, OUTPUT_PIN, 1);

        /*
         * DS2502 should reply with 60-240uS pulse after a 15-60uS Delay
         * There is a recovery time as well, at least 480uS.
         */
        timer_expired = FALSE;
        hw_timer = hw_timer_start(uSeconds, 500, FALSE, expiry_fn, NULL);

        /*
         * I'll ignore the return code from the function. We've tested the
         * value of pin previously so unless it's been corrupted by a write
         * it should be good.
         */
        rc = get_pin(pin, &value);
        while(value && !timer_expired)
                rc = get_pin(pin, &value);
        
        /*
         * The line has either gone low or the timer has expired.
         */
        if(timer_expired) {
                /*
                 * Bad! :-( Timer has expired with no response from the Bus
                 */
                return(ERR_NO_RESPONSE);                
        }
        
        /*
         * The Bus level should now be low because whatever is on the bus
         * (assume it's the DS2502) has pulled the bus low. The bus should
         * remain low for 60-240uS but I'm not going to time that response 
         * for the moment we'll assume it's in spec.
         */
        if(value) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
                LOG_E("Unexpected level detected on the BUS\n\r");
#endif
                return(ERR_GENERAL_ERROR);
        }

//        LOG_D("\\/ Value 0x%x\n\r", value);
        
        while(!value && !timer_expired)
                rc = get_pin(pin, &value);
        
        /*
         * Now the Bus should have gone high again after the response pulse
         * from the Slave device(s).
         */
        if(timer_expired) {
                /*
                 * Oops! The timer expired, as if the Slave(s) pulled the bus
                 * low but forgot to raise it again?
                 */
                if(!value) {
                        /*
                         * Oops? The Bus should have gone high
                         */
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
                        LOG_E("Unexpected level detected on the BUS\n\r");
#endif
                        return(ERR_GENERAL_ERROR);
                }
        }

        /*
         * Wait for the timer to expire so we allow recovery time on bus
         */
        while(!timer_expired) Nop();
        
        return(SUCCESS);
}
#endif

static result_t set_pin(enum pin_t pin, uint8_t direction, uint8_t value)
{
        switch(pin) {
        case (RF3):
                ODCFbits.ODCF3 = 1;            // Open Drive Pin
                TRISFbits.TRISF3 = direction;
                LATFbits.LATF3= value;
                break;
                
        case (RD0):
                ODCDbits.ODCD0 = 1;            // Open Drive Pin
                TRISDbits.TRISD0 = direction;
                LATDbits.LATD0= value;
                break;
                
        default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                LOG_E("Pin has not been coded yet!\n\r");
#endif
                return(ERR_NOT_CODED);
              break;
        }
        return(SUCCESS);
}

//static result_t get_pin(enum pin_t pin, uint8_t *value)
//{
//        switch(pin) {
//        case (RF3):
//                ODCFbits.ODCF3 = 1;            // Open Drive Pin
//                TRISFbits.TRISF3 = INPUT_PIN;
//                *value = PORTFbits.RF3;
//                break;
//                
//        case (RD0):
//                ODCDbits.ODCD0 = 1;            // Open Drive Pin
//                TRISDbits.TRISD0 = INPUT_PIN;
//                *value = PORTDbits.RD0;
//                break;
//                
//        default:
//                pin = INVALID_PIN;
//#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
//                LOG_E("Pin has not been coded yet!\n\r");
//#endif
//                return(ERR_BAD_INPUT_PARAMETER);
//              break;
//        }
//        
//        return(SUCCESS);
//}

//result_t one_wire_get_device_count(enum pin_t pin, uint8_t *count)
//{
//        if(bus_busy) {
//                return(ERR_NOT_READY);
//        }
//        
//        *count = 0;
//        return(SUCCESS);
//}


//static void expiry_fn(void *data)
//{
//        timer_expired = TRUE;
//}

//static result_t tx_byte(enum pin_t pin, uint8_t byte)
//{
//        uint8_t  bit = 0;
//        uint8_t  i;
//        result_t rc;
//
//        /*
//         * Start with the least significant bit. 
//         */
//        bit = 0x01;
//        
//        for(i = 0; i < 8; i++, bit = bit << 1) {
//                /*
//                 * Check is the bit zero or one
//                 */
//                if(byte & bit) {
//                        /*
//                         * Write one to the Bus. Starts with pulling bus low 
//                         * for less then 15uS. I'll go 2uS
//                         */
//                        // Todo make the 10uS time a #define
//                        INTCON2bits.GIE = DISABLED;
//                        rc = set_pin(pin, OUTPUT_PIN, 0);
//                        
//                        Nop();
//                        Nop();
////                        delay(uSeconds, 2);
//                        
//                        /*
//                         * Now all the line to go high for the remainder of 
//                         * the slot.
//                         * The slot is 60uS <= slot <= 120uS Total
//                         */
//                        rc = set_pin(pin, OUTPUT_PIN, 1);
//                        INTCON2bits.GIE = ENABLED;
//        
//                        delay(uSeconds, 100);
//                } else {
//                        /*
//                         * writing 0 to the bus we pull it low for the entire
//                         * slot. I'll use 100uS for the slot
//                         */
//                        rc = set_pin(pin, OUTPUT_PIN, 0);
//        
//                        delay(uSeconds, 100);
//
//                        rc = set_pin(pin, OUTPUT_PIN, 1);
//                }
//                
//                /*
//                 * Let's pause for thought before the next bit. The datasheet 
//                 * for the DS2502 says between 1uS and Infinity. I'll got 5uS
//                 */
//                delay(uSeconds, 5);
//        }
//        
//        /*
//         * End of the byte so pause 
//         */
//        delay(uSeconds, 100);
//
////#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
////        LOG_D("Finished the tx_byte() active timers %d\n\r", hw_timer_active_count());
////#endif
//        return(SUCCESS);
//}

//result_t rx_byte(enum pin_t pin, uint8_t *byte)
//{
//        uint8_t  i;
//        uint8_t  read_byte = 0x00;
//        uint8_t  value;
//        uint8_t  hw_timer;
//        result_t rc;
//        uint32_t j;
//
//        for(i = 0; i < 8; i++) {
//                read_byte = read_byte >> 1;
//                __asm__ ("CLRWDT");
//                /*
//                 * Send the low pulse on the bus to signal the Slave can
//                 * write to the bus. 1uS <= pulse <= 15uS. I'll use 2uS
//                 */
//                INTCON2bits.GIE = DISABLED;
////                SRbits.IPL = 0x07;
//                rc = set_pin(pin, OUTPUT_PIN, 0);
//                
//                Nop();  
//                        
//                /*
//                 * Now all the line to go high for the remainder of 
//                 * the read slot.
//                 * The slot is 60uS <= slot <= 120uS Total
//                 */
//                rc = set_pin(pin, OUTPUT_PIN, 1);
////                SRbits.IPL = 0x00;
//                INTCON2bits.GIE = ENABLED;
//
//                if(rx_bit(pin)) read_byte = read_byte | 0x80;
//        }
//
//        *byte = read_byte;
//        return(SUCCESS);
//}

//static uint8_t rx_bit(enum pin_t pin)
//{
//        uint8_t  i;
//        uint8_t  value;
//        uint8_t  hw_timer;
//        result_t rc;
//        uint32_t j;
//
//        rc = get_pin(pin, &value);
//
//        /*
//         * We'll delay for the slot 60 - 120uS
//         */
//        delay(uSeconds, 80);
//        return(value);
//}

//void reset_pulse()
//{
//        uint8_t hw_timer;
//
//        /*
//         * Make sure the pin is an output
//         */
//        if (pin != INVALID_PIN) {
//                device_present = FALSE;
//                
//                switch(pin) {
//                case RF3:
//                        TRISFbits.TRISF3 = OUTPUT_PIN;
//                        LATFbits.LATF3 = 0;
//                        break;
//
//                default:
//#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
//                        LOG_E("Pin has not been coded yet!\n\r");
//#endif
//                        break;
//                }
//
//                /*
//                 * Reset pulse minimum 480 uS
//                 */        
//                hw_timer = hw_timer_start(uSeconds, 500, FALSE, exp_end_reset, NULL);
//                
//                bus_busy = TRUE;
//        }
//}

//void exp_end_reset(void *data)
//{
//        uint8_t hw_timer;
//        
//        /*
//         * Now set the pin as an input and wait for presence pulse
//         */
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        TRISFbits.TRISF3 = INPUT_PIN;
//                        
//                        /*
//                         * Store the current value on the bus to detect change
//                         */
//                        bus_level = PORTFbits.RF3;
//                        CNENFbits.CNIEF3 = ENABLED;
//                        break;
//                        
//                default:
//                        break;
//                }
//                
//                IFS1bits.CNIF = 0;
//                IEC1bits.CNIE = 1;
//        
//                hw_timer = hw_timer_start(uSeconds, 500, FALSE, exp_end_presence, NULL);
//        }
//}

//void exp_end_presence(void *data)
//{
//        /*
//         * set pin to Output and cancel change notification
//         */
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        TRISFbits.TRISF3 = OUTPUT_PIN;
//                        CNENFbits.CNIEF3 = DISABLED;
//                        break;
//                        
//                default:
//                        break;
//                }
//                
//                IFS1bits.CNIF = 0;
//                IEC1bits.CNIE = 0;
//
//                if(!device_present) {
//#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
//                        LOG_E("No Devices!\n\r");
//#endif
//                } else {
//                        if(callback) {
//                                callback();
//                        } else {
//                                bus_busy = FALSE;
//#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
//                                LOG_E("No callback function\n\r");
//#endif
//                        }
//                }
//        }
//}

//void write_one()
//{
//        uint8_t hw_timer;
//        
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        TRISFbits.TRISF3 = OUTPUT_PIN;
//                        LATFbits.LATF3 = 0;
//                        break;
//                        
//                default:
//                        break;
//                }
//
//                /*
//                 * pulse 15 uS
//                 */
//                hw_timer = hw_timer_start(uSeconds, 20, FALSE, exp_end_write_one_pulse, NULL);
//        }
//}

//void exp_end_write_one_pulse(void *data)
//{
//        uint8_t hw_timer;
//
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        LATFbits.LATF3 = 1;
//                        break;
//                        
//                default:
//                        break;
//                }
//        
//                hw_timer = hw_timer_start(uSeconds, 100, FALSE, exp_end_write_one, NULL);
//        }
//}

//void exp_end_write_one(void *data)
//{
//        write_bit_fn();
//}

//void write_zero()
//{
//        uint8_t hw_timer;
//
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        TRISFbits.TRISF3 = 0;
//                        LATFbits.LATF3 = 0;
//                        break;
//                        
//                default:
//                        break;
//                }
//                
//                /*
//                 * pulse 15 uS
//                 */
//                hw_timer = hw_timer_start(uSeconds, 120, FALSE, exp_end_write_zero, NULL);
//        }
//}

//void exp_end_write_zero(void *data)
//{
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        TRISFbits.TRISF3 = INPUT_PIN;
//                        write_bit_fn();
//                        
//                default:
//                        break;
//                }
//        }
//}

//void read()
//{
//        uint8_t hw_timer;
//
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        TRISFbits.TRISF3 = OUTPUT_PIN;
//                        LATFbits.LATF3 = 0;
//                        break;
//                        
//                default:
//                        break;
//                }
//
//                /*
//                 * pulse 15 uS
//                 */        
//                hw_timer = hw_timer_start(uSeconds, 20, FALSE, exp_end_read_pulse, NULL);
//        }
//}

//void exp_end_read_pulse(void *data)
//{
//        uint8_t hw_timer;
//
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        LATFbits.LATF3 = 1;
//                        TRISFbits.TRISF3 = INPUT_PIN;
//                        break;
//                        
//                default:
//                        break;
//                }
//                hw_timer = hw_timer_start(uSeconds, 20, FALSE, exp_read, NULL);
//        }
//}

//void exp_read(void *data)
//{
//        uint8_t hw_timer;
//        
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        read_bit = PORTFbits.RF3;
//                        break;
//                        
//                default:
//                        break;
//                }
//                hw_timer = hw_timer_start(uSeconds, 80, FALSE, exp_end_read, NULL);        
//        }
//}

//void exp_end_read(void *data)
//{
//        if (pin != INVALID_PIN) {
//                switch(pin) {
//                case RF3:
//                        TRISFbits.TRISF3 = OUTPUT_PIN;
//                        break;
//                        
//                default:
//                        break;
//                }        
//                bit_read(read_bit);
//        }
//}

//void write_byte_fn(uint8_t data)
//{
//#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//        LOG_D("write_byte_fn(0x%x)\n\r", data);
//#endif
//        write_byte = data;
//        write_count = 0;
//
//        write_bit_fn();
//}

//void write_bit_fn(void)
//{
//        if(write_count < 8) {
//                if (write_byte & 0x01) {
//                        write_one();
//                } else {
//                        write_zero();
//                }
//                write_byte = write_byte >> 1;
//                write_count++;
//        } else {
//                if(callback) {
//                        callback();
//                } else {
//#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
//		        LOG_E("No callback function\n\r");
//#endif
//                }
//        }
//}

//void bit_read(uint8_t read_bit)
//{
//#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//        LOG_D("bit_read(0x%x)\n\r", read_bit);
//#endif
//        read_count--;        
//        if(read_count > 0) {
//                read();
//        }
//}

#endif // SYS_ONE_WIRE
