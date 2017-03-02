#if defined(__dsPIC33EP256MU806__)

#include <p33EP256MU806.h>

#include "system.h"

#include "es_lib/comms/can/l2_dsPIC33EP256MU806.h"
#include "es_lib/timers/timers.h"
#include "es_lib/comms/can/es_can.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "dsPIC33_CAN"

#if DEBUG_LEVEL < NO_LOGGING
char baud_rate_strings[8][10] = {
    "baud_10K",
    "baud_20K",
    "baud_50K",
    "baud_125K",
    "baud_250K",
    "baud_500K",
    "baud_800K",
    "baud_1M"
};
#endif

#define MASK_0    0b00
#define MASK_1    0b01
#define MASK_2    0b10

#define WIN_ZERO  C1CTRL1bits.WIN = 0;
#define WIN_ONE   C1CTRL1bits.WIN = 1;

typedef struct __attribute__ ((packed))
{
        u16 ide   :1;
        u16 ssr   :1;
        u16 sid   :11;
        u16       :3;
        u16 eid_h :12;
        u16       :4;
        u16 dlc   :4;
        u16 rb0   :1;
        u16       :3;
        u16 rb1   :1;
        u16 rtr   :1;
        u16 eid_l :6;
        u8  data[8];
        u16       :8;
        u16 filhit:5;
} can_buffer;

#define NUM_CAN_BUFFERS 32

//static can_buffer can_buffers[NUM_CAN_BUFFERS] __attribute__((aligned(NUM_CAN_BUFFERS * 16)));
//unsigned int can_buffers[NUM_CAN_BUFFERS][8] __attribute__((aligned(NUM_CAN_BUFFERS * 16)));
__eds__ unsigned int can_buffers[NUM_CAN_BUFFERS][8] __attribute__((eds,space(dma)));

//__eds__ unsigned int can_buffers[NUM_CAN_BUFFERS] __attribute__((eds,space(dma)));

typedef struct __attribute__ ((packed))
{
        u16 TXmPRI:2;
        u16 RTRENm:1;
        u16 TXREQm:1;
        u16 TXERRm:1;
        u16 TXLARBm:1;
        u16 TXABTm:1;
        u16 TXENm:1;
        u16 TXnPRI:2;
        u16 RTRENn:1;
        u16 TXREQn:1;
        u16 TXERRn:1;
        u16 TXLARBn:1;
        u16 TXABTn:1;
        u16 TXENn:1;
} CxTRmnCON;

#define NUM_TX_CONTROL  4

CxTRmnCON *tx_control[NUM_TX_CONTROL];

//static u32 rxMsgCount = 0;

//#define CAN_RX_CIR_BUFFER_SIZE 5

// canBuffer_t cirBuffer[CAN_RX_CIR_BUFFER_SIZE];
//u8 cirBufferNextRead = 0;
//u8 cirBufferNextWrite = 0;
//u8 cirBufferCount = 0;

// can_msg_t rxCanMsg;

//#define TX_BUFFERS  3

// canBuffer_t *tx_buffers[TX_BUFFERS];

//struct l2_can_frame rx_frame;

//#define RX_BUFFERS  5

//struct can_buffer *tx_buffers[TX_BUFFERS];
//struct can_buffer *rx_buffers[RX_BUFFERS];

static void set_mode(u8 mode);
static void set_bit_rate(can_baud_rate_t baudRate);

static void (*status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud);

void __attribute__((__interrupt__, __no_auto_psv__)) _C1RxRdyInterrupt(void)
{
        LOG_D("C1RxRdy Isr");
}

