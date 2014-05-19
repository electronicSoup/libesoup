/**
 *
 * \file os.c
 *
 * Functions provided by the OS to the Application.
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "system.h"
#include <stdio.h>
#include <string.h>

#include "es_lib/timers/timer_sys.h"
#include "es_lib/utils/utils.h"
#define DEBUG_FILE
#include "es_lib/logger/serial.h"
#undef DEBUG_FILE
#include "os_api.h"

#define TAG "OS"

BOOL app_valid;

typedef struct {
    BOOL active;
    expiry_function function;
} os_timer_t;

os_timer_t os_timers[NUMBER_OF_TIMERS];

void os_init_data(void)
{
    u16 loop;

    for(loop = 0; loop < NUMBER_OF_TIMERS; loop++) {
        os_timers[loop].active = FALSE;
    }
}

void exp_os_timer(timer_t timer_id, union sigval data)
{
    DEBUG_D("exp_os_timer()\n\r");
    if(os_timers[timer_id].active) {
        os_timers[timer_id].active = FALSE;
        os_timers[timer_id].function(timer_id, data);
    } else {
        DEBUG_E("Inactive timer expired!\n\r");
    }
}

/*
** start_timer
 */
result_t os_start_timer(UINT16 ticks,
        expiry_function function,
        union sigval data,
        es_timer *timer)
{
    DEBUG_D("os_start_timer()\n\r");
    start_timer(ticks, exp_os_timer, data, timer);

    os_timers[timer->timer_id].active = TRUE;
    os_timers[timer->timer_id].function = function;
    return(SUCCESS);
}

/*
** os_cancel_timer
*/
result_t os_cancel_timer(es_timer *timer)
{
    DEBUG_D("os_cancel_timer()\n\r");
    os_timers[timer->timer_id].active = FALSE;
    cancel_timer(timer);
    return(SUCCESS);
}

/*
** os_cancel_all_timers
*/
result_t os_cancel_all_timers(void)
{
    u8 loop;
    es_timer timer;

    for(loop = 0; loop < NUMBER_OF_TIMERS; loop++) {
        if(os_timers[loop].active) {
            os_timers[loop].active = FALSE;
            timer.status = ACTIVE;
            timer.timer_id = loop;

            cancel_timer(&timer);
        }
    }
    return(SUCCESS);
}

/*
** os_eeprom_read
*/
result_t os_eeprom_read(UINT16 address, BYTE *data)
{
    UINT16   eeprom_address;
    
    eeprom_address = APP_EEPROM_START + address; 
    if(eeprom_address <= EEPROM_MAX_ADDRESS) {
        return(eeprom_read(eeprom_address, data));
    } else {
        return(ERR_ADDRESS_RANGE);
    }
}

/*
** os_eeprom_write
*/
result_t os_eeprom_write(u16 address, BYTE data)
{
    u16   eeprom_address;

    eeprom_address = APP_EEPROM_START + address;
    if(eeprom_address <= EEPROM_MAX_ADDRESS) {
        return(eeprom_write(eeprom_address, data));
    } else {
        return(ERR_ADDRESS_RANGE);
    }
}

void os_clear_app_eeprom(void)
{
    u16 loop;

    for(loop = APP_EEPROM_START; loop <= EEPROM_MAX_ADDRESS; loop++) {
        asm ("CLRWDT");
        eeprom_write(loop, 0x00);
    }

}

/*
** os_l2_tx_frame
*/
result_t os_l2_tx_frame()
{
    return(SUCCESS);
}

/*
** os_l2_reg_handler
*/
result_t os_l2_reg_handler()
{
    return(SUCCESS);
}

/*
** os_l2_unreg_handler
*/
result_t os_l2_unreg_handler()
{
    return(SUCCESS);
}

/*
** os_l3_get_address
*/
result_t os_l3_get_address()
{
    return(SUCCESS);
}

/*
** os_l3_tx_msg
*/
result_t os_l3_tx_msg()
{
    return(SUCCESS);
}

/*
** os_l3_reg_handler
*/
result_t os_l3_reg_handler()
{
    return(SUCCESS);
}

/*
** os_l3_unreg_handler
*/
result_t os_l3_unreg_handler()
{
    return(SUCCESS);
}

/*
** os_node_log
*/
result_t os_node_log(log_level_t level, char* tag, char* fmt, ...)
{
    va_list args;
    UINT16 psv_page;
    auto char colon[] = ":";

    switch (level)
    {
        case Debug:
            printf("App D :");
            break;

        case Info:
            printf("App I :");
            break;

        case Warning:
            printf("App W :");
            break;

        case Error:
            printf("App*E*:");
            break;
    }

    CORCONbits.PSV = 1;
    psv_page = PSVPAG;
    PSVPAG = 0x03;

    printf(tag);
    printf(colon);

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    PSVPAG = psv_page;
    return(SUCCESS);
}

/*
** os_net_log
*/
result_t os_net_log()
{
    return(SUCCESS);
}

/*
** os_net_reg_handler
*/
result_t os_net_reg_handler()
{
    return(SUCCESS);
}

/*
** os_net_unreg_handler
*/
result_t os_net_unreg_handler()
{
    return(SUCCESS);
}

/*
** os_invalidate_app
*/
result_t os_invalidate_app_isr(void)
{
    DEBUG_D("os_invalidate_app()\n\r");
    eeprom_write(APP_VALID_MAGIC_ADDR, 0x00);
    eeprom_write(APP_VALID_MAGIC_ADDR + 1, 0x00);
    app_valid = FALSE;
    return(SUCCESS);
}
