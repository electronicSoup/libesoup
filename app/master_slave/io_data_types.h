/**
 *
 * \file es_can/app/master_slave/io_data_types.h
 *
 * Definitions of the data type messages that can be sent between
 * the master and slaves in the system.
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef MASTER_SLAVE_DATA_TYPES_H
#define MASTER_SLAVE_DATA_TYPES_H

/*
 * We definition of the CAN frames used by the data types
 */
#define DATA_TYPE_CAN_BASE_ID   0x700


/*
 * The CAN frame message ID's defined by the Master Slave Protocol
 */
typedef enum io_data_type {
	DigitalOutput = 0,
	DigitalInput,
	PercentageOutput,
	PercentageInput
} io_data_type;


typedef union
{
    struct
    {
        BYTE    value:1;
        BYTE    pin:3;
        BYTE    node:4;
    } bit_field;

    u8 byte;
} can_digital_data_t;

/*
 * Percentage Data
 *
 * Two bytes:
 *           The Addressing byte (Node and Pin)
 *           The Value Byte
 */
typedef struct can_percentage_data_t
{
    union
    {
        struct
        {
            BYTE    pin:3;
            BYTE    node:4;
        } bit_field;

        BYTE byte;
    } address;

    BYTE value;
} can_percentage_data_t;

#endif // MASTER_SLAVE_DATA_TYPES_H
