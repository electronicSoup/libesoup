
#define U1_RX_ISR_FLAG   IFS0bits.U1RXIF
#define U1_TX_ISR_FLAG   IFS0bits.U1TXIF
#define U1_RX_ISR_ENABLE IEC0bits.U1RXIE
#define U1_TX_ISR_ENABLE IEC0bits.U1TXIE

#define U2_RX_ISR_FLAG   IFS1bits.U2RXIF
#define U2_TX_ISR_FLAG   IFS1bits.U2TXIF
#define U2_RX_ISR_ENABLE IEC1bits.U2RXIE
#define U2_TX_ISR_ENABLE IEC1bits.U2TXIE

#define U3_RX_ISR_FLAG   IFS5bits.U3RXIF
#define U3_TX_ISR_FLAG   IFS5bits.U3TXIF
#define U3_RX_ISR_ENABLE IEC5bits.U3RXIE
#define U3_TX_ISR_ENABLE IEC5bits.U3TXIE

#define U4_RX_ISR_FLAG   IFS5bits.U4RXIF
#define U4_TX_ISR_FLAG   IFS5bits.U4TXIF
#define U4_RX_ISR_ENABLE IEC5bits.U4RXIE
#define U4_TX_ISR_ENABLE IEC5bits.U4TXIE

#define UART_2           0x00
#define UART_3           0x01
#define UART_4           0x02

#define NUM_UARTS        3

#define DEBUG_FILE
#define TAG "UART"

#include "system.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/utils/rand.h"

#include "uart.h"


enum uart_status {
	UART_FREE,
	UART_BUSY
};

struct uart {
	enum uart_status status;
	u16              magic;
	uart_data       *data;
} uart;

struct uart uarts[NUM_UARTS];

/*
 * Local static Function prototypes
 */
static void uart_tx_isr(u8);

static void uart_set_rx_pin(u8 uart, u8 pin);
static void uart_set_tx_pin(u8 uart, u8 pin);
static void uart_set_com_config(uart_data *com);

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static u16 load_tx_buffer(uart_data *com);

/*
 * Interrupt Service Routines
 */
