#define TAG "MODBUS_IDLE"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void transmit(struct modbus_channel *channel, u8 *data, u16 len, modbus_response_function fn, void* callback_data);
static void process_timer_35_expiry(void *data);
static void process_rx_character(struct modbus_channel *channel, u8 ch);

void set_modbus_idle_state(struct modbus_channel *channel)
{
	LOG_D("set_modbus_idle_state(channel %d)\n\r", channel->uart->uart);

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

void transmit(struct modbus_channel *channel, u8 *data, u16 len, modbus_response_function fn, void *callback_data)
{
	LOG_D("Modbus Idle state Transmit(%d)\n\r", channel->uart->uart);

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

	u8  start_index;
	u16 loop;

	LOG_D("process_timer_35_expiry() channel %d msg length %d\n\r", channel->uart->uart, channel->rx_write_index);

//	for(loop = 0; loop < channel->rx_write_index; loop++) {
//		LOG_D("Char %d - 0x%x\n\r", loop, channel->rx_buffer[loop]);
//	}
        start_index = 0;
        if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
                /*
                 * Response Good
                 * Subtract 2 for the CRC
                 */
                LOG_D("Message Good! Start at 0\n\r");

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
                LOG_D("Message Good! Start at 1\n\r");

                if(channel->process_unsolicited_msg) {
                        channel->process_unsolicited_msg(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2), channel->response_callback_data);
                }
                channel->rx_write_index = 0;
                return;
        }

        LOG_D("Message bad!\n\r");
	for(loop = 0; loop < channel->rx_write_index; loop++) {
		LOG_D("Char %d - 0x%x\n\r", loop, channel->rx_buffer[loop]);
	}
        if(channel->process_unsolicited_msg) {
                channel->process_unsolicited_msg(NULL, 0, channel->response_callback_data);
        }
        channel->rx_write_index = 0;
}

void process_rx_character(struct modbus_channel *channel, u8 ch)
{
	if ((channel->rx_write_index == 0) && (ch == 0x00)) {
		return;
	}

	start_35_timer(channel);

	channel->rx_buffer[channel->rx_write_index++] = ch;

	if (channel->rx_write_index == MODBUS_RX_BUFFER_SIZE) {
		LOG_E("UART 2 Overflow: Line too long\n\r");
	}
}