void __attribute__((__interrupt__, __no_auto_psv__)) _C1Interrupt(void)
{
        LOG_D("C1 Isr Flag 0x%x - 0lx%lx ICODE 0x%x\n\r", C1INTF, C1INTF, C1VECbits.ICODE);
        
        if(C1INTFbits.TBIF) {
                C1INTFbits.TBIF = 0;
                LOG_D("TBIF\n\r");
        } else if(C1INTFbits.RBIF) {
                C1INTFbits.RBIF = 0;
                LOG_D("RBIF\n\r");
        } else if(C1INTFbits.RBOVIF) {
                C1INTFbits.RBOVIF = 0;
                LOG_D("RBOVIF\n\r");
        } else if(C1INTFbits.FIFOIF) {
                C1INTFbits.FIFOIF = 0;
                LOG_D("FIFOIF\n\r");
        } else if(C1INTFbits.ERRIF) {
                C1INTFbits.ERRIF = 0;
                LOG_D("ERRIF\n\r");
        } else if(C1INTFbits.WAKIF) {
                C1INTFbits.WAKIF = 0;
                LOG_D("WAKIF\n\r");
        } else if(C1INTFbits.IVRIF) {
                C1INTFbits.IVRIF = 0;
                LOG_D("IVRIF\n\r");
        } else if(C1INTFbits.EWARN) {
                C1INTFbits.EWARN = 0;
                LOG_D("EWARN\n\r");
        } else if(C1INTFbits.RXWAR) {
                C1INTFbits.RXWAR = 0;
                LOG_D("RXWAR\n\r");
        } else if(C1INTFbits.TXWAR) {
                C1INTFbits.TXWAR = 0;
                LOG_D("TXWAR\n\r");
        } else if(C1INTFbits.RXBP) {
                C1INTFbits.RXBP = 0;
                LOG_D("RXBP\n\r");
        } else if(C1INTFbits.TXBP) {
                C1INTFbits.TXBP = 0;
                LOG_D("TXBP\n\r");
        } else if(C1INTFbits.TXBO) {
                C1INTFbits.TXBO = 0;
                LOG_D("TXBO\n\r");
        } else {
                LOG_D("Unprocessed ISR\n\r");
        }
}

void __attribute__((__interrupt__, __no_auto_psv__)) _DMA0Interrupt(void)
{
        LOG_D("DMA-0 ISR");
        IFS0bits.DMA0IF = 0;
}

void __attribute__((__interrupt__, __no_auto_psv__)) _DMA1Interrupt(void)
{
        LOG_D("DMA-1 ISR");
}

void __attribute__((__interrupt__, __no_auto_psv__)) _DMA2Interrupt(void)
{
        LOG_D("DMA-2 ISR");
}

