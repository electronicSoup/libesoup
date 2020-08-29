#define DEBUG_FILE
#define TAG  "OKK_Main"

#include "libesoup_config.h"
#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/change_notification.h"
#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/comms/usb/keyboard/usb_keyboard.h"

#define DEBOUNCE_DURATION_uS   150

static timer_id          bounce_timer;
static struct timer_req  bounce_tmr_request;
static uint8_t  port_d;

void bounce_expiry(timer_id timer, union sigval data)
{
	static uint16_t  ticks_0 = 0;
	uint16_t ticks_diff;
	uint8_t new_port_d = PORTD;

	bounce_timer = BAD_TIMER_ID;
	
	if ((port_d & 0x01) != (new_port_d & 0x01)) {
		port_d = new_port_d;

		if(!(new_port_d & 0x01)) {
			ticks_0 = current_system_ticks();
		} else {
			ticks_diff = current_system_ticks() - ticks_0;
			
			if (ticks_diff > 100) {
				send_key(44, TRUE);
				LOG_D("Long - %d\n\r", ticks_diff);
			} else {
				send_key(44, FALSE);
				LOG_D("Short - %d\n\r", ticks_diff);
			}
		}
	}
	
}

void rd0_change(enum gpio_pin pin) {
	if (bounce_timer == BAD_TIMER_ID) {
		bounce_timer = hw_timer_start(&bounce_tmr_request);
	}
}
int main(void)
{
	result_t rc;
	
	rc = libesoup_init();

	port_d                             = PORTD;
	bounce_timer                       = BAD_TIMER_ID;
	bounce_tmr_request.period.units    = uSeconds;
	bounce_tmr_request.period.duration = DEBOUNCE_DURATION_uS;
	bounce_tmr_request.type            = single_shot_expiry;
	bounce_tmr_request.data.sival_int  = 0;
	bounce_tmr_request.exp_fn          = bounce_expiry;

	rc = gpio_set(RD0, GPIO_MODE_DIGITAL_INPUT, 0);
	rc = change_notifier_register(RD0, rd0_change);	
	LOG_D("Entering main Loop\n\r");
	while(1) {
		libesoup_tasks();
	}
	return(0);
}