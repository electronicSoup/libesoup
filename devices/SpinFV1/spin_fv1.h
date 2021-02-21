#include "libesoup_config.h"

struct spin_fv1 {
        enum gpio_pin trigger;
        enum gpio_pin i2c_sda;
        enum gpio_pin i2c_scl;
};

extern result_t spin_init_device(struct spin_fv1 *device);
extern result_t spin_program(struct spin_fv1 *device, uint8_t *buffer);
