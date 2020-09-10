
#include "libesoup_config.h"

#ifdef SYS_TEST_24LC64

#define DEBUG_FILE
#define TAG "Main"

#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/i2c/i2c.h"
#include "libesoup/comms/i2c/mc24LC64.h"

void callback(uint8_t *buffer)
{
        LOG_D("Callback\n\r");
}

int main(void)
{
        result_t rc;
        uint8_t  buffer[10];

        rc = libesoup_init();

        rc = mc24lc64_read(I2C3, 0x00, 0x00, 0x01, buffer, callback);

        while (1) {
                libesoup_tasks();
        }
        return (0);
}

#endif // SYS_TEST_24LC64
