/**
 * @file libesoup/comms/usb/keyboard/usb_keyboard.c
 *
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
 *
 * This file contains all the functionality for our USB Keyboard and the Single
 * API Function.
 *     Based on Microchips example 
 *     .../mla/v2018_11_26/apps/usb/device/hid_keyboard/firmware/demo_src/app_device_keyboard.c
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
//#define TAG "USB_KBD"
//#define DEBUG_FILE

#include "libesoup_config.h"

#ifdef SYS_USB_KEYBOARD

#include "libesoup/logger/serial_log.h"

#include <stdint.h>
#include <string.h>

#include "system.h"
#include "usb.h"
#include "usb_device_hid.h"

//Class specific descriptor - HID Keyboard
const struct{uint8_t report[HID_RPT01_SIZE];}hid_rpt01={
{   0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0}                          // End Collection
};


typedef struct __attribute__((packed))
{
    union __attribute__((packed))
    {
        uint8_t value;
        struct __attribute__((packed))
        {
            unsigned leftControl    :1;
            unsigned leftShift      :1;
            unsigned leftAlt        :1;
            unsigned leftGUI        :1;
            unsigned rightControl   :1;
            unsigned rightShift     :1;
            unsigned rightAlt       :1;
            unsigned rightGUI       :1;
        } bits;
    } modifiers;

    unsigned :8;

    uint8_t keys[6];
} KEYBOARD_INPUT_REPORT;


typedef union __attribute__((packed))
{
	uint8_t value;
	struct
	{
		unsigned numLock        :1;
		unsigned capsLock       :1;
		unsigned scrollLock     :1;
		unsigned compose        :1;
		unsigned kana           :1;

		unsigned                :3;
	} leds;
} KEYBOARD_OUTPUT_REPORT;


typedef struct
{
	USB_HANDLE lastINTransmission;
	USB_HANDLE lastOUTTransmission;
	unsigned char key;
	bool waitingForRelease;
} KEYBOARD;

static KEYBOARD keyboard;

static uint8_t have_key = 0;
static uint8_t key      = 0;
static uint8_t shift    = 0;

#if !defined(KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG)
    #define KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG
#endif
static KEYBOARD_INPUT_REPORT inputReport KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG;

#if !defined(KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG)
    #define KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG
#endif
static volatile KEYBOARD_OUTPUT_REPORT outputReport KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG;


static void APP_KeyboardProcessOutputReport(void);

KEYBOARD_INPUT_REPORT oldInputReport;
signed int keyboardIdleRate;
signed int LocalSOFCount;
static signed int OldSOFCount;

static void usb_keyboard_init(void);


volatile signed int SOFCounter = 0;

void SYSTEM_Initialize( SYSTEM_STATE state )
{
	switch(state)
	{
        case SYSTEM_STATE_USB_START:
		LOG_D("Start!\n\r");
		// Configure the device PLL to obtain 60 MIPS operation. The crystal
		// frequency is 16MHz. Divide 16MHz by 2, multiply by 60 and divide by
		// 2. This results in Fosc of 120MHz. The CPU clock frequency is
		// Fcy = Fosc/2 = 60MHz. Wait for the Primary PLL to lock and then
		// configure the auxilliary PLL to provide 48MHz needed for USB
		// Operation.
		//
		// Fcy = Insctruciton clock frequency = Fosc / 2
		// Fosc = (Fcrystal * M) / (N1 * N2)
		// 
		// (16M * 60) / (4 * 2) = 120M
            

		PLLFBD = 58;                        /* M  = 60  */
		CLKDIVbits.PLLPOST = 0;             /* N2 = 2   */
		CLKDIVbits.PLLPRE = 2;              /* N1 = 4   */
		OSCTUN = 0;

		/* Initiate Clock Switch to Primary
		 * Oscillator with PLL (NOSC= 0x3)*/
		__builtin_write_OSCCONH(0x03);
		__builtin_write_OSCCONL(0x01);

		while (OSCCONbits.COSC != 0x3);

		// Configuring the auxiliary PLL, since the primary
		// oscillator provides the source clock to the auxiliary
		// PLL, the auxiliary oscillator is disabled. Note that
		// the AUX PLL is enabled. The input 16MHz clock is divided
		// by 2, multiplied by 24 and then divided by 2. Wait till
		// the AUX PLL locks.
            
		// (16M * 24) / (4 * 2) = 48MHz

		ACLKCON3 = 0x24C1;
		ACLKCON3bits.ENAPLL = 0b0;
		ACLKCON3bits.SELACLK = 0b1;
		ACLKCON3bits.AOSCMD = 0b00;
		ACLKCON3bits.ASRCSEL = 0b1;
		ACLKCON3bits.APLLPOST = 0b110;     // N2 divide by 2
		ACLKCON3bits.APLLPRE = 0b011;      // N1 divide by 4
		ACLKDIV3bits.APLLDIV = 0b111;      // M  times 24 

		ACLKCON3bits.ENAPLL = 1;
		while(ACLKCON3bits.APLLCK != 1);

		break;

        case SYSTEM_STATE_USB_SUSPEND:
		LOG_D("Suspend!\n\r");
		USBSleepOnSuspend();
		break;

        case SYSTEM_STATE_USB_RESUME:
		LOG_D("Resume!\n\r");
		break;

        default:
		break;
	}
}

