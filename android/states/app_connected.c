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
#define TAG "AppConnected"
#endif

void app_connected_process_msg(android_command_t, void *, UINT16);
void app_connected_main(void);
void app_connected_process_usb_event(USB_EVENT event);

static void transmit_ready(void);
static void transmit_app_type_info(void);
static void transmit_hardware_info(void);
static void transmit_bootcode_info(void);
static void transmit_firmware_info(void);
#ifdef NODE
static void transmit_node_config_info(void);
static void update_node_config_info(char *buffer);
#endif //NODE
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

void app_connected_process_msg(android_command_t cmd, void *data, UINT16 data_len)
{
    UINT32 address;
    BYTE *byte_data;
    UINT8 loop;
    UINT16 len;

    DEBUG_D("app_connected_process_msg data lenght %d\n\r", data_len);
    switch(cmd) {
#ifdef BOOT
        case COMMAND_BOOT:
            DEBUG_D("COMMAND_BOOT Jump to application\n\r");
            jmp_firmware();
            break;
#endif //BOOT

#ifdef BOOT
        case COMMAND_BEGIN_FLASH:
            DEBUG_D("COMMAND_BEGIN_FLASH transmit ready\n\r");
            transmit_ready();
            break;
#endif //BOOT

#ifdef BOOT
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
#endif //BOOT

#ifdef BOOT
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
#endif //BOOT

        case ANDROID_APP_TYPE_REQ:
            DEBUG_D("ANDROID_APP_TYPE_REQ\n\r");
            transmit_app_type_info();
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

#ifdef NODE
        case NODE_CONFIG_INFO_REQ:
            DEBUG_D("NODE_CONFIG_INFO_REQ\n\r");
            transmit_node_config_info();
            break;
#endif //NODE

#ifdef NODE
        case NODE_CONFIG_UPDATE:
            byte_data = (BYTE *) data;
            DEBUG_D("NODE_CONFIG_UPDATE\n\r");
            DEBUG_D("data[0] = 0x%x\n\r", byte_data[0]);
            DEBUG_D("data[1] = 0x%x\n\r", byte_data[1]);
            DEBUG_D("data[2] = 0x%x\n\r", byte_data[2]);
            DEBUG_D("descrioption String is %s\n\r", &byte_data[3]);

            eeprom_write(L3_NODE_ADDRESS, byte_data[0]);
            eeprom_write(BAUD_RATE, byte_data[1]);
            eeprom_write(IO_ADDRESS, byte_data[2]);

            len = eeprom_str_write(NODE_DESCRIPTION, &byte_data[3]);
            DEBUG_D("%d bytes written to EEPROM\n\r", len);
            break;
#endif //NODE

        default:
            DEBUG_W("Unprocessed message 0x%x\n\r", cmd);
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

void transmit_app_type_info(void)
{
    char buffer[3];

    DEBUG_D("transmit_app_type_info()\n\r");

    buffer[0] = 2;
    buffer[1] = ANDROID_APP_TYPE_RESP;
#if defined(BOOT)
    buffer[2] = BOOTLOADER_APP;
#elif defined(NODE)
    buffer[2] = NODE_CONFIG_APP;
#endif
    android_transmit(buffer, 3);
}
void transmit_hardware_info(void)
{
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

#ifdef NODE
void transmit_node_config_info(void)
{
    char buffer[55];
    UINT16 index;
    UINT16 len;
    BYTE value;

    buffer[1] = NODE_CONFIG_INFO_RESP;

    index = 2;

    eeprom_read(L3_NODE_ADDRESS, (BYTE *) &value);
    DEBUG_D("Layer 3 Node Address 0x%x\n\r", value);
    buffer[index++] = value;

    eeprom_read(BAUD_RATE, (BYTE *) &value);
    DEBUG_D("CAN Baud Rate 0x%x\n\r", value);
    buffer[index++] = value;

    eeprom_read(IO_ADDRESS, (BYTE *) &value);
    DEBUG_D("I/O Address 0x%x\n\r", value);
    buffer[index++] = value;

    len = eeprom_str_read(NODE_DESCRIPTION, &buffer[index], 50);
    DEBUG_D("Description: String Length %d string '%s'\n\r", len, &buffer[index]);

    index = index + len;
    buffer[0] = index;

    android_transmit((BYTE *)buffer, index);
}
#endif //NODE

#ifdef NODE
void update_node_config_info(char *buffer)
{
    char string[50];
    UINT16 index;
    BYTE value;
#if 0
    eeprom_read(L3_NODE_ADDRESS, (BYTE *) &value);
    DEBUG_D("Layer 3 Node Address 0x%x\n\r", value);
    buffer[index++] = value;

    eeprom_read(BAUD_RATE, (BYTE *) &value);
    DEBUG_D("CAN Baud Rate 0x%x\n\r", value);
    buffer[index++] = value;

    eeprom_read(IO_ADDRESS, (BYTE *) &value);
    DEBUG_D("I/O Address 0x%x\n\r", value);
    buffer[index++] = value;

    eeprom_str_read(NODE_DESCRIPTION, string, 50);
    DEBUG_D("Description: %s\n\r", string);
    strcpy(buffer[index], string);

    index = index + strlen(string);
    buffer[0] = index;

    android_transmit((BYTE *)buffer, index);
#endif // 0
}
#endif //NODE
