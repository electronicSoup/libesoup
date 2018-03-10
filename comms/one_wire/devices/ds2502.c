/**
 *
 * \file libesoup/comms/one_wire/devices/ds2502.c
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

#ifdef SYS_ONE_WIRE_DS2502

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "DS2502";
#include "libesoup/logger/serial_log.h"
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#define READ_ROM                 0x33
#define MATCH_ROM                0x55
#define SKIP_ROM                 0xCC
#define SEARCH_ROM               0xF0

#define READ_DATA                0xF0 // Followed by 2 Byte Address
#define READ_STATUS              0xAA // Followed by 2 Byte Address
#define READ_DATA_GENERATE_CRC   0xC3 // Followed by 2 Byte Address
#define WRITE_DATA               0x0F // Followed by 2 Byte Address
#define WRITE_STATUS             0x55 // Followed by 2 Byte Address

#define MAX_ADDRESS              0x7F

#define DS2502_FAMILY_CODE       0x09




//extern result_t one_wire_init(enum pin_t pin);
//extern result_t one_wire_get_device_count(enum pin_t pin, uint8_t *count);
extern result_t one_wire_ds2502_read_rom(enum pin_t pin);

#if 0
#define DS2502_DDR                      TRISFbits.TRISF3
#define DS2502_OPEN_DRAIN_BIT           ODCFbits.ODCF3
#define DS2502_DATA_W                   LATFbits.LATF3
#define DS2502_DATA_R                   PORTFbits.RF3
#define DS2502_CHANGE_NOTIFICATION_ISR  CNENFbits.CNIEF3
#endif


result_t one_wire_ds2502_read_rom(enum pin_t pin)
{
        uint8_t  i;
        uint8_t  byte;
        result_t rc;
        
        /*
         * Start with a reset pulse
         */
        rc = reset_pulse(pin);

        rc = program_pulse(pin);
        return;
//        if(rc != SUCCESS) return(rc);
        
        /*
         * Now we want to send the read rom command
         */
        rc =  tx_byte(pin, READ_ROM);
        
        /*
         * Read the response from the Slave
         */
  //      for (i = 0; i < 8; i++)
        rc =  rx_byte(pin, &byte);
        
        if (byte != DS2502_FAMILY_CODE) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
                LOG_E("Unexpected Family Code\n\r");
#endif                
        } else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("DS2502 Present on OneWire Bus\n\r");
#endif                
        }
        
        return(SUCCESS);
}

result_t one_wire_ds2502_read_rom(enum pin_t pin)
{
        uint8_t  i;
        uint8_t  byte;
        result_t rc;
        
        /*
         * Start with a reset pulse
         */
        rc = reset_pulse(pin);

        rc = program_pulse(pin);
        return;
//        if(rc != SUCCESS) return(rc);
        
        /*
         * Now we want to send the read rom command
         */
        rc =  tx_byte(pin, READ_ROM);
        
        /*
         * Read the response from the Slave
         */
  //      for (i = 0; i < 8; i++)
        rc =  rx_byte(pin, &byte);
        
        if (byte != DS2502_FAMILY_CODE) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL != NO_LOGGING))
                LOG_E("Unexpected Family Code\n\r");
#endif                
        } else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("DS2502 Present on OneWire Bus\n\r");
#endif                
        }
        
        return(SUCCESS);
}

static result_t program_pulse(enum pin_t pin)
{
        uint8_t  i;
        uint8_t  value;
        uint8_t  hw_timer;
        result_t rc;
        
        /*
         */
        LATDbits.LATD1= 0;
        delay(uSeconds, 480);
        LATDbits.LATD1= 1;        
}

#endif // SYS_ONE_WIRE_DS2502
