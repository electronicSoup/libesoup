/**
 *
 * \file libesoup/can/dcncp/cinnamonbun_info.h
 *
 * Function prototypes for retrieving the CinnamonBun Info Strings
 *
 * Copyright 2017 electronicSoup Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU Lesser General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#define HARDWARE_INFO_BASE 0x200

#define HW_MANUFACTURER_24_ADDRESS       (HARDWARE_INFO_BASE)
#define HW_MODEL_24_ADDRESS              (HW_MANUFACTURER_24_ADDRESS + 24)
#define HW_DESCRIPTION_50_ADDRESS        (HW_MODEL_24_ADDRESS + 24)
#define HW_VERSION_10_ADDRESS            (HW_DESCRIPTION_50_ADDRESS + 50)
#define HW_URI_50_ADDRESS                (HW_VERSION_10_ADDRESS + 10)

//
// Hardware Info
//
//extern __prog__ char hardware_manufacturer[24] __attribute__ ((space(prog),address(HARDWARE_INFO_BASE)));
//extern __prog__ char hardware_model[24]        __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24)));
//extern __prog__ char hardware_description[50]  __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24)));
//extern __prog__ char hardware_version[10]      __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50)));
//extern __prog__ char hardware_uri[50]          __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10)));

#define BOOT_AUTHOR_40_ADDRESS           (HW_URI_50_ADDRESS + 50)
#define BOOT_DESCRIPTION_50_ADDRESS      (BOOT_AUTHOR_40_ADDRESS + 40)
#define BOOT_VERSION_10_ADDRESS          (BOOT_DESCRIPTION_50_ADDRESS + 50)
#define BOOT_URI_50_ADDRESS              (BOOT_VERSION_10_ADDRESS + 10)
//
// Bootloader Info
//
//extern __prog__ char bootcode_author[40]       __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50)));
//extern __prog__ char bootcode_description[50]  __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40)));
//extern __prog__ char bootcode_version[10]      __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40 + 50)));
//extern __prog__ char bootcode_uri[50]          __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40 + 50 + 10)));

//#define FIRMWARE_STRINGS_BASE 0x8800
//extern __prog__ char firmware_author[40]       __attribute__ ((space(prog),address(FIRMWARE_STRINGS_BASE)));
//extern __prog__ char firmware_description[50]  __attribute__ ((space(prog),address(FIRMWARE_STRINGS_BASE + 40)));
//extern __prog__ char firmware_version[10]      __attribute__ ((space(prog),address(FIRMWARE_STRINGS_BASE + 40 + 50)));
//extern __prog__ char firmware_uri[50]          __attribute__ ((space(prog),address(FIRMWARE_STRINGS_BASE + 40 + 50 + 10)));


#define APPLICATION_STRINGS_BASE 0x18000
#define APP_AUTHOR_40_ADDRESS       APPLICATION_STRINGS_BASE
#define APP_SOFTWARE_50_ADDRESS     (APP_AUTHOR_40_ADDRESS + 40)
#define APP_VERSION_10_ADDRESS      (APP_SOFTWARE_50_ADDRESS + 50)
#define APP_URI_50_ADDRESS          (APP_VERSION_10_ADDRESS + 10)

extern result_t cb_get_hardware_info(uint8_t *data, uint16_t *length);
extern result_t cb_get_boot_info(uint8_t *data, uint16_t *data_len);
extern result_t cb_get_firmware_info(uint8_t *data, uint16_t *data_len);
extern result_t cb_get_application_info(uint8_t *data, uint16_t *length);
#ifdef SYS_CAN_NODE_OS
extern result_t cb_get_node_config_info(uint8_t *data, uint16_t *data_len);
#endif // SYS_CAN_NODE_OS
