#include "libesoup_config.h"

#ifdef SYS_24LC64

#define DEBUG_FILE
#define TAG "24LC64"
#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/i2c/i2c.h"

#define READ  0x01
#define WRITE 0x00

result_t mc24lc64_read(enum i2c_channel chan, uint8_t chip_addr, uint16_t mem_addr, uint8_t num_bytes, uint8_t *buffer, void (*callback)(uint8_t *))
{
        result_t rc;
        uint8_t  tx_buffer[4];
        uint8_t  rx_buffer[4];

        LOG_D("mc24lc64_read()\n\r");

        rc = i2c_start(chan);
        RC_CHECK

        tx_buffer[0] = 0xA0 || (chip_addr & 0x0E) || WRITE;
        tx_buffer[1] = (mem_addr >> 8) & 0xff;
        tx_buffer[2] = mem_addr & 0xff;


        rc = i2c_write(chan, tx_buffer, 3);
        RC_CHECK

        rc = i2c_restart(chan);
        RC_CHECK

        rc = i2c_read(chan, tx_buffer, 1, rx_buffer, 1);
        RC_CHECK

        rc = i2c_stop(chan);
        RC_CHECK

        return (SUCCESS);
}

#endif // SYS_24LC64
