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
        
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "set_modbus_awaiting_response_state()\n\r");
#endif
	channel->rx_write_index = 0;

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = process_timer_35_expiry;
	channel->transmit = NULL;
	channel->modbus_tx_finished = NULL;
	channel->process_rx_character = process_rx_character;
	channel->process_response_timeout = process_response_timeout;

	rc = start_response_timer(channel);
        
        if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
                log_e(TAG, "Failed to start response timer\n\r");
#endif
        }
}

void process_timer_35_expiry(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;
        
	uint8_t  start_index;
//	uint16_t loop;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "process_timer_35_expiry()\n\r");
#endif
//	RX_ISR_ENABLE = 0;
	set_modbus_idle_state(channel);

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "process_timer_35_expiry() channel %d msg length %d\n\r", channel->uart->uart, channel->rx_write_index);
#endif

//	for(loop = 0; loop < channel->rx_write_index; loop++) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
//		log_d(TAG, "Char %d - 0x%x\n\r", loop, channel->rx_buffer[loop]);
#endif
//	}
	if(channel->rx_write_index > 2) {
		if(channel->rx_buffer[0] == channel->address) {
			start_index = 0;
		} else if (channel->rx_buffer[1] == channel->address) {
			start_index = 1;
		} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "message from wrong address channel Address 0x%x\n\r", channel->address);
#endif
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "channel->rx_buffer[0] = 0x%x\n\r", channel->rx_buffer[0]);
#endif
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "channel->rx_buffer[1] = 0x%x\n\r", channel->rx_buffer[1]);
#endif
			return;
		}

		if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
			/*
			 * Response Good
			 * Subtract 2 for the CRC
			 */
			channel->process_response(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2), channel->response_callback_data);
		} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Message bad!\n\r");
#endif
			channel->process_response(NULL, 0, channel->response_callback_data);
		}
	} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Message too short\n\r");
#endif
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
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "UART 2 Overflow: Line too long\n\r");
#endif
	}
}

static void process_response_timeout(struct modbus_channel *channel)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "process_response_timeout()\n\r");
#endif
	set_modbus_starting_state(channel);
	if(channel->process_response) {
		channel->process_response(NULL, 0, channel->response_callback_data);
	} else {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "No response Function\n\r");
#endif
	}
}
