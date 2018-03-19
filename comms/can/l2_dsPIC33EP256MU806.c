/**
 *
 * \file libesoup/comms/can/l2_dsPIC33EP256MU806.c
 *
 * Core SYS_CAN Functionality of electronicSoup CAN code
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

#include <p33EP256MU806.h>

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
#include "libesoup/status/status.h"
#include "libesoup/comms/can/l2_dsPIC33EP256MU806.h"
#include "libesoup/comms/can/can.h"
#include "libesoup/comms/can/frame_dispatch.h"

/*
 * Check for required System Switches
 */
#ifndef SYS_SYSTEM_STATUS
#error "CAN Module relies on System Status module libesoup.h must define SYS_SYSTEM_STATUS"
#endif

#define MASK_0    0b00
#define MASK_1    0b01
#define MASK_2    0b10

#define MEMORY_MAP_WIN_CONFIG_STATUS  C1CTRL1bits.WIN = 0;
#define MEMORY_MAP_WIN_MASK_FILTERS   C1CTRL1bits.WIN = 1;

 static status_handler_t status_handler = NULL;
 
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

static void set_mode(ty_can_mode mode);
static result_t set_bitrate(can_baud_rate_t baudRate);

void __attribute__((__interrupt__, __no_auto_psv__)) _C1RxRdyInterrupt(void)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("C1RxRdy Isr\n\r");
#endif
        IEC2bits.C1IE = 0x00;
        IEC2bits.C1RXIE = 0x00;
}

void __attribute__((__interrupt__, __no_auto_psv__)) _C1Interrupt(void)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("C1 Isr Flag 0x%x - 0x%x ICODE 0x%x\n\r", C1INTF, C1INTF, C1VECbits.ICODE);
#endif
        IEC2bits.C1IE = 0x00;
        IEC2bits.C1RXIE = 0x00;

	// WIN Bit 0 | 1
        if(C1INTFbits.TBIF) {
                C1INTFbits.TBIF = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("TBIF\n\r");
#endif
        } else if(C1INTFbits.RBIF) {
                C1INTFbits.RBIF = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("RBIF\n\r");
#endif
        } else if(C1INTFbits.RBOVIF) {
                C1INTFbits.RBOVIF = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("RBOVIF\n\r");
#endif
        } else if(C1INTFbits.FIFOIF) {
                C1INTFbits.FIFOIF = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("FIFOIF\n\r");
#endif
        } else if(C1INTFbits.ERRIF) {
                C1INTFbits.ERRIF = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("ERRIF\n\r");
#endif
        } else if(C1INTFbits.WAKIF) {
                C1INTFbits.WAKIF = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("WAKIF\n\r");
#endif
        } else if(C1INTFbits.IVRIF) {
                C1INTFbits.IVRIF = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("IVRIF\n\r");
#endif
        } else if(C1INTFbits.EWARN) {
                C1INTFbits.EWARN = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("EWARN\n\r");
#endif
        } else if(C1INTFbits.RXWAR) {
                C1INTFbits.RXWAR = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("RXWAR\n\r");
#endif
        } else if(C1INTFbits.TXWAR) {
                C1INTFbits.TXWAR = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("TXWAR\n\r");
#endif
        } else if(C1INTFbits.RXBP) {
                C1INTFbits.RXBP = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("RXBP\n\r");
#endif
        } else if(C1INTFbits.TXBP) {
                C1INTFbits.TXBP = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("TXBP\n\r");
#endif
        } else if(C1INTFbits.TXBO) {
                C1INTFbits.TXBO = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("TXBO\n\r");
#endif
        } else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                LOG_D("Unprocessed ISR\n\r");
#endif
        }
}

void __attribute__((__interrupt__, __no_auto_psv__)) _DMA0Interrupt(void)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("DMA-0 ISR");
#endif
        IFS0bits.DMA0IF = 0;
        IEC0bits.DMA0IE = DISABLED;
}

void __attribute__((__interrupt__, __no_auto_psv__)) _DMA1Interrupt(void)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("DMA-1 ISR");
#endif
        IEC0bits.DMA1IE = DISABLED;
}

/*
 */
result_t can_l2_init(can_baud_rate_t arg_baud_rate, status_handler_t arg_status_handler)
{
	result_t rc;
	uint32_t address;
	uint8_t  loop;
	
	if (arg_baud_rate <= no_baud) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("L2_CanInit() Baud Rate %s\n\r", can_baud_rate_strings[arg_baud_rate]);
#endif
	} else {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("L2_CanInit() ToDo!!! No Baud Rate Specified\n\r");
#endif
		return (-ERR_BAD_INPUT_PARAMETER);
	}

	status_handler = arg_status_handler;
	
        /*
         * Initialise the I/O Pins and pipheral functions
         */
	CAN_RX_PIN_ANSEL = DIGITAL_PIN;
        CAN_RX_PIN_DIRECTION = INPUT_PIN;
	PPS_CAN1_RX = CAN_RX_INPUT_PERIPHERAL_PIN;
	
        CAN_TX_PIN_DIRECTION = OUTPUT_PIN;
	CAN_TX_OUTPUT_PERIPHERAL_PIN = PPS_CAN1_TX;

        /*
         * Enter configuration mode
         */
	set_mode(config);

	rc = set_bitrate(baud_125K);
	if(rc < 0) return(rc);

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

	/*
	 * Filter on the Standard Indentifier we're sending
	 */
	// Todo get rid of this it's a quick test
	C1RXF0SIDbits.SID = 0x555;
        
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
        C1INTEbits.ERRIE  = 0b01;
        C1INTEbits.IVRIE  = 0b01;
        C1INTEbits.FIFOIE = 0b01;

        IFS2bits.C1IF   = 0x00;
        IFS2bits.C1RXIF = 0x00;
        IEC2bits.C1IE   = 0x01;
        IEC2bits.C1RXIE = 0x00;

	/*
	 * Drop out of the configuration mode
	 */