void __attribute__((interrupt,auto_psv)) _USB1Interrupt()
{
	USBDeviceTasks();
}

bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
	switch((int)event)
	{
        case EVENT_TRANSFER:
		break;

        case EVENT_SOF:
		if(SOFCounter < 32767) {
			SOFCounter++;
		} else {
			SOFCounter = 0;
		}
		break;

        case EVENT_SUSPEND:
		SYSTEM_Initialize(SYSTEM_STATE_USB_SUSPEND);
		break;

        case EVENT_RESUME:
		SYSTEM_Initialize(SYSTEM_STATE_USB_RESUME);
		break;

        case EVENT_CONFIGURED:
		usb_keyboard_init();
		break;

        case EVENT_SET_DESCRIPTOR:
		break;

        case EVENT_EP0_REQUEST:
		USBCheckHIDRequest();
		break;

        case EVENT_BUS_ERROR:
		break;

        case EVENT_TRANSFER_TERMINATED:
		break;

        default:
		break;
	}
	return true;
}


void usb_keyboard_init(void)
{
	keyboard.lastINTransmission = 0;
    
	keyboard.waitingForRelease = false;

	//Set the default idle rate to 500ms (until the host sends a SET_IDLE request to change it to a new value)
	keyboardIdleRate = 500;

	while(OldSOFCount != SOFCounter)
	{
		OldSOFCount = SOFCounter;
	}

	USBEnableEndpoint(HID_EP, USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
}

result_t usb_keyboard_tasks(void)
{
	signed int TimeDeltaMilliseconds;
	unsigned char i;
	bool needToSendNewReportPacket;

	if( USBGetDeviceState() < CONFIGURED_STATE ) {
		return(SUCCESS);
	}

	// Not sure what to do in suspend as yet
	if( USBIsDeviceSuspended()== true ) {
		return(SUCCESS);
	}
    
	while(LocalSOFCount != SOFCounter) {
		LocalSOFCount = SOFCounter;
	}

	TimeDeltaMilliseconds = LocalSOFCount - OldSOFCount;
	if(TimeDeltaMilliseconds < 0) {
		TimeDeltaMilliseconds = (32767 - OldSOFCount) + LocalSOFCount;
	}

	if(TimeDeltaMilliseconds > 5000) {
		OldSOFCount = LocalSOFCount - 5000;
	}


	if(HIDTxHandleBusy(keyboard.lastINTransmission) == false) {
		/* Clear the INPUT report buffer.  Set to all zeros. */
		memset(&inputReport, 0, sizeof(inputReport));

		if(have_key) {
			inputReport.keys[0] = key;
			inputReport.modifiers.bits.leftShift = shift;
			have_key = FALSE;
		}

        //Check to see if the new packet contents are somehow different from the most
        //recently sent packet contents.
        needToSendNewReportPacket = false;
        for(i = 0; i < sizeof(inputReport); i++)
        {
            if(*((uint8_t*)&oldInputReport + i) != *((uint8_t*)&inputReport + i))
            {
                needToSendNewReportPacket = true;
                break;
            }
        }

        //Check if the host has set the idle rate to something other than 0 (which is effectively "infinite").
        //If the idle rate is non-infinite, check to see if enough time has elapsed since
        //the last packet was sent, and it is time to send a new repeated packet or not.
        if(keyboardIdleRate != 0)
        {
            //Check if the idle rate time limit is met.  If so, need to send another HID input report packet to the host
            if(TimeDeltaMilliseconds >= keyboardIdleRate)
            {
                needToSendNewReportPacket = true;
            }
        }

        //Now send the new input report packet, if it is appropriate to do so (ex: new data is
        //present or the idle rate limit was met).
        if(needToSendNewReportPacket == true)
        {
            //Save the old input report packet contents.  We do this so we can detect changes in report packet content
            //useful for determining when something has changed and needs to get re-sent to the host when using
            //infinite idle rate setting.
            oldInputReport = inputReport;

            /* Send the 8 byte packet over USB to the host. */
            keyboard.lastINTransmission = HIDTxPacket(HID_EP, (uint8_t*)&inputReport, sizeof(inputReport));
            OldSOFCount = LocalSOFCount;    //Save the current time, so we know when to send the next packet (which depends in part on the idle rate setting)
        }

    }//if(HIDTxHandleBusy(keyboard.lastINTransmission) == false)


    /* Check if any data was sent from the PC to the keyboard device.  Report
     * descriptor allows host to send 1 byte of data.  Bits 0-4 are LED states,
     * bits 5-7 are unused pad bits.  The host can potentially send this OUT
     * report data through the HID OUT endpoint (EP1 OUT), or, alternatively,
     * the host may try to send LED state information by sending a SET_REPORT
     * control transfer on EP0.  See the USBHIDCBSetReportHandler() function. */
    if(HIDRxHandleBusy(keyboard.lastOUTTransmission) == false)
    {
        APP_KeyboardProcessOutputReport();

        keyboard.lastOUTTransmission = HIDRxPacket(HID_EP,(uint8_t*)&outputReport,sizeof(outputReport));
    }
    
    return(SUCCESS);
}

/*
 * At present not processing any messages from the Master
 */
static void APP_KeyboardProcessOutputReport(void)
{
}

static void USBHIDCBSetReportComplete(void)
{
    /* 1 byte of LED state data should now be in the CtrlTrfData buffer.  Copy
     * it to the OUTPUT report buffer for processing */
    outputReport.value = CtrlTrfData[0];

    /* Process the OUTPUT report. */
    APP_KeyboardProcessOutputReport();
}

void USBHIDCBSetReportHandler(void)
{
    /* Prepare to receive the keyboard LED state data through a SET_REPORT
     * control transfer on endpoint 0.  The host should only send 1 byte,
     * since this is all that the report descriptor allows it to send. */
    USBEP0Receive((uint8_t*)&CtrlTrfData, USB_EP0_BUFF_SIZE, USBHIDCBSetReportComplete);
}


//Callback function called by the USB stack, whenever the host sends a new SET_IDLE
//command.
void USBHIDCBSetIdleRateHandler(uint8_t reportID, uint8_t newIdleRate)
{
    //Make sure the report ID matches the keyboard input report id number.
    //If however the firmware doesn't implement/use report ID numbers,
    //then it should be == 0.
    if(reportID == 0)
    {
        keyboardIdleRate = newIdleRate;
    }
}

result_t send_key(uint8_t p_key, uint8_t p_shift)
{
	if (!have_key) {
		key      = p_key;
		shift    = p_shift;
		have_key = TRUE;
	}
	return (SUCCESS);
}

#endif // SYS_USB_KEYBOARD
