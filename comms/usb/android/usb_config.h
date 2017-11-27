/**
 *
 * \file libesoup/usb/android/system_config.h
 *
 * Definition file required by the Microchip USB Host stack
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
#define USB_SUPPORT_HOST

#define MAX_ALLOWED_CURRENT   500

#define USB_PING_PONG_MODE  USB_PING_PONG__FULL_PING_PONG

#define NUM_TPL_ENTRIES 3
#define NUM_CLIENT_DRIVER_ENTRIES 2

#define USB_ENABLE_TRANSFER_EVENT

#define USB_HOST_APP_DATA_EVENT_HANDLER USB_ApplicationDataEventHandler
#define USB_ENABLE_1MS_EVENT

#define ANDROID_DEVICE_ATTACH_TIMEOUT 3000

#define USB_MAX_GENERIC_DEVICES 1
#define USB_NUM_CONTROL_NAKS 20
#define USB_SUPPORT_INTERRUPT_TRANSFERS
#define USB_SUPPORT_BULK_TRANSFERS
#define USB_NUM_INTERRUPT_NAKS 3
#define USB_INITIAL_VBUS_CURRENT (100/2)
#define USB_INSERT_TIME (250+1)
#define USB_HOST_APP_EVENT_HANDLER USB_ApplicationEventHandler

#define USBTasks()                    \
    {                                 \
        USBHostTasks();               \
        AndroidTasks();               \
        android_tasks();              \
    }

#define USBInitialize(x)            \
    {                               \
        USBHostInit(x);             \
    }

