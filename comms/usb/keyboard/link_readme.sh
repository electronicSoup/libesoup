#!/bin/bash

# Notes -
#
# The header files in this directory are expected to be in the include
# path of the compiler so this directory has to be add to the path defined
# in MPLAB-x
#
# usb_config.h specifies the VID and PID of the USB Device being created.

# We need the framework files for USB Device mode so we'll simply link
# to those files:
ln -s /opt/microchip/mla/v2018_11_26/framework/usb/src/usb_device.c     ./mcp_usb_device.c
ln -s /opt/microchip/mla/v2018_11_26/framework/usb/src/usb_device_hid.c ./mcp_usb_device_hid.c
ln -s /opt/microchip/mla/v2018_11_26/framework/usb/src/usb_hal_16bit.c  ./mcp_usb_hal_16bit.c

#
# This is a header file so we have to keep the same name. Alternatively
# we could add this directory to the Compiler's Include search path,
# but that's already getting a bit budsy.
ln -s /opt/microchip/mla/v2018_11_26/framework/usb/src/usb_device_local.h
