#include "libesoup_config.h"

#ifdef SYS_CAN_PING_PROTOCOL

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
const char *TAG = "CAN_PING";
#endif // SYS_SERIAL_LOGGING

#include "libesoup/logger/serial_log.h"
#include "libesoup/timers/timers.h"

/**
 * \brief Network Idle functionality
 *
 * We need to have CAN Bus Network traffic so that when devices connect
 * to listen to the Network to establish the CAN Bus Baud Rate. As a result
 * we'll keep a timer and if nothing has been received or transmitted in this
 * time we'll fire a ping message.
 */
static u16 networkIdleDuration = 0;
static es_timer networkIdleTimer;
//static can_status_t canStatus;

/*
 * Idle duration before sending a Ping Message. Initialised to a random value
 * on powerup.
 */
static uint16_t ping_time;
//static result_t send_ping(void);


static void     exp_test_ping(timer_t timer_id, union sigval data);
static void     restart_ping_timer(void);
static uint8_t ping_timer;





void ping_network(u8 *);

void ping_init(void)
{
	// Create a random timer between 1 and 1.5 seconds for firing the
	// Network Idle Ping message
	networkIdleDuration = (u16) ((rand() % 500) + 1000);

	TIMER_INIT(networkIdleTimer)

#if DEBUG_LEVEL <= LOG_DEBUG
	LOG_D("Network Idle Duration set to %d milliSeconds\n\r", networkIdleDuration);
#endif
//	networkIdleTimer = start_timer(networkIdleDuration, ping_network, NULL);

	ping_time = (uint16_t)((rand() % SECONDS_TO_TICKS(1)) + (SECONDS_TO_TICKS(CAN_PING_PROTOCOL_PERIOD) - MILLI_SECONDS_TO_TICKS(500)));
        restart_ping_timer();

	// Create a random timer for firing the
	// Network Idle Ping message
	frame.can_id = CAN_DCNCP_NodePingMessage;
	frame.can_dlc = 0;

	can_l2_tx_frame(&frame);
}

void ping_network(u8 *data)
{
	LOG_D("Network Idle Expired so send a ping message and restart\n\r");
//	networkIdleTimer = start_timer(networkIdleDuration, pingNetwork, NULL);
	send_ping_message();
}

void exp_test_ping(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("exp_test_ping()\n\r");
#endif
#endif
	dcncp_send_ping();
        restart_ping_timer();
}

void restart_ping_timer(void)
{
	if(ping_timer.status == ACTIVE) {
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//		LOG_D("Cancel running ping timer\n\r");
#endif
#endif
 		if(timer_cancel(&ping_timer) != SUCCESS) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			LOG_E("Failed to cancel the Ping timer\n\r");
#endif
#endif
			return;
		}
	}

	sw_timer_start(ping_time, exp_test_ping, (union sigval)(void *) NULL, &ping_timer);
}

#endif // #ifdef SYS_CAN_PING_PROTOCOL
