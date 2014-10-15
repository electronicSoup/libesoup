#include "es_lib/core.h"
#include "system.h"
#include "usb/usb.h"
#include "usb/usb_host_xpad.h"

// *****************************************************************************
// Client Driver Function Pointer Table for the USB Embedded Host foundation
// *****************************************************************************

CLIENT_DRIVER_TABLE usbClientDrvTable[NUM_CLIENT_DRIVER_ENTRIES] =
{                                        
    {
        xpad_initialise,
        xpad_event_handler,
        xpad_data_event_handler,
        0
    }
};

// *****************************************************************************
// USB Embedded Host Targeted Peripheral List (TPL)
// *****************************************************************************
USB_TPL usbTPL[NUM_TPL_ENTRIES] =
{
      /*[1] Device identification information
        [2] Initial USB configuration to use
        [3] Client driver table entry
        [4] Flags (HNP supported, client driver entry, SetConfiguration() commands allowed)
    ---------------------------------------------------------------------
                [1]                      [2][3] [4]
    ---------------------------------------------------------------------*/
    { INIT_VID_PID( 0x046Dul, 0xC21Dul ), 0, 0, {0} }, // Logitech Gamepad F310
};

