This project uses the source file 'usb_host.c' from the Microchip MLA
(Microchip Libraries for Applications) At the time of writing I'm using the
latest available MLA from 2017-03-06. The file is simply linked to from the
command line and left insitu in the MLA directory: 

$ ln -s /opt/microchip/mla/v2017_03_06/framework/usb/src/usb_host.c src/usb_host.c

If you need to enable logging in the usb_host.c file it's probably better to
copy the file directly into your project and add the libesoup debugging code
to the top of the local copy of the file.

$ cp /opt/microchip/mla/v2017_03_06/framework/usb/src/usb_host.c src

The additons would be:
/***********************************************************/
#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
//#undef DEBUG_FILE
static const char *TAG = "HOST";
#include "libesoup/logger/serial_log.h"
#define DEBUG_ENABLE
#define DEBUG_PutString LOG_D
#endif
/***********************************************************/

The project uses the file libesoup/comms/usb/xpad/usb_config.c to define the
functions used for the Logitech game pad device and USB Identifiers recognised
by the Host. In this case Vendor ID  0x046D, Product ID 0xC21D.

Device Descriptor:

0x12 - Length 18 
0x01 - Type Device Descriptor
0x00 - \ 
0x02 -  \ USB 2.0
0xff - Device Class
0xff - Device Sub Class
0xff - Device Protocol
0x08 - Max Packet Size for Endpoint Zero 8
0x6d - \
0x04 -  \ Vendor 0x046D
0x1d - \
0xc2 -  \ Product 0xC21D
0x14 - \ 
0x40 -  \ Release Number 
0x01 - String Index - Manufacturer
0x02 - String Index - Product
0x03 - String Index - Serial Number
0x01 - Number of configurations 


Configuration Descriptor
0x09 - Length 9 
0x02 - Type - Configuration
0x30 - \
0x00 -  \ Total length 0x0030 = 48
0x01 - Number of Interfaces
0x01 - Identifier for Set and Get Configuration Requests
0x00 - String Index
0x80 - Attributes - Bus Powered and no wakeup
0xfa - Max Power 500mA (250 * 2)


Interface:

0x09 - Length
0x04 - Type Interface
0x00 - Interface Number
0x00 - Alternate Setting
0x02 - Number of Endpoints
0xff - Interface Class  (0x03 for HID)
0x5d - Interface Sub Class
0x01 - Interface Protocol
0x00 - String Index


HID Descriptor:

0  0x10 - Length 16 
1  0x21 - Type HID
2  0x10 - \
3  0x01 -  \ HID 1.1
4  0x01 - Country Code    * Arabic ???
5  0x24 - Number of Descriptors
6  0x81 - Descriptor Type * Error bit 7 should always be 0!
7  0x14 - \
8  0x03 -  \ Length
9  0x00 - Optional Descriptor Type
10 0x03 - \
11 0x13 -  \ Optional Descriptor Length
12 0x02 
13 0x00 
14 0x03 
15 0x00
 D :XPAD:HID Descriptor
 D :XPAD:HID Num Descriptors 0x24
 D :XPAD:HID Descriptor Type 0x81
 D :XPAD:HID Descriptor Lenght 0xfa80
 D :XPAD:HID Optional Descriptor Type 0x0
 D :XPAD:HID Optional Descriptor Lenght 0x4
 D :XPAD:*******************************


Endpoint:

0x07 - Length 7
0x05 - Type Endpoint 
0x81 - Number IN 1
0x03 - Attributes - Interrupt
0x20 - \
0x00 -  \ Max Packet Size 0x0020  = 32
0x04 - Service Interval 

Endpoint:
0x07 - Length
0x05 - Type Endpoint
0x02 - Number OUT 2
0x03 - Attributes - Interrupt
0x20 - \
0x00 -  \ Max Packet Size 32
0x08 - Service Interval
