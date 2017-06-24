/**
 *
 * \file libesoup/app/app.h
 *
 * Application header which must be used in main.c of Application
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "libesoup/core.h"

/*
 * The following are definitions for setting up the identification strings
 * in the Flash which the CAN Node OS expects to be at specific locations.
 * There are four strings describing the installed Application
 * An example useage would be:

DEF_APP_AUTHOR_40("me@mail.com")
DEF_APP_DESCRIPTION_50("Serial_Firmware")
DEF_APP_VERSION_10("v1.0")
DEF_APP_URL_50("www.test.com")

 */
#define APP_STRINGS_BASE           0x18000
#define APP_AUTHOR_40_ADDR         APP_STRINGS_BASE
#define APP_DESCRIPTION_50_ADDR    APP_AUTHOR_40_ADDR + 40
#define APP_VERSION_10_ADDR        APP_DESCRIPTION_50_ADDR + 50
#define APP_URL_50_ADDR            APP_VERSION_10_ADDR + 10

#define DEF_APP_AUTHOR_40(x)      __prog__ char app_author[40]       __attribute__ ((space(prog),address(APP_AUTHOR_40_ADDR))) = x;
#define DEF_APP_DESCRIPTION_50(x) __prog__ char app_description[50]  __attribute__ ((space(prog),address(APP_DESCRIPTION_50_ADDR))) = x;
#define DEF_APP_VERSION_10(x)     __prog__ char app_version[10]      __attribute__ ((space(prog),address(APP_VERSION_10_ADDR))) = x;
#define DEF_APP_URL_50(x)         __prog__ char app_uri[50]          __attribute__ ((space(prog),address(APP_URL_50_ADDR))) = x;
