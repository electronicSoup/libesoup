#ifndef _SLAVE_24LCxx_H
#define _SLAVE_24LCxx_H

#include "libesoup_config.h"

#ifdef SYS_SLV_24LCxx
#include "libesoup/comms/i2c/i2c.h"

extern result_t i2c_24lcxx_init(enum i2c_channel channel);

#endif

#endif // _SLAVE_24LCxx_H
