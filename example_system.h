
#include "es_lib/core.h"

/*
 * Baud rate of the serial debug port
 */
#define SERIAL_BAUD 19200

/*
 * The configuration of the Serial Port there are 3 pins, the Gnd pin is fixed
 * so you define the configuration of the other two pins. Uncomment as required
 */
//#define SERIAL_PORT_GndTxRx //== 20
#define SERIAL_PORT_GndRxTx //== 20

/*
 * Log level. these are defined in es_lib/core.h as they are included everywhere
 */
#define LOG_LEVEL LOG_DEBUG