void _ISR __attribute__((__no_auto_psv__)) _U2TXInterrupt(void)
{
	while(U2_TX_ISR_FLAG) {
		U2_TX_ISR_FLAG = 0;
		uart_tx_isr(UART_2);
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U3TXInterrupt(void)
{
	while(U3_TX_ISR_FLAG) {
		U3_TX_ISR_FLAG = 0;
		uart_tx_isr(UART_3);
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U4TXInterrupt(void)
{
	while(U4_TX_ISR_FLAG) {
		U4_TX_ISR_FLAG = 0;
		uart_tx_isr(UART_4);
	}
}

static void uart_tx_isr(u8 uart)
{
	uart_data *com;

	LOG_D("uart_tx_isr()\n\r");

	if ((uarts[uart].data == NULL) || (uarts[uart].status != UART_BUSY)) {
		LOG_E("UART Null in ISR!\n\r");
		return;
	}

	com = uarts[uart].data;

	/*
	 * If the TX buffer is not full load it from the tx buffer
	 */
	if (!load_tx_buffer(com)) {
		switch(uart) {
			case UART_2:
				while (!U2STAbits.TRMT) {
					Nop();
				}
				break;

			case UART_3:
				while (!U3STAbits.TRMT) {
					Nop();
				}
				break;

			case UART_4:
				while (!U4STAbits.TRMT) {
					Nop();
				}
				break;

			default:
				LOG_E("Bad comm port given!\n\r");
				break;
		}
	}
}

void uart_init(void)
{
	u8 loop;

	random_init();

	for(loop = 0; loop < NUM_UARTS; loop++) {
		uarts[loop].status = UART_FREE;
		uarts[loop].magic = 0x0000;
		uarts[loop].data = NULL;
	}
}

result_t uart_reserve(uart_data *data)
{
	/*
	 * Find a free uart to use
	 */
	u8  loop;
	u32 long_magic = 0x00;

	for(loop = 0; loop < NUM_UARTS; loop++) {
		if(uarts[loop].status == UART_FREE) {

			while(long_magic == 0x00) {
				long_magic = rand();
			}
			uarts[loop].data = data;
			uarts[loop].status = UART_BUSY;
			uarts[loop].magic = (u16)(long_magic & 0xffff);

			data->uart = loop;
			data->magic = uarts[loop].magic;

			AD1PCFGL = 0xffff;

			/*
			 * Set up the Rx & Tx pins
			 */
			uart_set_rx_pin((u8) data->uart, data->rx_pin);
			uart_set_tx_pin((u8) data->uart, data->tx_pin);

			uart_set_com_config(data);

			data->tx_buffer_read_index = 0;

			/*
			 * Load up the transmit buffer
			 */
			load_tx_buffer(data);

			return(SUCCESS);
		}
	}

	return(ERR_NO_RESOURCES);
}


static void uart_set_rx_pin(u8 uart, u8 pin)
{
	switch (pin) {
		case RP0:
			TRISBbits.TRISB0 = 1;
			break;

		case RP1:
			TRISBbits.TRISB1 = 1;
			break;

		case RP13:
			TRISBbits.TRISB2 = 1;
			break;

		case RP25:
			TRISDbits.TRISD4 = 1;
			break;

		case RP28:
			TRISBbits.TRISB4 = 1;
			break;

		default:
			LOG_E("Unknow Peripheral Rx Pin\n\r");
			break;
	}

	switch (uart) {
//		case UART_1:
//			RPINR18bits.U1RXR = pin;
//			break;
//
		case UART_2:
			RPINR19bits.U2RXR = pin;
			break;

		case UART_3:
			RPINR17bits.U3RXR = pin;
			break;

		case UART_4:
			RPINR27bits.U4RXR = pin;
			break;
	}
}

static void uart_set_tx_pin(u8 uart, u8 pin)
{
	u8 tx_function;

	switch (uart) {
//		case UART_1:
//			tx_function = 3;
//			break;

		case UART_2:
			tx_function = 5;
			break;

		case UART_3:
			tx_function = 28;
			break;

		case UART_4:
			tx_function = 30;
			break;
	}

	switch (pin) {
		case RP0:
			TRISBbits.TRISB0 = 0;
			RPOR0bits.RP0R = tx_function;
			break;

		case RP1:
			TRISBbits.TRISB1 = 0;
			RPOR0bits.RP1R = tx_function;
			break;

		case RP13:
			TRISBbits.TRISB2 = 0;
			RPOR6bits.RP13R = tx_function;
			break;

		case RP25:
			TRISDbits.TRISD4 = 0;
			RPOR12bits.RP25R = tx_function;
			break;

		case RP28:
			TRISBbits.TRISB4 = 0;
			RPOR14bits.RP28R = tx_function;
			break;

		default:
			LOG_E("Unknow Peripheral Tx Pin\n\r");
			break;
	}
}

static void uart_set_com_config(uart_data *com)
{
	switch (com->uart) {
//		case UART_1:
//			U1MODE = com->uart_mode;
//
//			U1STA = 0x8410;
//
//			/*
//			 * Interrupt when a character is transferred to the Transmit Shift
//			 * Register (TSR), and as a result, the transmit buffer becomes empty
//			 */
//			U1STAbits.UTXISEL1 = 1;
//			U1STAbits.UTXISEL0 = 0;
//
//			/*
//			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
//			 *
//			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
//			 *
//			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
//			 *
//			 */
//			U1BRG = ((CLOCK_FREQ / com->baud) / 16) - 1;
//
//			U1_RX_ISR_ENABLE = 0;
//			U1_TX_ISR_ENABLE = 1;
//			break;
//
		case UART_2:
			U2MODE = com->uart_mode;

			U2STA = 0x8410;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (TSR), and as a result, the transmit buffer becomes empty
			 */
			U2STAbits.UTXISEL1 = 1;
			U2STAbits.UTXISEL0 = 0;

			/*
			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
			 *
			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
			 *
			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
			 *
			 */
			U2BRG = ((CLOCK_FREQ / com->baud) / 16) - 1;

			U2_RX_ISR_ENABLE = 0;
			U2_TX_ISR_ENABLE = 1;
			break;

		case UART_3:
			U3MODE = com->uart_mode;

			U3STA = 0x8410;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (TSR), and as a result, the transmit buffer becomes empty
			 */
			U3STAbits.UTXISEL1 = 1;
			U3STAbits.UTXISEL0 = 0;

			/*
			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
			 *
			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
			 *
			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
			 *
			 */
			U3BRG = ((CLOCK_FREQ / com->baud) / 16) - 1;

			U3_RX_ISR_ENABLE = 0;
			U3_TX_ISR_ENABLE = 1;
			break;

		case UART_4:
			U4MODE = com->uart_mode;

			U4STA = 0x8410;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (TSR), and as a result, the transmit buffer becomes empty
			 */
			U4STAbits.UTXISEL1 = 1;
			U4STAbits.UTXISEL0 = 0;

			/*
			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
			 *
			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
			 *
			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
			 *
			 */
			U4BRG = ((CLOCK_FREQ / com->baud) / 16) - 1;

			U4_RX_ISR_ENABLE = 0;
			U4_TX_ISR_ENABLE = 1;
			break;
	}
}

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static u16 load_tx_buffer(uart_data *com)
{
	switch (com->uart) {
//		case UART_1:
//			while(!U1STAbits.UTXBF && (com->tx_buffer_read_index < com->tx_buffer_size)) {
//				U1TXREG = com->tx_buffer[com->tx_buffer_read_index++];
//			}
//
//			if(com->tx_buffer_read_index < com->tx_buffer_size) {
//				/*
//				 * Interrupt when a character is transferred to the Transmit Shift
//				 * Register (TSR), and as a result, the transmit buffer becomes empty
//				 */
//				U1STAbits.UTXISEL1 = 1;
//				U1STAbits.UTXISEL0 = 0;
//			} else {
//				/*
//				 * Interrupt when the last character is shifted out of the Transmit
//				 * Shift Register; all transmit operations are completed
//				 */
//				U1STAbits.UTXISEL1 = 0;
//				U1STAbits.UTXISEL0 = 1;
//			}
//			break;
//
		case UART_2:
			break;

		case UART_3:
			break;

		case UART_4:
			break;
	}

	return(com->tx_buffer_size - com->tx_buffer_read_index);
}