result_t can_l2_init(can_baud_rate_t arg_baud_rate, void (*arg_status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud))
//void init(void)
{
        u32 address;
        
        /*
         * Tx pin is RF4 (RP100)
         * Rx is RG7 (RPI119)
         */
        TRISFbits.TRISF4 = 0;
        TRISGbits.TRISG7 = 1;
        RPOR9bits.RP100R = 0x0e;
        RPINR26bits.C1RXR = 119;

        /*
         * Enter configuration mode
         */
        C1CTRL1bits.REQOP = 0b100;
        while (C1CTRL1bits.OPMODE != 0b100) Nop();

        /*
         * Set 8 transmit buffers
         */
        C1TR01CONbits.TXEN0 = 1;
        C1TR01CONbits.TXEN1 = 1;
        C1TR23CONbits.TXEN2 = 1;
        C1TR23CONbits.TXEN3 = 1;
        C1TR45CONbits.TXEN4 = 1;
        C1TR45CONbits.TXEN5 = 1;
        C1TR67CONbits.TXEN6 = 1;
        C1TR67CONbits.TXEN7 = 1;
    
        /*
         * Set the number of DMA buffers we're using to 32
         */
	C1CTRL1bits.WIN = 0;
	C1FCTRLbits.DMABS = 0b110;   // 32 Buffers in RAM
        C1FCTRLbits.FSA   = 0x08;    // FIFO starts Buffer 8
        
        /*
         * All filters result in Rx'd frame going into FIFO
         */
	C1CTRL1bits.WIN = 1;
        C1BUFPNT1 - 0xffff;
        C1BUFPNT2 - 0xffff;
        C1BUFPNT3 - 0xffff;
        C1BUFPNT4 - 0xffff;
//        C1BUFPNT1bits.F0BP = 0xf;
//        C1BUFPNT1bits.F1BP = 0xf;
//        C1BUFPNT1bits.F2BP = 0xf;
//        C1BUFPNT1bits.F3BP = 0xf;
//        C1BUFPNT2bits.F4BP = 0xf;
//        C1BUFPNT2bits.F5BP = 0xf;
//        C1BUFPNT2bits.F6BP = 0xf;
//        C1BUFPNT2bits.F7BP = 0xf;
//        C1BUFPNT3bits.F8BP = 0xf;
//        C1BUFPNT3bits.F9BP = 0xf;
//        C1BUFPNT3bits.F10BP = 0xf;
//        C1BUFPNT3bits.F11BP = 0xf;
//        C1BUFPNT4bits.F12BP = 0xf;
//        C1BUFPNT4bits.F13BP = 0xf;
//        C1BUFPNT4bits.F14BP = 0xf;
//        C1BUFPNT4bits.F15BP = 0xf;

        /*
         * All filters user mask 0
         */
	C1CTRL1bits.WIN = 0;
        C1FMSKSEL1 = 0x0000;
        C1FMSKSEL2 = 0x0000;

        /*
         * Filter zero is looking for 0x555
         */
        C1RXF0SIDbits.SID = 0x555;
        C1RXF0SIDbits.EXIDE = 0;
        
        /*
         * Setup mask zero, much don't care
         */
	C1CTRL1bits.WIN = 1;
        C1RXM0SIDbits.SID = 0x3ff;// = 0x0000;
        C1RXM0SIDbits.MIDE = 1; // Match Standard/Extended
        C1RXM0EID = 0x0000;
        
        /*
         * Setup mask one
         */
        C1RXM1SID = 0x0000;
        C1RXM1EID = 0x0000;

        /*
         * Setup mask two
         */
        C1RXM2SID = 0x0000;
        C1RXM2EID = 0x0000;

        /*
         * Enable all Rx Filters for the moment.
         */
        C1FEN1 = 0xffff;
        
	C1CTRL1bits.WIN = 0;

	/*
	 * Set Baud rate 125Kbs
	 */
        C1CTRL1bits.CANCKS = 0;   //Use peripheral clock  
        C1CFG1bits.SJW = 3;
        C1CFG1bits.BRP = 3;

        C1CFG2bits.PRSEG = 6;
        C1CFG2bits.SEG1PH = 3;
        C1CFG2bits.SEG2PH = 3;
        
	C1CFG2bits.SAM = 0; //One sample point

        /*
         * Setup DMA Channel 2 for CAN 1 TX
         */
        DMA2CONbits.SIZE = 0x00;    // Word transfer mode
        DMA2CONbits.DIR = 0x01;     // Direction - Device RAM to Peripheral
        DMA2CONbits.AMODE = 0x02;   // Addressing mode: Peripheral indirect
        DMA2CONbits.MODE = 0x00;    // Operating Mode: Continuous no Ping Pong
        DMA2REQ = 70;               // ECAN1 Tx
        DMA2CNT = 7;                // Buffer size, 8 words
        
        DMA2PAD = (volatile unsigned int)&C1TXD;
        
        address = (u32)&can_buffers;
        
        LOG_D("Address of CAN Buffers is 0x%lx\n\r", address);
        
//        DMA2STAL = __builtin_dmaoffset(can_buffers);
//        DMA2STAH = 0x00;

        DMA0STAL = (uint16_t)(int_least24_t)(&can_buffers);
        DMA0STAH = 0;
        
        IEC1bits.DMA2IE = 1;        // Enable DMA-0 ISR        
        DMA2CONbits.CHEN = 1;    // Enable DMA-0
        
        /*
         * DMA Channel One for Rx
         */
        DMA1CONbits.SIZE = 0x0;    // Data Transfer Size: Word Transfer Mode
        DMA1CONbits.DIR = 0x0;     // Direction: Peripheral to device RAM 
        DMA1CONbits.AMODE = 0x2;   // Addressing Mode: Peripheral Indirect
        DMA1CONbits.MODE = 0x0;    // Operating Mode: Continuous, no Ping Pong
        DMA1REQ = 34;              // Assign ECAN1 Rx event for DMA Channel 1
        DMA1CNT = 7;               // DMA Transfer per ECAN message to 8 words

        /* 
         * Peripheral Address: ECAN1 Receive Register 
         */
        DMA1PAD = (volatile unsigned int) &C1RXD; 
        
        
//        DMA1STAL = ((unsigned int)(&can_buffers)) & 0xffff;
//        DMA1STAH = (((unsigned int)(&can_buffers)) >> 16) & 0xff;
        DMA1STAL = address & 0xffff;
        DMA1STAH = (address >> 16) & 0xff;

//        DMA1STAL = (unsigned int) &can_buffers;
//        DMA1STAH = (unsigned int) &can_buffers;

        DMA1CONbits.CHEN = 0x1;    // Channel Enable: Enable DMA Channel 1
        IEC0bits.DMA1IE = 1;       // Enable DMA Channel 1 Interrupt
 
        /*
         * Clear all interrupt flags and enable all interrupts for the moment.
         */
	C1CTRL1bits.WIN = 0;
        C1INTF = 0x00;        
        C1INTE = 0x00ff;

        IFS2bits.C1IF = 0x00;
        IFS2bits.C1RXIF = 0x00;
        IEC2bits.C1IE = 0x01;
        IEC2bits.C1RXIE = 0x01;

	/*
	 * Drop out of the configuration mode into loopback
	 */
        C1CTRL1bits.REQOP = 0b000;
        while (C1CTRL1bits.OPMODE != 0b000) Nop();
#if 0
	/*
	 * Send a test frame out of buffer zero:
	 */
        can_buffers[0].rb0 = 0;
        can_buffers[0].rb1 = 0;
        can_buffers[0].ide = 0;
        can_buffers[0].rtr = 0;
        can_buffers[0].ssr = 0;
        can_buffers[0].sid = 0x5aa;
        can_buffers[0].dlc = 0;

	/*
	 * Mark the buffer for transmission
	 */
        C1TR01CONbits.TXREQ0 = 1;
        while(C1TR01CONbits.TXREQ0 == 1);
        
        LOG_D("Test frame sent\n\r");
#endif // 0
        return(SUCCESS);
}

