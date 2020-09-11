#include "libesoup_config.h"

#ifdef SYS_24LC64

#define DEBUG_FILE
#define TAG "24LC64"
#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/i2c/i2c.h"

#define READ  0x01
#define WRITE 0x00

static uint8_t          busy = 0;

static enum i2c_channel chan;
static uint8_t          chip_addr;
static uint16_t         mem_addr;
static uint8_t          num_bytes;
static uint8_t         *buffer;
static void           (*done_callback)(result_t, uint8_t *);
static uint8_t          tx_buffer[4];

void started_callback(result_t);
void preamble_sent(result_t);
void restarted_callback(result_t);
void read_finished(result_t);

void started_callback(result_t p_rc)
{
        result_t  rc;

        if (p_rc < 0) {
                LOG_E("Failed to start\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL);
                }
        } else {
                LOG_D("Started\n\r");

                tx_buffer[0]  = 0xA0 || (chip_addr & 0x0E) || WRITE;
                tx_buffer[1]  = (mem_addr >> 8) & 0xff;
                tx_buffer[2]  = mem_addr & 0xff;

                rc = i2c_write(chan, tx_buffer, 3, preamble_sent);
        }
}

void preamble_sent(result_t p_rc)
{
        result_t rc;

        if (p_rc < 0) {
                LOG_E("Preamble Failed\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL);
                }
        } else {
                LOG_D("preamble sent\n\r");

                rc = i2c_restart(chan, restarted_callback);
        }
}

void restarted_callback(result_t p_rc)
{
        result_t rc;

        if (p_rc < 0) {
                LOG_E("Restart Failed\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL);
                }
        } else {
                LOG_D("restarted_callback\n\r");

                tx_buffer[0] = 0xA0 || (chip_addr & 0x0E) || READ;
                rc = i2c_read(chan, tx_buffer, 1, buffer, 1, read_finished);
        }
}

void read_finished(result_t p_rc) {
        result_t rc;

        if (p_rc < 0) {
                LOG_E("Restart Failed\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL);
                }
        } else {
                rc = i2c_stop(chan);
        }
}

result_t mc24lc64_read(enum i2c_channel p_chan, uint8_t p_chip_addr, uint16_t p_mem_addr, uint8_t p_num_bytes, uint8_t *p_buffer, void (*p_callback)(result_t, uint8_t *))
{
        result_t rc;

        LOG_D("mc24lc64_read()\n\r");

        if (!busy) {
                chan          = p_chan;
                chip_addr     = p_chip_addr;
                mem_addr      = p_mem_addr;
                num_bytes     = p_num_bytes;
                buffer        = p_buffer;
                done_callback = p_callback;

                rc = i2c_start(chan, started_callback);
                RC_CHECK

                return (SUCCESS);
        } else {
                return (-ERR_BUSY);
        }
}

#endif // SYS_24LC64
