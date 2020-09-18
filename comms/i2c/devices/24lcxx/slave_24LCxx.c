#include "libesoup_config.h"

#if defined(SYS_SLV_24LCxx)

#include "libesoup/comms/i2c/i2c.h"

static struct i2c_device device;

result_t i2c_24lcxx_init(enum i2c_channel channel)
{
	device.channel = channel;

	return(i2c_reserve(&device));
}


#endif // SYS_SLV_24LC64