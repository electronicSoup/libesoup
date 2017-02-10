#define TAG "MODBUS_IDLE"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void transmit(struct modbus_channel *channel, uint8_t *data, uint16_t len, modbus_response_function fn, void* callback_data);
static void process_timer_35_expiry(void *data);
static void process_rx_character(struct modbus_channel *channel, uint8_t ch);

void set_modbus_idle_state(struct modbus_channel *channel)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "set_modbus_idle_state(channel %d)\n\r", channel->uart->uart);
#endif

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = process_timer_35_expiry;
	channel->transmit = transmit;
        channel->rx_write_index = 0;
	channel->modbus_tx_finished = NULL;
	channel->process_rx_character = process_rx_character;
	channel->process_response_timeout = NULL;

	if(channel->idle_callback) {
		channel->idle_callback(channel->idle_callback_data);
	}
}

void transmit(struct modbus_channel *channel, uint8_t *data, uint16_t len, modbus_response_function fn, void *callback_data)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Modbus Idle state Transmit(%d)\n\r", channel->uart->uart);
#endif

	/*
	 * The response timeout timer is started when the transmission is
	 * completed in the modbus_awaiting_response state.
	 */
	channel->address = data[0];
	channel->process_response = fn;
	channel->response_callback_data = callback_data;
	set_modbus_transmitting_state(channel);
	modbus_tx_data(channel, data, len);
}

void process_timer_35_expiry(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;

	uint8_t  start_index;
	uint16_t loop;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "process_timer_35_expiry() channel %d msg length %d\n\r", channel->uart->uart, channel->rx_write_index);
#endif

//	for(loop = 0; loop < channel->rx_write_index; loop++) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
//		log_d(TAG, "Char %d - 0x%x\n\r", loop, channel->rx_buffer[loop]);
#endif
//	}
        start_index = 0;
        if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
                /*
                 * Response Good
                 * Subtract 2 for the CRC
                 */
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
                log_d(TAG, "Message Good! Start at 0\n\r");
#endif

                if(channel->process_unsolicited_msg) {
                        channel->process_unsolicited_msg(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2), channel->response_callback_data);
                }
                channel->rx_write_index = 0;
                return;
        }

        start_index = 1;
        if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
                /*
                 * Response Good
                 * Subtract 2 for the CRC
                 */
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
                log_d(TAG, "Message Good! Start at 1\n\r");
#endif

                if(channel->process_unsolicited_msg) {
                        channel->process_unsolicited_msg(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2), channel->response_callback_data);
                }
                channel->rx_write_index = 0;
                return;
        }

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        log_d(TAG, "Message bad!\n\r");
#endif
	for(loop = 0; loop < channel->rx_write_index; loop++) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Char %d - 0x%x\n\r", loop, channel->rx_buffer[loop]);
#endif
	}
        if(channel->process_unsolicited_msg) {
                channel->process_unsolicited_msg(NULL, 0, channel->response_callback_data);
        }
        channel->rx_write_index = 0;
}

void process_rx_character(struct modbus_channel *channel, uint8_t ch)
{
	if ((channel->rx_write_index == 0) && (ch == 0x00)) {
		return;
	}

	start_35_timer(channel);

	channel->rx_buffer[channel->rx_write_index++] = ch;

	if (channel->rx_write_index == SYS_MODBUS_RX_BUFFER_SIZE) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "UART 2 Overflow: Line too long\n\r");
#endif
	}
}

