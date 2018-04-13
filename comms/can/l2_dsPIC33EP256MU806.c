/**
 * @file libesoup/comms/can/l2_dsPIC33EP256MU806.c
 *
 * @author John Whitmore
 * 
 * @brief Core SYS_CAN Functionality of electronicSoup CAN code
 *
 * Copyright 2017-2018 electronicSoup Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU Lesser General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#if defined(__dsPIC33EP256MU806__)

#include "libesoup_config.h"

#ifdef SYS_CAN_BUS

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
#include "libesoup/logger/serial_log.h"
static const char *TAG = "dsPIC33_CAN";
#endif // SYS_SERIAL_LOGGING

#ifndef SYS_SYSTEM_STATUS
#error "CAN Module relies on System Status module libesoup.h must define SYS_SYSTEM_STATUS"
#endif

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/peripheral.h"
#include "libesoup/timers/delay.h"
#include "libesoup/status/status.h"
#include "libesoup/comms/can/can.h"
#ifdef SYS_CAN_PING_PROTOCOL
#include "libesoup/comms/can/ping.h"
#endif // SYS_CAN_PING_PROTOCOL
#if defined(SYS_SW_TIMERS) && defined(SYS_DEBUG_BUILD)
#include "libesoup/timers/sw_timers.h"
#endif
/*
 * Check for required System Switches
 */
#ifndef SYS_SYSTEM_STATUS
#error "CAN Module relies on System Status module libesoup.h must define SYS_SYSTEM_STATUS"
#endif

#ifdef SYS_CAN_BAUD_AUTO_DETECT
uint16_t rx_frame_count = 0;

extern result_t can_bad_start_baud_scan();
#endif  // SYS_CAN_BAUD_AUTO_DETECT

#define NORMAL_MODE       0b000
#define DISABLE_MODE      0b001
#define LOOPBACK_MODE     0b010
#define LISTEN_ONLYMODE   0b011
#define CONFIG_MODE       0b100
#define LISTEN_ALL_MODE   0b111

typedef struct can_mask
{
    uint8_t   *sidh;
    uint8_t   *sidl;
    uint8_t   *eidh;
    uint8_t   *eidl;
} can_mask;

#define MASK_0    0b00
#define MASK_1    0b01
#define MASK_2    0b10

#define MEMORY_MAP_WIN_CONFIG_STATUS  C1CTRL1bits.WIN = 0;
#define MEMORY_MAP_WIN_MASK_FILTERS   C1CTRL1bits.WIN = 1;

static status_handler_t   status_handler = NULL;
static enum can_l2_status current_status;
static can_baud_rate_t    baud_rate;

struct  __attribute__ ((packed)) can_buffer
{
        uint16_t ide   :1;
        uint16_t ssr   :1;
        uint16_t sid   :11;
        uint16_t       :3;

        uint16_t eid_h :12;
        uint16_t       :4;

        uint16_t dlc   :4;
        uint16_t rb0   :1;
        uint16_t       :3;
        uint16_t rb1   :1;
        uint16_t rtr   :1;
        uint16_t eid_l :6;

        uint8_t  data[8];

        uint16_t       :8;
        uint16_t filhit:5;
        uint16_t       :3;
};

#define NUM_CAN_BUFFERS 32
__eds__ static struct can_buffer can_buffers[NUM_CAN_BUFFERS] __attribute__((eds,space(dma),aligned(NUM_CAN_BUFFERS * 16)));


struct __attribute__ ((packed)) TR_Control
{
        uint8_t priority       :2;
        uint8_t auto_remote_tx :1;
        uint8_t tx_request     :1;
        uint8_t tx_error       :1;
        uint8_t arbritation_err:1;
        uint8_t aborted        :1;
        uint8_t tx_buffer      :1;
};

#define NUM_TX_CONTROL  8

// WIN Bit = 0
struct TR_Control *tx_control = (struct TR_Control *)&C1TR01CON;

static ty_can_l2_mode requested_mode;
static result_t set_requested_mode();

static void set_mode(uint8_t mode);

result_t can_l2_bitrate(can_baud_rate_t baud);

static void reset(void)
{
	uint8_t          current_mode;

	current_mode = C1CTRL1bits.OPMODE;
	set_mode(CONFIG_MODE);
	delay(mSeconds, 5);
	set_mode(current_mode);
}

