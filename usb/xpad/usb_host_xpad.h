#ifndef __USB_HOST_XPAD

#define __USB_HOST_XPAD

extern bool xpad_initialise ( uint8_t address, uint32_t flags, uint8_t clientDriverID );
extern bool xpad_event_handler( uint8_t address, USB_EVENT event, void *data, uint32_t size );
extern bool xpad_data_event_handler( uint8_t address, USB_EVENT event, void *data, uint32_t size );

#endif // __USB_HOST_XPAD