result_t can_l2_tx_frame(can_frame *frame)
{
	/*
	 * Send a test frame out of buffer zero:
	 */
#if 0
        can_buffers[0].rb0 = 0;
        can_buffers[0].rb1 = 0;
        can_buffers[0].ide = 0;
        can_buffers[0].rtr = 0;
        can_buffers[0].ssr = 0;
        can_buffers[0].sid = 0x5aa;
        can_buffers[0].dlc = 0;
#endif
        can_buffers[0][0] = 0x123C;
        can_buffers[0][1] = 0x0000;
        can_buffers[0][2] = 0x0008;
        can_buffers[0][3] = 0xabcd;
        can_buffers[0][4] = 0xabcd;
        can_buffers[0][5] = 0xabcd;
        can_buffers[0][6] = 0xabcd;

        /*
	 * Mark the buffer for transmission
	 */
        C1TR01CONbits.TXREQ0 = 1;
//        while(C1TR01CONbits.TXREQ0 == 1);        
        LOG_D("Test frame sent\n\r");
        
        return(SUCCESS);
}
#if 0
result_t can_l2_init(can_baud_rate_t arg_baud_rate, void (*arg_status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud))
{
	u8 loop;

	if (arg_baud_rate <= no_baud) {
		LOG_D("L2_CanInit() Baud Rate %s\n\r", baud_rate_strings[arg_baud_rate]);
	} else {
		LOG_E("L2_CanInit() ToDo!!! No Baud Rate Specified\n\r");
		return (ERR_BAD_INPUT_PARAMETER);
	}

        status_handler = arg_status_handler;

	/*
	 * Set up the CAN Pin Configuration
	 */
        CAN_TX_DDR = OUTPUT_PIN;
        CAN_RX_DDR = INPUT_PIN;

        CAN_TX_PIN = CAN_1_TX;
        CAN_1_RX = CAN_RX_PIN;
        
	set_mode(CONFIG_MODE);

        /*
         * Set 8 transmit buffers
         */
        C1TR01CONbits.TXEN0 = 1;
        C1TR01CONbits.TXEN1 = 1;
        C1TR23CONbits.TXEN2 = 1;
        C1TR23CONbits.TXEN3 = 1;
        C1TR45CONbits.TXEN4 = 1;
        C1TR45CONbits.TXEN5 = 1;
        C1TR67CONbits.TXEN6 = 1;
        C1TR67CONbits.TXEN7 = 1;
#if 0
        tx_control[0] = (CxTRmnCON *)&C1TR01CON;
        tx_control[1] = (CxTRmnCON *)&C1TR23CON;
        tx_control[2] = (CxTRmnCON *)&C1TR45CON;
        tx_control[3] = (CxTRmnCON *)&C1TR67CON;

        for(loop = 0; loop < NUM_TX_CONTROL; loop++) {
                tx_control[loop]->TXENm = 1;
                tx_control[loop]->TXENn = 1;
        }
#endif
        /*
         * Set the number of DMA buffers we're using to 32
         */
        WIN_ZERO;
        C1FCTRLbits.DMABS = 0b110;   // 32 Buffers in RAM
        C1FCTRLbits.FSA   = 0x08;    // FIFO starts Buffer 8
        
        /*
         * All filters result in Rx'd frame going into FIFO
         */
        WIN_ONE;
        C1BUFPNT1bits.F0BP = 0xf;
        C1BUFPNT1bits.F1BP = 0xf;
        C1BUFPNT1bits.F2BP = 0xf;
        C1BUFPNT1bits.F3BP = 0xf;
        C1BUFPNT2bits.F4BP = 0xf;
        C1BUFPNT2bits.F5BP = 0xf;
        C1BUFPNT2bits.F6BP = 0xf;
        C1BUFPNT2bits.F7BP = 0xf;
        C1BUFPNT3bits.F8BP = 0xf;
        C1BUFPNT3bits.F9BP = 0xf;
        C1BUFPNT3bits.F10BP = 0xf;
        C1BUFPNT3bits.F11BP = 0xf;
        C1BUFPNT4bits.F12BP = 0xf;
        C1BUFPNT4bits.F13BP = 0xf;
        C1BUFPNT4bits.F14BP = 0xf;
        C1BUFPNT4bits.F15BP = 0xf;

        /*
         * All filters user mask 0
         */
        WIN_ZERO;
        C1FMSKSEL1 = 0x0000;
        C1FMSKSEL2 = 0x0000;

        /*
         * Setup mask zero, much don't care
         */
        WIN_ONE;
        C1RXM0SID = 0x0000;
        C1RXM0EID = 0x0000;
        
        /*
         * Setup mask one
         */
        C1RXM1SID = 0x0000;
        C1RXM1EID = 0x0000;

        /*
         * Setup mask two
         */
        C1RXM2SID = 0x0000;
        C1RXM2EID = 0x0000;

        /*
         * Enable all Rx Filters for the moment.
         */
        C1FEN1 = 0xffff;
        
        /*
         * Filter 0 Uses Mask 0
         */
        C1FMSKSEL1bits.F0MSK = MASK_0;
        
	/*
	 * Set the Baud rate.
	 */
        WIN_ZERO;
	set_bit_rate(arg_baud_rate);

        /*
         * Setup DMA Channel 0 for CAN 1 TX
         */
        DMA0CONbits.SIZE = 0x00;    // Word transfer mode
        DMA0CONbits.DIR = 0x01;     // Direction - Device RAM to Peripheral
        DMA0CONbits.AMODE = 0x02;   // Addressing mode: Peripheral indirect
        DMA0CONbits.MODE = 0x00;    // Operating Mode: Continuous no Ping Pong
        DMA0REQ = 70;               // ECAN1 Tx
        DMA0CNT = 7;                // Buffer size, 8 words
        
        DMA0PAD = (volatile unsigned int)&C1TXD;
        DMA0STAL = ((unsigned int)(&can_buffers)) & 0xffff;
        DMA0STAH = (((unsigned int)(&can_buffers)) >> 16) & 0xff;
        
        IEC0bits.DMA0IE = 1;        // Enable DMA-0 ISR        
        DMA0CONbits.CHEN = 0x01;    // Enable DMA-0
        
        /*
         * DMA Channel One for Rx
         */
        DMA1CONbits.SIZE = 0x0;    // Data Transfer Size: Word Transfer Mode
        DMA1CONbits.DIR = 0x0;     // Direction: Peripheral to device RAM 
        DMA1CONbits.AMODE = 0x2;   // Addressing Mode: Peripheral Indirect
        DMA1CONbits.MODE = 0x0;    // Operating Mode: Continuous, no Ping Pong
        DMA1REQ = 34;              // Assign ECAN1 Rx event for DMA Channel 1
        DMA1CNT = 7;               // DMA Transfer per ECAN message to 8 words

        /* 
         * Peripheral Address: ECAN1 Receive Register 
         */
        DMA1PAD = (volatile unsigned int) &C1RXD; 
        DMA1STAL = ((unsigned int)(&can_buffers)) & 0xffff;
        DMA1STAH = (((unsigned int)(&can_buffers)) >> 16) & 0xff;

//        DMA1STAL = (unsigned int) &can_buffers;
//        DMA1STAH = (unsigned int) &can_buffers;

        DMA1CONbits.CHEN = 0x1;    // Channel Enable: Enable DMA Channel 1
        IEC0bits.DMA1IE = 1;       // Enable DMA Channel 1 Interrupt
 
        /*
         * Clear all interrupt flags and enable all interrupts for the moment.
         */
        WIN_ZERO
        C1INTF = 0x00;        
        C1INTE = 0xff;
                
	/*
	 * Drop out of the configuration mode
	 * we're good to go
	 */
	set_mode(LOOPBACK_MODE);

//        can_status = Connected;

	// Create a random timer between 1 and 1.5 seconds for firing the
	// Network Idle Ping message
//	networkIdleDuration = (u16) ((rand() % 500) + 1000);

//#if DEBUG_LEVEL <= LOG_DEBUG
//	LOG_D("Network Idle Duration set to %d milliSeconds\n\r", networkIdleDuration);
//#endif
//	networkIdleTimer = start_timer(networkIdleDuration, pingNetwork, NULL);

        return(SUCCESS);
}
#endif // 0

