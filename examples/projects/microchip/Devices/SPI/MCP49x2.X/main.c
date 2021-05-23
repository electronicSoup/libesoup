#include "libesoup_config.h"
#ifdef SYS_EXAMPLE_DEVICE_MCP29x2

#define DEBUG_FILE
#define TAG "MAIN"

#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/spi/spi.h"

int main(int argc, char** argv)
{
	result_t           rc;
	struct spi_device  spi_device;

	spi_device.io.cs    = BRD_MCP49x2_CS;
	spi_device.bus_mode = bus_mode_0;

	libesoup_init();

	rc = spi_reserve(&spi_device);
	if (rc < 0) {
		LOG_E("Failed to reserve SPI Channel\n\r");
	}
	rc = spi_write_byte(&spi_device, 0x55);
	if (rc < 0) {
		LOG_E("Failed to write SPI Byte\n\r");
	}

	LOG_D("Entering main loop\n\r");
	while(1) {
		libesoup_tasks();
	}
	return(0);
}

#endif // SYS_EXAMPLE_DEVICE_MCP29x2
