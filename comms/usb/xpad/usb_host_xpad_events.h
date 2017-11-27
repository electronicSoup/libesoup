/*********************************************************************
 *
 * \file libesoup/usb/xpad/usb_host_xpad_events.h
 *
 * File contains the xpad Driver defined USB Events which are sent 
 * to the Application's event handler for changes on the xpad 
 * controller.
 *
 * Copyright 2017 electronicSoup Limited
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
 *********************************************************************
 *
 * Currently only process the xpad's D-Pad Down button as proof of
 * concept. 
 */
#define USB_XPAD_D_DOWN_PRESS           EVENT_USER_BASE + 0
#define USB_XPAD_D_DOWN_RELEASE         EVENT_USER_BASE + 1
