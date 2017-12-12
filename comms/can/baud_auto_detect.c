#if defined(SYS_CAN_BAUD_AUTO_DETECT)

static void     exp_check_network_connection(timer_t timer_id, union sigval);

static can_baud_rate_t listen_baudrate = no_baud;

static uint8_t listen_timer;


void baud_auto_detect_init(void)
{
	/*
	 * Have to search for the Networks baud rate. Start at the bottom
	 */
	rx_msg_count = 0;
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Before trying 10K REC - %d, rxCount - %ld\n\r", read_reg(REC), rx_msg_count);
#endif
#endif
	listen_baudrate = baud_10K;
	set_baudrate(listen_baudrate);
	exit_mode = LISTEN_MODE;

	connecting_errors = 0;
	status.bit_field.l2_status = L2_Listening;
	status_baud = listen_baudrate;
	if(status_handler)
		status_handler(L2_STATUS_MASK, status, status_baud);

	/* Now wait and see if we have errors */
	sw_timer_start(SECONDS_TO_TICKS(CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD), exp_check_network_connection, (union sigval)(void *)NULL, &listen_timer);
}



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

void exp_check_network_connection(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
	result_t result;
//	uint8_t rec = read_reg(REC);

	if(listen_baudrate < no_baud) {
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("After trying %s Errors - %d, rxCount - %ld\n\r", can_baud_rate_strings[listen_baudrate], connecting_errors, rx_msg_count);
#endif
#endif
	} else {
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("After trying %s Errors - %d, rxCount - %ld\n\r", "NO BAUD RATE", connecting_errors, rx_msg_count);
#endif
#endif
	}
	/*
	 * If we heard valid messages with no errors we've found the baud rate.
	 */
	if(rx_msg_count > 0 && connecting_errors == 0) {
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("*** Network connected ***\n\r");
#endif
#endif
		connected_baudrate = listen_baudrate;

		set_can_mode(NORMAL_MODE);

		status.bit_field.l2_status = L2_Connected;
		status_baud = connected_baudrate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);
	} else {
		listen_baudrate++;
		if(listen_baudrate == no_baud)
			listen_baudrate = baud_10K;

#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("No joy try Baud Rate - %s\n\r", can_baud_rate_strings[listen_baudrate]);
#endif
#endif
		set_can_mode(CONFIG_MODE);
		set_baudrate(listen_baudrate);
		set_can_mode(LISTEN_MODE);

                rx_msg_count = 0;
		connecting_errors = 0;
		status.bit_field.l2_status = L2_Listening;
	        status_baud = listen_baudrate;

		if(status_handler)
			status_handler(L2_STATUS_MASK, status, status_baud);

#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Restart timer\n\r");
#endif
#endif
		result = sw_timer_start(SECONDS_TO_TICKS(CAN_BAUD_AUTO_DETECT_LISTEN_PERIOD),
				     exp_check_network_connection,
				     (union sigval)(void *)NULL,
				     &listen_timer);
		if(result != SUCCESS) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			LOG_E("Failed to start listen timer, result 0x%x\n\r", result);
#endif
#endif
		}
	}
}

#endif // #if defined(SYS_CAN_BAUD_AUTO_DETECT)