void __attribute__((__interrupt__, __no_auto_psv__)) _C1Interrupt(void)
{
	if(C1VECbits.ICODE == 0x40) {
		LOG_E("No ISR C1INTF %x\n\r", C1INTF);
	}
	
	while(C1INTF & 0xff) {
		// WIN Bit 0 | 1
		if(C1INTFbits.TBIF) {
			C1INTFbits.TBIF = 0;
			LOG_D("TBIF\n\r");
			if(current_status == can_l2_connecting) {
				current_status = can_l2_connected;
				if(status_handler) {
					status_handler(can_bus_l2_status, current_status, baud_rate);
				}
			}
			C1INTEbits.TBIE   = 0b00;  // Disable this interrupt no longer interested once connected
		}
	
		if(C1INTFbits.RBIF) {
			C1INTFbits.RBIF = 0;
			LOG_D("RBIF\n\r");
#ifdef SYS_CAN_BAUD_AUTO_DETECT
			rx_frame_count++;
#endif
		}
	
		if(C1INTFbits.RBOVIF) {
			C1INTFbits.RBOVIF = 0;
			LOG_D("RBOVIF\n\r");
		}
	
		if(C1INTFbits.FIFOIF) {
			C1INTFbits.FIFOIF = 0;
			LOG_D("FIFOIF\n\r");
		}
	
		if(C1INTFbits.ERRIF) {
			C1INTFbits.ERRIF = 0;
			LOG_D("ERRIF\n\r");
		}
	
		if(C1INTFbits.WAKIF) {
			C1INTFbits.WAKIF = 0;
			LOG_D("WAKIF\n\r");
		}
	
		if(C1INTFbits.IVRIF) {
			C1INTFbits.IVRIF = 0;
			LOG_D("IVRIF EC-RX %d  Tx %d\n\r", C1ECbits.RERRCNT, C1ECbits.TERRCNT);
		}
	
		if(C1INTFbits.EWARN) {
			LOG_D("EWARN\n\r");
		}
	
		if(C1INTFbits.RXWAR) {
			LOG_D("RXWAR EC %d\n\r", C1ECbits.RERRCNT);
		}
	
		if(C1INTFbits.TXWAR) {
			LOG_D("TXWAR EC %d\n\r", C1ECbits.TERRCNT);
		}
	
		if(C1INTFbits.RXBP) {
			LOG_D("RXBP EC %d\n\r", C1ECbits.RERRCNT);
		}
	
		if(C1INTFbits.TXBP) {
			LOG_D("TXBP EC %d\n\r", C1ECbits.TERRCNT);
		}
	
		if(C1INTFbits.TXBO) {
			LOG_D("TXBO\n\r");
		}
	}
        IFS2bits.C1IF   = 0x00;
}

#if defined(SYS_SW_TIMERS) && defined(SYS_DEBUG_BUILD)
void exp_fn(timer_id timer, union sigval data)
{
	LOG_D("EC-RX %d  Tx %d\n\r", C1ECbits.RERRCNT, C1ECbits.TERRCNT);	
}
#endif

/*
 */
