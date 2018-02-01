/**
 *
 * \file libesoup/comms/can/baud_auto_detect.c
 *
 * Protocol for auto detecting CAN Bus baud rate
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
#include "libesoup_config.h"

#ifdef SYS_CAN_BAUD_AUTO_DETECT

void L2_SetCanNodeBuadRate(can_baud_rate_t baudRate)
{
//	baud_rate_t testRate;
	LOG_D("L2_SetCanNodeBuadRate()\n\r");
#if 0
	sys_eeprom_write(NETWORK_BAUD_RATE, (u8) baudRate);

	sys_eeprom_read(NETWORK_BAUD_RATE, (u8 *) & testRate);

	if (testRate != baudRate) {
		LOG_E("Baud Rate NOT Stored!\n\r");
	} else {
		LOG_D("Baud Rate Stored\n\r");
	}

	canStatus = ChangingBaud;
	setMode(CONFIG_MODE);

	setBitRate(baudRate);

	/*
	 * The Baud rate is being changed so going to stay in config mode
	 * for 10 Seconds and let the Network settle down.
	 */
	start_timer(SECONDS_TO_TICKS(10), finaliseBaudRateChange, NULL);
#endif //0
}

static void finaliseBaudRateChange(u8 *data)
{
	LOG_D("finaliseBaudRateChange()\n\r");
#if 0
	canStatus = Connected;
	setMode(NORMAL_MODE);
#endif
}

void L2_SetCanNetworkBuadRate(can_baud_rate_t baudRate)
{
	LOG_D("L2_SetCanNetworkBuadRate()\n\r");
#if 0
	setMode(CONFIG_MODE);
	setBitRate(baudRate);
	setMode(NORMAL_MODE);
#endif //0
}

#endif // SYS_CAN_BAUD_AUTO_DETECT
