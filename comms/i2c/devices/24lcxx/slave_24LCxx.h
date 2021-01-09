#ifndef _SLAVE_24LCxx_H
#define _SLAVE_24LCxx_H

#include "libesoup_config.h"

#ifdef SYS_SLV_24LCxx
#include "libesoup/comms/i2c/i2c.h"

extern result_t slave_24lcxx(struct i2c_device *i2c_device, uint8_t *buffer, uint16_t size);

#endif

#endif // _SLAVE_24LCxx_H
