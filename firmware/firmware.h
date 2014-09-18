/**
 *
 * \file es_can/firmware/firmware.h
 *
 * Definitions for creating firmware for the CinnamonBun Bootloader
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef FIRMWARE_H
#define FIRMWARE_H

/*******************************************************************************
 *
 *                               WARNING
 *
 * The bootloader enables the watchdog timer so ensure that your firmware 
 * pulses the watchdog in a timely fashion!!!
 *
 *  asm("CLRWDT");
 *
 *******************************************************************************
/*
 * The configuration settings are needed to program into the last page of Flash
 *
 * This is might seem a little strange as the Bootloader has set up the
 * config settings but if we re-write the last page of the Flash we'll overwrite
 * thos configuation settings and potentially mess up the device.
 * One solution to this problem is when we erase the last page to read out
 * the three config registers current settings. Then perform the earse and then
 * when we program the last row to write back the config settings to the last
 * three locations in Flash.
 * All of the above requires code to execute the algorythm and the following
 * lines are a lot simpler then maintaining that code, at the moment. Perhaps
 * it's a todo item to make it more intelligent but this'll do at the moment.
 */
_CONFIG1(JTAGEN_OFF & FWDTEN_ON & FWPSA_PR32 & WDTPS_PS1024 & WINDIS_OFF & ICS_PGx2)   // JTAG off, watchdog timer on
_CONFIG2(FNOSC_FRCPLL & POSCMOD_NONE & OSCIOFNC_ON & PLL_96MHZ_ON & PLLDIV_NODIV & DISUVREG_OFF)  // CLOCK 16000000

/*
 * The following are definitions for setting up the identification strings
 * in the Flash which the Bootloader expects to be at specific locations.
 * There are four strings describing the Firmware installed in the Device.
 * An example useage would be:

DEF_FIRMWARE_AUTHOR_40("me@mail.com")
DEF_FIRMWARE_DESCRIPTION_50("Serial_Firmware")
DEF_FIRMWARE_VERSION_10("v1.0")
DEF_FIRMWARE_URL_50("www.test.com")

 */
#define FIRMWARE_STRINGS_BASE          0x8800
#define FIRMWARE_AUTHOR_40_ADDR        FIRMWARE_STRINGS_BASE
#define FIRMWARE_DESCRIPTION_50_ADDR   FIRMWARE_AUTHOR_40_ADDR + 40
#define FIRMWARE_VERSION_10_ADDR       FIRMWARE_DESCRIPTION_50_ADDR + 50
#define FIRMWARE_URL_50_ADDR           FIRMWARE_VERSION_10_ADDR + 10

#define DEF_FIRMWARE_AUTHOR_40(x)      __prog__ char firmware_author[40]       __attribute__ ((space(prog),address(FIRMWARE_AUTHOR_40_ADDR))) = x;
#define DEF_FIRMWARE_DESCRIPTION_50(x) __prog__ char firmware_description[50]  __attribute__ ((space(prog),address(FIRMWARE_DESCRIPTION_50_ADDR))) = x;
#define DEF_FIRMWARE_VERSION_10(x)     __prog__ char firmware_version[10]      __attribute__ ((space(prog),address(FIRMWARE_VERSION_10_ADDR))) = x;
#define DEF_FIRMWARE_URL_50(x)         __prog__ char firmware_uri[50]          __attribute__ ((space(prog),address(FIRMWARE_URL_50_ADDR))) = x;


#endif // FIRMWARE_H
