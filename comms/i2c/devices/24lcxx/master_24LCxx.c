#include "libesoup_config.h"

#ifdef SYS_24LC64

#define DEBUG_FILE
#define TAG "24LC64"
#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/i2c/i2c.h"

#define READ  0x01
#define WRITE 0x00

static uint8_t          busy = 0;

static struct i2c_device i2c;
//static enum i2c_channel chan;
static uint8_t          chip_addr;
static uint16_t         mem_addr;
static uint16_t         num_bytes;
static uint8_t         *buffer;
static void           (*done_callback)(result_t, uint8_t *, uint8_t);
static uint8_t          tx_buffer[4];

void started_callback(result_t);
void preamble_sent(result_t);
void restarted_callback(result_t);
void read_bytes_sent(result_t);
//void read_finished(result_t, uint8_t);
void read_finished(result_t);

void started_callback(result_t p_rc)
{
        result_t  rc;

        if (p_rc < 0) {
                LOG_E("Failed to start\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL, 0);
                }
        } else {
//                LOG_D("Send preamble\n\r");

                tx_buffer[0]  = 0xA0 | (chip_addr & 0x0E) | WRITE;
                tx_buffer[1]  = (mem_addr >> 8) & 0xff;
                tx_buffer[2]  = mem_addr & 0xff;

		i2c.callback = preamble_sent;
                rc = i2c_write(&i2c, tx_buffer, 3, preamble_sent);
        }
}

void preamble_sent(result_t p_rc)
{
        result_t rc;

        if (p_rc < 0) {
                LOG_E("Preamble Failed\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL, 0);
                }
        } else {
                LOG_D("preamble sent\n\r");

		i2c.callback = restarted_callback;
                rc = i2c_restart(&i2c);
        }
}

void restarted_callback(result_t p_rc)
{
        result_t rc;

        if (p_rc < 0) {
                LOG_E("Restart Failed\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL, 0);
                }
        } else {
                LOG_D("restarted_callback\n\r");

                tx_buffer[0]  = 0xA0 | (chip_addr & 0x0E) | READ;

		//i2c.callback = read_bytes_sent;
                rc = i2c_write(&i2c, tx_buffer, 1, read_bytes_sent);
        }
}

void read_bytes_sent(result_t p_rc)
{
        result_t rc;
	LOG_D("read_bytes_sent()\n\r");

        if (p_rc < 0) {
                LOG_E("Read ctrl bytes Failed\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL, 0);
                }
        } else {
                LOG_D("Read ctrl sent\n\r");

		//i2c.callback = read_finished;
                rc = i2c_read(&i2c, buffer, num_bytes, read_finished);
        }

}

//void read_finished(result_t p_rc, uint8_t count) {
void read_finished(result_t p_rc) {
        result_t rc;

	LOG_D("Read Finished\n\r");
	rc = i2c_stop(&i2c);
        if (p_rc < 0) {
                LOG_E("Restart Failed\n\r");
                if (done_callback) {
                        done_callback(p_rc, NULL, 0);
                }
        } else {
		LOG_E("Code here!")
		//done_callback(SUCCESS, buffer, count);
        }
}

result_t mc24lc64_read(enum i2c_channel p_chan, uint8_t p_chip_addr, uint16_t p_mem_addr, uint16_t p_num_bytes, uint8_t *p_buffer, void (*p_callback)(result_t, uint8_t *, uint8_t))
{
        result_t rc;

        LOG_D("mc24lc64_read()\n\r");

	i2c.channel  = p_chan;
	i2c.callback = started_callback;

        if (!busy) {
                chip_addr     = p_chip_addr;
                mem_addr      = p_mem_addr;
                num_bytes     = p_num_bytes;
                buffer        = p_buffer;
                done_callback = p_callback;

		rc = i2c_py_reserve(&i2c);
		if(rc < 0) {
			LOG_D("Failed to reserve I2C channel\n\r");
		}

                rc = i2c_start(&i2c);
                RC_CHECK

                return (SUCCESS);
        } else {
                return (-ERR_BUSY);
        }
}

#endif // SYS_24LC64