#ifdef SYS_CAN_LOOPBACK
	set_mode(loopback);
#else
	set_mode(normal);
#endif
	
        return(0);
}

result_t can_l2_tx_frame(can_frame *frame)
{
	uint8_t  loop;
	
	/*
	 * Find a free TX Buffer
	 */
	for(loop = 0; loop < NUM_TX_CONTROL; loop++) {
		if(tx_control[loop].tx_buffer && !tx_control[loop].tx_request) {
			can_buffers[loop].sid = 0x00;
			can_buffers[loop].sid = frame->can_id & CAN_SFF_MASK;
			
			if(frame->can_id & CAN_EFF_FLAG) {
				can_buffers[loop].ide = 0b1;
				can_buffers[loop].ssr = 0b1;
				can_buffers[loop].rtr = frame->can_id & CAN_RTR_FLAG;
				can_buffers[loop].eid_l = (frame->can_id >> 11) & 0b111111; 
				can_buffers[loop].eid_h = (frame->can_id >> 17) & 0x0fff;
			} else {
				can_buffers[loop].ssr = frame->can_id & CAN_RTR_FLAG;
			}
			
			/*
			 * Mark the buffer for transmission
			 */
			tx_control[loop].tx_request = 0b1;
			
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

	while(buffer_full) {
		/*
		 * Check for overflow in the buffer
		 */
		if(  ((fifo_rd_index < 16) && C1RXOVF1 & (0x01 << fifo_rd_index))
		   || (C1RXOVF2 & (0x01 << (fifo_rd_index - 16)))) {
			/*
			 * Todo - notify the system status handler
			 */
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("CAN Overflow\n\r");
#endif	
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

static result_t set_bitrate(can_baud_rate_t baud)
{
	uint32_t bit_freq;
	uint32_t tq_freq;
	uint8_t  tq_count = 25;
	boolean  found = FALSE;
	uint8_t  sjw = 1;
	uint8_t  brp;
	uint8_t  phseg1 = 0;
	uint8_t  phseg2 = 0;
	uint8_t  propseg = 0;

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

	for(brp = 1; brp <= 64 && !found; brp++) {
		/*
		 * Calculate the potential TQ Frequency (Fp/brp/2) 
		 */
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
//		LOG_D("Testing BRP %d\n\r", brp);
//		LOG_D("remainder %ld\n\r", sys_clock_freq % brp);
#endif		
		if((sys_clock_freq % brp != 0) || (((sys_clock_freq / brp) % 2) != 0)) continue;
		tq_freq = ((sys_clock_freq / brp) / 2);
		
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
			for(tq_count = 25; tq_count >= 8 && !found; tq_count--) {
				if(tq_freq % tq_count != 0) continue;
				if(tq_freq / tq_count == bit_freq) {
					found = TRUE;
				}
			}
		}
	}	

	if(!found) return(-ERR_CAN_INVALID_BAUDRATE);

	brp--;       // End of the found loop will have incremented
	tq_count++;  // ^^^
	
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Fp %ld\n\r", sys_clock_freq);
	LOG_D("BRP %d\n\r", brp);
	LOG_D("Ftq %ld\n\r", tq_freq);
	LOG_D("TQ Periods %d\n\r", tq_count);
#endif
	
	sjw = 1;
	phseg1 = phseg2 = (tq_count - sjw) / 3;
	propseg = tq_count - sjw - phseg1 - phseg2;

	/*
	 * CANCKS: ECAN Module Clock Freg(CAN) Source Select bit
	 * 1 = Freq(CAN) is equal to 2 * Freq(Peripheral)
	 * 0 = Freq(CAN) is equal to Freq(Peripheral)
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

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("propseg-%d, phseg1-%d, phseg2-%d\n\r", propseg, phseg1, phseg2);
#endif
	
	return(0);
}

/*
 * Function Header
 */
static void set_mode(ty_can_mode mode)
{
	// WIN Bit 0/1
	C1CTRL1bits.REQOP = mode;

	/*
	 * CAN Mode is a request so we have to wait for confirmation
	 */
	while (C1CTRL1bits.OPMODE != mode) Nop();
}

#if 0
result_t can_l2_dispatch_reg_handler(can_l2_target_t *target)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("can_l2_dispatch_reg_handler()\n\r");
#endif
        return(0);
}
#endif // defined(__dsPIC33EP256MU806__)

#endif // #ifdef SYS_CAN_BUS

#endif //  defined(__dsPIC33EP256MU806__)
