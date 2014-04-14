/**
 *
 * \file states/appConnected.c
 *
 * The Android Application Connected state for the Android Communications.
 *
 * State for processing comms from the Android Application.
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
#include "usb/usb.h"
#include "usb/usb_host_android.h"
#include "ipc.h"
#include "states.h"
#include "es_lib/utils/utils.h"
#include "es_lib/android/android.h"
#include "main.h"

#define DEBUG_FILE
#include "es_lib/logger/serial.h"

#if LOG_LEVEL < NO_LOGGING
#define TAG "Boot App"
#endif

void app_connected_process_msg(android_command_t, void *);
void app_connected_main(void);
void app_connected_process_usb_event(USB_EVENT event);

static void transmit_ready(void);
static void transmit_hardware_info(void);
static void transmit_bootcode_info(void);
static void transmit_firmware_info(void);
#ifdef BOOT
extern void jmp_firmware(void);
#endif //BOOT
void set_app_connected_state(void)
{
    DEBUG_D("App Connected State\n\r");
    // Android App connected so cancel the timer. 
    // Android App now controls booting
    T2CONbits.TON = 0;

    current_state.process_msg = app_connected_process_msg;
    current_state.main = app_connected_main;
    current_state.process_usb_event = app_connected_process_usb_event;
}

void app_connected_process_msg(android_command_t cmd, void *data)
{
    UINT32 address;
    BYTE *byte_data;
    UINT8 loop;

    switch(cmd) {
        case COMMAND_BOOT:
            DEBUG_D("COMMAND_BOOT Jump to application\n\r");
#ifdef BOOT
            jmp_firmware();
#endif //BOOT
            break;

        case COMMAND_BEGIN_FLASH:
            DEBUG_D("COMMAND_BEGIN_FLASH transmit ready\n\r");
            transmit_ready();
            break;

        case COMMAND_ERASE:
            if(data != NULL) {
                byte_data = (BYTE *) data;

                address = 0x00;
                for (loop = 0; loop < 4; loop++) {
                    address = (address << 8) | (byte_data[loop] & 0xff);
                }
                DEBUG_I("COMMAND_ERASE %lx\n\r", address);
                if(!flash_page_empty(address)) {
                    flash_erase(address);
                }
                transmit_ready();
            }
            break;

        case COMMAND_ROW:
//            DEBUG_D("COMMAND_ROW\n\r");
            if(data != NULL) {
                byte_data = (BYTE *) data;

                address = 0x00;
                for (loop = 0; loop < 4; loop++) {
                    address = (address << 8) | (byte_data[loop] & 0xff);
                }

                flash_write(address, &byte_data[4]);
                transmit_ready();
            }
            break;

        case HARDWARE_INFO_REQ:
            DEBUG_D("HARDWARE_INFO_REQ\n\r");
            transmit_hardware_info();
            break;

        case BOOTCODE_INFO_REQ:
            DEBUG_D("BOOTCODE_INFO_REQ\n\r");
            transmit_bootcode_info();
            break;

        case FIRMWARE_INFO_REQ:
            DEBUG_D("APP_INFO_REQ\n\r");
            transmit_firmware_info();
            break;

        default:
            DEBUG_W("Unprocessed message\n\r");
            break;
    }
}

void app_connected_main()
{
}

void app_connected_process_usb_event(USB_EVENT event)
{
    switch (event) {
        case EVENT_ANDROID_ATTACH:
            break;

        case EVENT_ANDROID_DETACH:
            set_idle_state();
            break;
    }
}

static void transmit_ready(void)
{
    BYTE buffer[2];

    buffer[0] = 1;
    buffer[1] = COMMAND_READY;

    android_transmit(buffer, 2);
}

void transmit_hardware_info(void)
{
//    char string[50];
    char buffer[160];
    BYTE index = 0;

    DEBUG_D("transmit_hardware_info()\n\r");

    buffer[1] = HARDWARE_INFO_RESP;

    index = 2;
    index += strcpypgmtoram(&buffer[index], hardware_manufacturer, 24) + 1;
    index += strcpypgmtoram(&buffer[index], hardware_model, 24) + 1;
    index += strcpypgmtoram(&buffer[index], hardware_description, 50) + 1;
    index += strcpypgmtoram(&buffer[index], hardware_version, 10) + 1;
    index += strcpypgmtoram(&buffer[index], hardware_uri, 50) + 1;
#if 0
    strcpypgmtoram(string, hardware_manufacturer, 24);
    DEBUG_D("HW Manufacturer - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;

    string[0] = 0x00;
    strcpypgmtoram(string, hardware_model, 24);
    DEBUG_D("HW Model - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;

    string[0] = 0x00;
    strcpypgmtoram(string, hardware_description, 50);
    DEBUG_D("HW Description - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;

    string[0] = 0x00;
    strcpypgmtoram(string, hardware_version, 10);
    DEBUG_D("HW Version - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;

    string[0] = 0x00;
    strcpypgmtoram(string, hardware_uri, 50);
    DEBUG_D("HW uri - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;
#endif
    DEBUG_D("Transmit boot info length %d\n\r", index);
    buffer[0] = index;

    android_transmit((BYTE *)buffer, index);
}

void transmit_bootcode_info(void)
{
//    char string[50];
    char buffer[152];
    BYTE index = 0;

    buffer[1] = BOOTCODE_INFO_RESP;

    index = 2;
    index += strcpypgmtoram(&buffer[index], bootcode_author, 40) + 1;
    index += strcpypgmtoram(&buffer[index], bootcode_description, 50) + 1;
    index += strcpypgmtoram(&buffer[index], bootcode_version, 10) + 1;
    index += strcpypgmtoram(&buffer[index], bootcode_uri, 50) + 1;
#if 0
    strcpypgmtoram(string, bootcode_author, 40);
    DEBUG_D("Bootcode Author - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;

    string[0] = 0x00;
    strcpypgmtoram(string, bootcode_description, 50);
    DEBUG_D("Bootcode Description - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;

    string[0] = 0x00;
    strcpypgmtoram(string, bootcode_version, 10);
    DEBUG_D("Bootcode Version - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;

    string[0] = 0x00;
    strcpypgmtoram(string, bootcode_uri, 50);
    DEBUG_D("Bootcode uri - %s\n\r", string);
    strcpy(&buffer[index], string);
    index += strlen(string) + 1;
#endif
    DEBUG_D("Transmit boot info length %d\n\r", index);
    buffer[0] = index;

    android_transmit((BYTE *)buffer, index);
}

void transmit_firmware_info(void)
{
    char buffer[152];
    UINT16 index = 0;
    buffer[1] = FIRMWARE_INFO_RESP;

    index = 2;

    index += strcpypgmtoram(&buffer[index], firmware_author, 40) + 1;
    index += strcpypgmtoram(&buffer[index], firmware_description, 50) + 1;
    index += strcpypgmtoram(&buffer[index], firmware_version, 10) + 1;
    index += strcpypgmtoram(&buffer[index], firmware_uri, 50) + 1;
    DEBUG_D("Transmit Firmware info length %d\n\r", index);
    buffer[0] = index;

    android_transmit((BYTE *)buffer, index);
}
