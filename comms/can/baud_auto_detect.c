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

