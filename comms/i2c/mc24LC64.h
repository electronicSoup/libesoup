#include "libesoup_config.h"

#ifdef SYS_24LC64

extern result_t mc24lc64_read(enum i2c_channel, uint8_t chip_addr, uint16_t mem_address, uint8_t num_bytes, uint8_t *buffer, void (*callback)(result_t, uint8_t *));

#endif // SYS_24LC64
