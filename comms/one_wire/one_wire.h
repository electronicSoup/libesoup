/**
 *
 * \file libesoup/comms/one_wire/one_wire.h
 *
 * Copyright 2017 - 2018 electronicSoup Limited
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
struct one_wire_device {
	struct one_wire_device *next;
	uint16_t                id;
} one_wire_device;

struct one_wire_bus {
	uint8_t                 pin;
	uint8_t                 semaphore;
	struct one_wire_device *device;
} one_wire_bus;


result_t one_wire_reserve(struct one_wire_bus *bus);

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
