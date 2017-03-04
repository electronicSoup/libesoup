#define TAG "MODBUS_AWAITING_RESPONSE"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(void *);
static void process_rx_character(struct modbus_channel *channel, uint8_t ch);
static void process_response_timeout(struct modbus_channel *channel);

void set_modbus_awaiting_response_state(struct modbus_channel *channel)
{
        result_t rc;
        
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "set_modbus_awaiting_response_state()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
	channel->rx_write_index = 0;

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = process_timer_35_expiry;
	channel->transmit = NULL;
	channel->modbus_tx_finished = NULL;
	channel->process_rx_character = process_rx_character;
	channel->process_response_timeout = process_response_timeout;

	rc = start_response_timer(channel);
        
        if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
                log_e(TAG, "Failed to start response timer\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
        }
}

void process_timer_35_expiry(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;
        
	uint8_t  start_index;
//	uint16_t loop;

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "process_timer_35_expiry()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
//	RX_ISR_ENABLE = 0;
	set_modbus_idle_state(channel);

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "process_timer_35_expiry() channel %d msg length %d\n\r", channel->uart->uart, channel->rx_write_index);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
	if(channel->rx_write_index > 2) {
		if(channel->rx_buffer[0] == channel->address) {
			start_index = 0;
		} else if (channel->rx_buffer[1] == channel->address) {
			start_index = 1;
		} else {
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "message from wrong address channel Address 0x%x\n\r", channel->address);
			log_d(TAG, "channel->rx_buffer[0] = 0x%x\n\r", channel->rx_buffer[0]);
			log_d(TAG, "channel->rx_buffer[1] = 0x%x\n\r", channel->rx_buffer[1]);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
			return;
		}

		if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
			/*
			 * Response Good
			 * Subtract 2 for the CRC
			 */
			channel->process_response(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2), channel->response_callback_data);
		} else {
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Message bad!\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
			channel->process_response(NULL, 0, channel->response_callback_data);
		}
	} else {
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Message too short\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
		channel->process_response(NULL, 0, channel->response_callback_data);
	}
}

void process_rx_character(struct modbus_channel *channel, uint8_t ch)
{
	if ((channel->rx_write_index == 0) && (ch == 0x00)) {
		return;
	}
	cancel_response_timer(channel);
	start_35_timer(channel);

	channel->rx_buffer[channel->rx_write_index++] = ch;

	if (channel->rx_write_index == SYS_MODBUS_RX_BUFFER_SIZE) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "UART 2 Overflow: Line too long\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
	}
}

static void process_response_timeout(struct modbus_channel *channel)
{
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "process_response_timeout()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
	set_modbus_starting_state(channel);
	if(channel->process_response) {
		channel->process_response(NULL, 0, channel->response_callback_data);
	} else {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "No response Function\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
	}
}