void can_l2_tasks(void)
{
#if 0
	u8 i;
	u8 buffer;
	u8 *ptr;

	buffer = CANCON & 0x0f;

	/*
	 * Read all the messages present and process them
	 */
	while (rx_buffers[buffer]->ctrl & CNTL_RXFUL) {
		/*
		 * cancel the timer if running we've received a frame
		 */
		LOG_D(Debug, TAG, "Rx L2 Message so restart Idle Timer\n\r");
		cancel_timer(networkIdleTimer);
		networkIdleTimer = start_timer(networkIdleDuration, pingNetwork, NULL);
		if (rx_buffers[buffer]->sidl & SIDL_EXIDE) {
			/*
			 * Extended message identifer ???
			 */
			rxMsg.header.extended_id = TRUE;
			rxMsg.header.can_id.id = rx_buffers[buffer]->sidh;
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 3 | ((rx_buffers[buffer]->sidl >> 5) & 0x07);
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 2 | (rx_buffers[buffer]->sidl & 0x03);
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 8 | rx_buffers[buffer]->eid8;
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 8 | rx_buffers[buffer]->eid0;
		} else {
			rxMsg.header.extended_id = FALSE;

			/*
			 * Pull out the identifier
			 */
			rxMsg.header.can_id.id = rx_buffers[buffer]->sidh;
			rxMsg.header.can_id.id = rxMsg.header.can_id.id << 3 | ((rx_buffers[buffer]->sidl >> 5) & 0x07);
		}

		/*
		 * Pull out Data Length
		 */
		rxMsg.header.data_length = rx_buffers[buffer]->dcl & 0x0f;
		rxMsg.header.rnr_frame = rx_buffers[buffer]->dcl & DCL_RNR;

		/*
		 * Pull out the data bytes
		 */
		ptr = rx_buffers[buffer]->data;
		for (i = 0; i < rxMsg.header.data_length; i++, ptr++) {
			rxMsg.data[i] = *ptr;
		}

		/* Clear the received flag */
		rx_buffers[buffer]->ctrl &= ~CNTL_RXFUL;
		LOG_D("rxMsg %lx\n\r", rxMsg.header.can_id.id);

		if(l2Handler) {
			l2Handler(&rxMsg);
		} else {
			LOG_D("No Handler so ignoring received message\n\r");
		}
		buffer = CANCON & 0x0f;
	}
#endif //0
}

