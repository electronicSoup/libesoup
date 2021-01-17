/**
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
 *
 */

/*
 * Set up the configuration words of the processor:
 */

#include "libesoup_config.h"

#include "libesoup/timers/delay.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/comms/uart/uart.h"
#include "libesoup/comms/midi/midi.h"
#include "ditto.h"

#define DEBUG_FILE
static const char *TAG = "MIDI";
#include "libesoup/logger/serial_log.h"

#if defined(SYS_MIDI_TX) || defined(SYS_MIDI_RX)
static struct uart_data midi_uart;

#ifdef MIDI_TX
static void expiry(timer_id timer_id, union sigval);
#endif // MIDI_TX

#ifdef SYS_MIDI_RX
static void process_midi(uint8_t uart_id, uint8_t ch)
{
	result_t rc;

	serial_printf("MIDI:0x%x\n\r", ch);
	if (ch == 0x90) {
		LOG_D("On\n\r");
		rc = ditto_pulse();
	} else if (ch == 0x80) {
		LOG_D("Off\n\r");
		rc = ditto_pulse();
	}
}
#endif  // MIDI_RX

result_t midi_reserve(struct midi_data *data)
{
	result_t         rc;

	LOG_D("midi_reserve()\n\r");
	midi_uart.rx_pin = data->rx_pin;
#ifndef SYS_MIDI_TX
	midi_uart.tx_pin = INVALID_GPIO_PIN;
#else
	midi_uart.tx_pin = data->tx_pin;
#endif
	rc = uart_calculate_mode(&midi_uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	if (rc < 0) {
		LOG_E("Failed to calculate\n\r")
		return(rc);
	}
	midi_uart.baud             = 31250;
	midi_uart.tx_finished      = NULL;
#ifdef SYS_MIDI_RX
	midi_uart.process_rx_char  = process_midi;
#else
	midi_port.process_rx_char  = NULL;
#endif
	rc = uart_reserve(&midi_uart);
	if (rc < 0) {
		LOG_E("Failed to reserver a UART (%d)\n\r", rc);
	}
	return(rc);
}

#if 0
int main(void)
{
        result_t         rc;
#ifdef MIDI_TX
	struct timer_req midi_tx_request;
	timer_id         midi_tx_timer;
#endif
#ifdef DITTO
	struct timer_req ditto_request;
	timer_id         ditto_timer;
#endif
	rc = libesoup_init();
	if (rc < 0) {
		LOG_E("Failed to init library\n\r");
	}

#if defined(MIDI_TX) || defined(MIDI_RX)
	rc = init_midi_port();
	if (rc < 0) {
		LOG_E("Failed to init MIDI Port\n\r");
	}
#endif
#ifdef DITTO
	rc = gpio_set(RD2, GPIO_MODE_DIGITAL_OUTPUT, 0);
	if (rc < 0) {
		LOG_E("Failed to set RD2\n\r");
	}

	ditto_request.period.units    = Seconds;
	ditto_request.period.duration = 10;
	ditto_request.type            = repeat_expiry;
	ditto_request.exp_fn          = ditto_expiry;
	ditto_request.data.sival_int  = 0;

        ditto_timer = sw_timer_start(&ditto_request);

        if(ditto_timer < 0) {
		LOG_E("Failed to start ditto timer\n\r");
        }
#endif
#ifdef MIDI_TX
	midi_tx_request.period.units    = Seconds;
	midi_tx_request.period.duration = 10;
	midi_tx_request.type            = repeat_expiry;
	midi_tx_request.exp_fn          = expiry;
	midi_tx_request.data.sival_int  = 0;

        midi_tx_timer = sw_timer_start(&midi_tx_request);

        if(midi_tx_timer < 0) {
		LOG_E("Failed to init MIDI Port\n\r");
        }
#endif
	LOG_D("Entering main loop\n\r");
        while(1) {
		libesoup_tasks();
        }
        return 0;
}
#endif //0

#ifdef MIDI_TX
static void expiry(timer_id timer  __attribute__((unused)), union sigval data __attribute__((unused)))
{
	result_t  rc;
	uint8_t   buffer[5];
        struct period    period;

	LOG_D("Expiry\n\r");

	buffer[0] = 0x91;
	buffer[1] = 0x3c;
	buffer[2] = 0x22;
	rc = uart_tx_buffer(&midi_port, (uint8_t *)&buffer, 3);
	if (rc < 0) {
		LOG_E("Failed to Tx on MIDI Port\n\r");
	}

	period.units    = mSeconds;
	period.duration = 500;
	rc = delay(&period);
	if (rc < 0) {
		LOG_E("Failed to Delay\n\r");
	}

	buffer[0] = 0x81;
	buffer[1] = 0x3c;
	buffer[2] = 0x29;
	rc = uart_tx_buffer(&midi_port, (uint8_t *)&buffer, 3);
	if (rc < 0) {
		LOG_E("Failed to Tx on MIDI Port\n\r");
	}
}
#endif

#ifdef DITTO
static void ditto_expiry(timer_id timer  __attribute__((unused)), union sigval data __attribute__((unused)))
{
	result_t         rc;
        struct period    period;

	LOG_D("DITTO Expiry\n\r");

	rc = gpio_set(RD2, GPIO_MODE_DIGITAL_OUTPUT, 1);
	if (rc < 0) {
		LOG_E("Failed to set RD2\n\r");
	}

	period.units    = mSeconds;
	period.duration = 10;
	rc = delay(&period);
	if (rc < 0) {
		LOG_E("Failed to Delay\n\r");
	}

	rc = gpio_set(RD2, GPIO_MODE_DIGITAL_OUTPUT, 0);
	if (rc < 0) {
		LOG_E("Failed to set RD2\n\r");
	}
}
#endif

#endif
