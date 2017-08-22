
//extern result_t one_wire_init(enum pin_t pin);
//extern result_t one_wire_get_device_count(enum pin_t pin, uint8_t *count);
extern result_t one_wire_ds2502_read_rom(enum pin_t pin);

#if 0
#define DS2502_DDR                      TRISFbits.TRISF3
#define DS2502_OPEN_DRAIN_BIT           ODCFbits.ODCF3
#define DS2502_DATA_W                   LATFbits.LATF3
#define DS2502_DATA_R                   PORTFbits.RF3
#define DS2502_CHANGE_NOTIFICATION_ISR  CNENFbits.CNIEF3
#endif