#if 0
result_t can_l2_tx_frame(can_frame *frame)
{
	u8 loop;
        u8 use_buffer;
//	u8 i;
//	u8 *ptr;

//	if (can_status != Connected) {
//		return (CAN_ERROR);
//	}

	LOG_D("L2_CanTxMessage(0x%lx)\n\r", frame->can_id);

        WIN_ONE
                
	/*
	 * Find a free buffer
	 */
        use_buffer = 0;
#if 0
        for(loop = 0; loop < NUM_TX_CONTROL; loop++) {
                if (!tx_control[loop]->TXREQm) {
                        use_buffer = loop * 2;
                        break;
                }
                
                if (!tx_control[loop]->TXREQn) {
                        use_buffer = (loop * 2) + 1;
                        break;
                }
        }
        
	if (loop == NUM_TX_CONTROL) {
		LOG_E("No empty TX buffer\n\r");
		return (ERR_NO_RESOURCES); //No Empty buffers
	}
#endif
        LOG_D("use_buffer %d\n\r", use_buffer);

	/*
	 * Trasmit buffer with index "use_buffer" is empty
	 * so fill in the registers with data.
	 */
        can_buffers[0].rb0 = 0;
        can_buffers[0].rb1 = 0;
        can_buffers[0].ide = 0;
        can_buffers[0].rtr = 0;
        can_buffers[0].ssr = 0;
        can_buffers[0].sid = 0x5aa;
        can_buffers[0].dlc = 0;
#if 0
        can_buffers[use_buffer].rb0 = 0;
        can_buffers[use_buffer].rb1 = 0;
        can_buffers[use_buffer].ide = frame->can_id & CAN_EFF_FLAG;
        can_buffers[use_buffer].rtr = frame->can_id & CAN_RTR_FLAG;
        can_buffers[use_buffer].ssr = can_buffers[use_buffer].rtr;
        can_buffers[use_buffer].sid = frame->can_id & CAN_SFF_MASK;

	if (can_buffers[use_buffer].ide) {
                can_buffers[use_buffer].ssr = 1;
                
                can_buffers[use_buffer].eid_h = (frame->can_id & 0x1ffe0000) >> 17;
                can_buffers[use_buffer].eid_l = (frame->can_id & 0x1f800) >> 11;
        }
        
        can_buffers[use_buffer].dlc = frame->can_dlc & 0x0f;

        if(!can_buffers[use_buffer].rtr) {
                for(loop = 0; loop < frame->can_dlc; loop++) {
                        can_buffers[use_buffer].data[loop] = frame->data[loop];
                }
        }
#endif   

	/*
	 * Mark the buffer for transmission
	 */
        C1TR01CONbits.TXREQ0 = 1;
        while(C1TR01CONbits.TXREQ0 == 1);
        
        LOG_D("Sent\n\r");
#if 0
        if(use_buffer & 0x01) {
                tx_control[use_buffer / 2]->TXREQn = 1;
        } else {
                tx_control[use_buffer / 2]->TXREQm = 1;
        }
#endif
	return (SUCCESS);
}