result_t can_l2_init(can_baud_rate_t arg_baud_rate, status_handler_t arg_status_handler, ty_can_l2_mode mode)
{
	result_t          rc;
	uint32_t          address;
	uint8_t           loop;
#if defined(SYS_SW_TIMERS) && defined(SYS_DEBUG_BUILD)
	struct timer_req  request;
#endif
	
	if (arg_baud_rate < no_baud) {
		LOG_D("L2_CanInit() Baud Rate %s\n\r", can_baud_rate_strings[arg_baud_rate]);
		baud_rate = arg_baud_rate;
		current_status = can_l2_connecting;
	} else {
#ifndef SYS_CAN_BAUD_AUTO_DETECT
		return (-ERR_BAD_INPUT_PARAMETER);
#else
		LOG_D("L2_CanInit() Auto Detect Baud Rate\n\r");
		current_status = can_l2_detecting_baud;
#endif
	}

#if defined(SYS_SW_TIMERS) && defined(SYS_DEBUG_BUILD)
	request.units          = Seconds;
	request.duration       = 10;
	request.type           = repeat;
	request.exp_fn         = exp_fn;
	request.data.sival_int = 0;
	rc = sw_timer_start(&request);
	RC_CHECK	
#endif
	
	status_handler = arg_status_handler;
	requested_mode = mode;

        /*
         * Initialise the I/O Pins and peripheral functions
         */
	rc = gpio_set(BRD_CAN_RX_PIN, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK
	rc = PPS_I_CAN1_RX = set_peripheral_input(BRD_CAN_RX_PIN);
	RC_CHECK
	
	rc = gpio_set(BRD_CAN_TX_PIN, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK
	rc = set_peripheral_output(BRD_CAN_TX_PIN, PPS_O_CAN1_TX);
	RC_CHECK

        /*
         * Enter configuration mode
         */
	set_mode(CONFIG_MODE);

	MEMORY_MAP_WIN_CONFIG_STATUS
		
	/*
	 * No Device net filtering
	 */
	C1CTRL2bits.DNCNT = 0x00;
	
	/*
	 * CAN Bus not used for wakeup
	 */
	C1CFG2bits.WAKFIL = 0b0;

	/*
         * Set 8 transmit buffers
         */
	for(loop = 0; loop < NUM_TX_CONTROL; loop++) {
		tx_control[loop].tx_buffer = 0b1;       // WIN Bit = 0
	}

        /*
         * Set the number of DMA buffers we're using to 32
         */	
	MEMORY_MAP_WIN_CONFIG_STATUS

	// WIN Bit 0 | 1
        C1FCTRLbits.FSA   = 0x08;    // FIFO starts Buffer 8
	C1FCTRLbits.DMABS = 0b110;   // 32 Buffers in RAM
        
        /*
         * All filters result in Rx'd frame going into FIFO
         */
	MEMORY_MAP_WIN_MASK_FILTERS
	// WIN Bit = 1
        C1BUFPNT1 = 0xffff;
        C1BUFPNT2 = 0xffff;
        C1BUFPNT3 = 0xffff;
        C1BUFPNT4 = 0xffff;

        /*
         * All filters use mask 0
         */
	// WIN Bit 0 | 1
        C1FMSKSEL1 = 0x0000;
	// WIN Bit 0 | 1
        C1FMSKSEL2 = 0x0000;

        /*
         * Setup mask zero, much don't care
         */
	// WIN Bit = 1
        C1RXM0SID = 0x0000;
	// WIN Bit = 1
        C1RXM0EID = 0x0000;
        
        /*
         * Setup mask one
         */
	// WIN Bit = 1
        C1RXM1SID = 0x0000;
	// WIN Bit = 1
        C1RXM1EID = 0x0000;

        /*
         * Setup mask two
         */
	// WIN Bit = 1
        C1RXM2SID = 0x0000;
	// WIN Bit = 1
        C1RXM2EID = 0x0000;

        /*
         * Enable all Rx Filters for the moment.
         */
	// WIN Bit 0 | 1
        C1FEN1 = 0xffff;

	MEMORY_MAP_WIN_CONFIG_STATUS
	/*
         * Setup DMA Channel 0 for CAN 1 TX
         */
        DMA0CONbits.SIZE  = 0x00;   // Word transfer mode
        DMA0CONbits.DIR   = 0x01;   // Direction - Device RAM to Peripheral
        DMA0CONbits.AMODE = 0x02;   // Addressing mode: Peripheral indirect
        DMA0CONbits.MODE  = 0x00;   // Operating Mode: Continuous no Ping Pong
        DMA0REQ = 70;               // ECAN1 Tx
        DMA0CNT = 7;                // Buffer size, 8 words
        
	// WIN Bit = 0
        DMA0PAD = (volatile unsigned int)&C1TXD;
        
        address  = (uint32_t)(&can_buffers);
        DMA0STAL = (uint16_t)(address & 0xffff);
        DMA0STAH = (uint16_t)((address >> 16) & 0xff);

//	IEC0bits.DMA0IE = ENABLED;
        DMA0CONbits.CHEN = ENABLED;

        /*
         * DMA Channel One for Rx
         */
        DMA1CONbits.SIZE  = 0x0;    // Data Transfer Size: Word Transfer Mode
        DMA1CONbits.DIR   = 0x0;    // Direction: Peripheral to device RAM 
        DMA1CONbits.AMODE = 0x2;    // Addressing Mode: Peripheral Indirect
        DMA1CONbits.MODE  = 0x0;    // Operating Mode: Continuous, no Ping Pong
        DMA1REQ = 34;               // Assign ECAN1 Rx event for DMA Channel 1
        DMA1CNT = 7;                // DMA Transfer per ECAN message to 8 words

        /* 
         * Peripheral Address: ECAN1 Receive Register 
         */
	// WIN Bit = 0
        DMA1PAD = (volatile unsigned int) &C1RXD; 
        
        address  = (uint32_t)(&can_buffers);
        DMA1STAL = (uint16_t)(address & 0xffff);
        DMA1STAH = (uint16_t)((address >> 16) & 0xff);

        DMA1CONbits.CHEN = 0x1;    // Channel Enable: Enable DMA Channel 1
//        IEC0bits.DMA1IE  = 1;       // Enable DMA Channel 1 Interrupt
 
        /*
         * Clear all interrupt flags and enable all interrupts for the moment.
         */
	MEMORY_MAP_WIN_CONFIG_STATUS
        C1INTF = 0x00;
//        C1INTEbits.ERRIE  = 0b01;
//        C1INTEbits.IVRIE  = 0b01;
        C1INTEbits.FIFOIE = 0b01;
        C1INTEbits.TBIE   = 0b01;

        IFS2bits.C1IF   = 0x00;
        IEC2bits.C1IE   = 0x01;

	if (arg_baud_rate < no_baud) {
		current_status = can_l2_connecting;
		/*
	         * Set the baud rate
	         */
		rc = can_l2_bitrate(arg_baud_rate);
		RC_CHECK

		/*
	         * Drop out of the configuration mode
	         */
		rc = set_requested_mode();
		RC_CHECK

		if(status_handler) {
			status_handler(can_bus_l2_status, current_status, 0);
		}
	} else if (current_status == can_l2_detecting_baud) {
#ifdef SYS_CAN_BAUD_AUTO_DETECT
		/*
		 * Enable the Rx Interrupt
		 */
		C1INTEbits.RBIE   = 0b01;
		set_mode(LISTEN_ONLYMODE);
		
		/*
		 */
		rc = can_bad_start_baud_scan();
		RC_CHECK

		if(status_handler) {
			status_handler(can_bus_l2_status, current_status, 0);
		}
#endif
	}
        return(0);
}

/*
 * Simple helper function to set the requested mode of operation
 */
static result_t set_requested_mode()
{
	switch(requested_mode) {
	case normal:
		set_mode(NORMAL_MODE);
		break;
	case loopback:
		set_mode(LOOPBACK_MODE);
		break;
	case listen_only:
		set_mode(LISTEN_ONLYMODE);
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	return(0);
}

result_t can_l2_tx_frame(can_frame *frame)
{
	uint8_t  loop;
	uint8_t  data_loop;
	
	/*
	 * Find a free TX Buffer
	 */
	for(loop = 0; loop < NUM_TX_CONTROL; loop++) {
		if(tx_control[loop].tx_buffer && !tx_control[loop].tx_request) {			
			if(frame->can_id & CAN_EFF_FLAG) {
				can_buffers[loop].ide = 0b1;
				can_buffers[loop].ssr = 0b1;
				can_buffers[loop].rtr = frame->can_id & CAN_RTR_FLAG;
				can_buffers[loop].eid_l = (frame->can_id >> 11) & 0b111111; 
				can_buffers[loop].eid_h = (frame->can_id >> 17) & 0x0fff;
			} else {
				can_buffers[loop].sid = frame->can_id & CAN_SFF_MASK;
				can_buffers[loop].ssr = frame->can_id & CAN_RTR_FLAG;
			}

			can_buffers[loop].dlc = frame->can_dlc;			
			for(data_loop = 0; data_loop < frame->can_dlc; data_loop++) {
				can_buffers[loop].data[data_loop] = frame->data[data_loop];
			}
			/*
			 * Mark the buffer for transmission
			 */
			tx_control[loop].tx_request = 0b1;

#ifdef SYS_CAN_PING_PROTOCOL
			restart_ping_timer();
#endif // SYS_CAN_PING_PROTOCOL
			
			return(0);
		}
	}
	return(-ERR_NO_RESOURCES);
}

void can_l2_tasks(void)
{
	static can_frame frame;
	uint8_t          fifo_rd_index;
	boolean          buffer_full;
	uint8_t          loop;

	MEMORY_MAP_WIN_CONFIG_STATUS

	fifo_rd_index = C1FIFObits.FNRB;
	
	if(fifo_rd_index < 16) {
		buffer_full = (C1RXFUL1 & (0x01 << fifo_rd_index)) != 0;
	} else {
		buffer_full = (C1RXFUL2 & (0x01 << (fifo_rd_index - 16))) != 0;
	}

	if(buffer_full && (current_status == can_l2_connecting)) {
		current_status = can_l2_connected;
		if(status_handler) {
			status_handler(can_bus_l2_status, current_status, baud_rate);
		}
	}

	while(buffer_full) {
		/*
		 * Check for overflow in the buffer
		 */
		if(  ((fifo_rd_index < 16) && C1RXOVF1 & (0x01 << fifo_rd_index))
		   || (C1RXOVF2 & (0x01 << (fifo_rd_index - 16)))) {
			/*
			 * Todo - notify the system status handler
			 */
			LOG_E("CAN Overflow\n\r");
		}
		/*
		 * Process the Rx'd buffer
		 */
		frame.can_id  = 0x00;  // Just to be safe clear the ID
		frame.can_id  = can_buffers[fifo_rd_index].sid;
		frame.can_id  |= ((uint32_t)can_buffers[fifo_rd_index].ide << 31);
		if (can_buffers[fifo_rd_index].ide) {
			/*
			 * Extended message received
			 */
			frame.can_id  |= ((uint32_t)can_buffers[fifo_rd_index].rtr << 30);
			frame.can_id  |= ((uint32_t)can_buffers[fifo_rd_index].eid_l << 11);
			frame.can_id  |= ((uint32_t)can_buffers[fifo_rd_index].eid_h << 17);
		} else {
			/*
			 * Standard message received
			 */
			frame.can_id  |= ((uint32_t)can_buffers[fifo_rd_index].ssr << 30);
		}
		frame.can_dlc = can_buffers[fifo_rd_index].dlc;

		for(loop = 0; loop < frame.can_dlc; loop++) {
			frame.data[loop] = can_buffers[fifo_rd_index].data[loop];
		}
#ifdef SYS_CAN_PING_PROTOCOL
		restart_ping_timer();
#endif // SYS_CAN_PING_PROTOCOL
		
		frame_dispatch_handle_frame(&frame);

		/*
		 * Mark the buffer as read
		 */
		if(fifo_rd_index < 16) {
			C1RXFUL1 &= ~(0x01 << fifo_rd_index);
		} else {
			C1RXFUL2 &= ~(0x01 << (fifo_rd_index - 16));		
		}
		
		/*
		 * Check is there another full buffer
		 */
		fifo_rd_index = C1FIFObits.FNRB;
		if(fifo_rd_index < 16) {
			buffer_full = (C1RXFUL1 & (0x01 << fifo_rd_index)) != 0;
		} else {
			buffer_full = (C1RXFUL2 & (0x01 << (fifo_rd_index - 16))) != 0;
		}
	}
}

#ifdef SYS_CAN_BAUD_AUTO_DETECT
result_t can_l2_get_rx_count(void)
{
	return(rx_frame_count);
}
#endif

#ifdef SYS_CAN_BAUD_AUTO_DETECT
result_t can_l2_baud_found(can_baud_rate_t rate)
{
	result_t rc;

	/*
	 * Finished with the Rx Interrupt
	 */
	C1INTEbits.RBIE   = 0b00;
	
	baud_rate = rate;
	current_status = can_l2_connecting;

	/*
	 * Set the baud rate
	 */
	rc = can_l2_bitrate(baud_rate);
	RC_CHECK

	/*
         * Drop out of the configuration mode
         */
	rc = set_requested_mode();
	RC_CHECK

	if(status_handler) {
		status_handler(can_bus_l2_status, current_status, 0);
	}
	return(0);
}
#endif

/*
 */
result_t can_l2_bitrate(can_baud_rate_t baud)
{
	result_t rc = 0;
	uint32_t bit_freq;
	uint32_t tq_freq = 0;
	uint8_t  tq_count = 25;
	uint8_t  sjw = 1;
	uint8_t  brp;
	uint8_t  phseg1 = 0;
	uint8_t  phseg2 = 0;
	uint8_t  propseg = 0;
	uint8_t  current_mode;

	switch (baud) {
	case baud_10K:
		bit_freq = 10000;
		break;
	case baud_20K:
		bit_freq = 20000;
		break;
	case baud_50K:
		bit_freq = 50000;
		break;
	case baud_125K:
		bit_freq = 125000;
		break;
	case baud_250K:
		bit_freq = 250000;
		break;
	case baud_500K:
		bit_freq = 500000;
		break;
	case baud_800K:
		bit_freq = 800000;
		break;
	case baud_1M:
		bit_freq = 1000000;
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}

	/*
	 * Assume that the requested bit rate is one we can't attain
	 */
	rc = -ERR_CAN_BITRATE_LOW;
	
	for(brp = 1; brp <= 64 && (rc != 0); brp++) {
		/*
		 * Calculate the potential TQ Frequency (Fp/brp/2) 
		 * We're only looking for integer values of bit rate so ignore
		 * fractional results of division by BRP.
		 */
		if((sys_clock_freq % brp != 0) || (((sys_clock_freq / brp) % 2) != 0)) continue;
		tq_freq = ((sys_clock_freq / brp) / 2);
		
		/*
		 * Check for a requested bit rate which is too high to attain
		 */
		if (brp == 1 && (bit_freq > tq_freq/8)) {
			return(-ERR_CAN_BITRATE_HIGH);
		}
		
		/*
		 * The Bits can be made up of from between 8 and 25 TQ periods
		 * so test to see if the calculated potential tq_freq can give
		 * us the required bit frequency.
		 */
		if( (bit_freq >= tq_freq/25) && (bit_freq <= tq_freq/8) ) {
			/*
			 * Now have a suitable tq Frequency have to establish
			 * the number (8 - 25) of Tq periods required  
			 */
			for(tq_count = 25; tq_count >= 8 && (rc != 0); tq_count--) {
				if(tq_freq % tq_count != 0) continue;
				if(tq_freq / tq_count == bit_freq) {
					rc = 0;
				}
			}
		}
	}
	RC_CHECK

	brp--;       // End of the found loop will have incremented
	tq_count++;  // ^^^
	
	sjw = 1;
	phseg1 = phseg2 = (tq_count - sjw) / 3;
	propseg = tq_count - sjw - phseg1 - phseg2;

	/*
	 * store the current mode to return to it on exit
	 */
	current_mode = C1CTRL1bits.OPMODE;
	
        /*
         * Enter configuration mode
         */
	set_mode(CONFIG_MODE);

#ifdef SYS_CAN_BAUD_AUTO_DETECT
	if (current_status == can_l2_detecting_baud) {
		rx_frame_count = 0;
	}
#endif
	/*
	 * CANCKS: ECAN Module Clock Freg(CAN) Source Select bit
	 * 0 = Freq(CAN) is equal to 2 * Freq(Peripheral)
	 * 1 = Freq(CAN) is equal to Freq(Peripheral)
	 */
	// WIN Bit 0 | 1
	//Use peripheral clock N.B. Reverse of documentation, see Errata
	C1CTRL1bits.CANCKS = 1;
	// WIN Bit 0 | 1
	C1CFG1bits.SJW = sjw;
	// WIN Bit 0 | 1
	C1CFG1bits.BRP = brp - 1;

	// WIN Bit 0 | 1
	C1CFG2bits.PRSEG = propseg - 1;
	// WIN Bit 0 | 1
	C1CFG2bits.SEG1PH = phseg1 - 1;
	// WIN Bit 0 | 1
	C1CFG2bits.SEG2PH = phseg2 - 1;
        
	// WIN Bit 0 | 1
	C1CFG2bits.SAM = 0; //One sampe point

//	LOG_I("propseg-%d, phseg1-%d, phseg2-%d\n\r", propseg, phseg1, phseg2);

	/*
	 * Drop out of the configuration mode
	 */
	set_mode(current_mode);
	return(0);
}

/*
 * Function Header
 */
static void set_mode(uint8_t mode)
{
	// WIN Bit 0/1
	C1CTRL1bits.REQOP = mode;

	/*
	 * CAN Mode is a request so we have to wait for confirmation
	 */
	while (C1CTRL1bits.OPMODE != mode) Nop();
}

#endif // #ifdef SYS_CAN_BUS

#endif //  defined(__dsPIC33EP256MU806__)
