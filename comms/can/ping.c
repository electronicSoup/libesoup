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
}

void ping_network(u8 *data)
{
	LOG_D("Network Idle Expired so send a ping message and restart\n\r");
//	networkIdleTimer = start_timer(networkIdleDuration, pingNetwork, NULL);
	send_ping_message();
}

#endif // #ifdef SYS_CAN_PING_PROTOCOL