static void set_bit_rate(can_baud_rate_t baudRate)
{
	u8 sjw = 0;
	u8 brp = 0;
	u8 phseg1 = 0;
	u8 phseg2 = 0;
	u8 propseg = 0;

	switch (baudRate) {
		case baud_10K:
			brp = 49;
			propseg = 7;
			sjw = 3;
			phseg1 = 4;
			phseg2 = 4;
			break;

		case baud_20K:
			brp = 24;
			propseg = 7;
			sjw = 3;
			phseg1 = 4;
			phseg2 = 4;
			break;

		case baud_50K:
			brp = 9;
			propseg = 7;
			sjw = 3;
			phseg1 = 4;
			phseg2 = 4;
			break;

		case baud_125K:
			brp = 3;
			propseg = 7;
			sjw = 3;
			phseg1 = 4;
			phseg2 = 4;
			break;

		case baud_250K:
			brp = 1;
			propseg = 7;
			sjw = 3;
			phseg1 = 4;
			phseg2 = 4;
			break;

		case baud_500K:
			brp = 0;
			propseg = 7;
			sjw = 3;
			phseg1 = 4;
			phseg2 = 4;
			break;

		case baud_800K:
			brp = 0;
			propseg = 3;
			sjw = 2;
			phseg1 = 3;
			phseg2 = 3;
			break;

		case baud_1M:
			brp = 0;
			propseg = 3;
			sjw = 1;
			phseg1 = 2;
			phseg2 = 2;
			break;

		default:
			LOG_E("Invalid Baud Rate Specified\n\r");
			break;
	}

        C1CTRL1bits.CANCKS = 0;   //Use peripheral clock  
        C1CFG1bits.SJW = sjw;
        C1CFG1bits.BRP = brp;

        C1CFG2bits.PRSEG = propseg - 1;
        C1CFG2bits.SEG1PH = phseg1 - 1;
        C1CFG2bits.SEG2PH = phseg2 - 1;
        
	C1CFG2bits.SAM = 0; //One sampe point
}

/*
 * Function Header
 */
static void set_mode(u8 mode)
{
        C1CTRL1bits.REQOP = mode;

	/*
	 * CAN Mode is a request so we have to wait for confirmation
	 */
	while (C1CTRL1bits.OPMODE != mode) Nop();
}

result_t can_l2_dispatch_reg_handler(can_l2_target_t *target)
{
        LOG_D("can_l2_dispatch_reg_handler()\n\r");
        return(SUCCESS);
}
#endif // defined(__dsPIC33EP256MU806__)

#endif
