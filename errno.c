/**
 * @file libesoup/errno.h
 *
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
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
#include "libesoup_config.h"

char *error_text(result_t rc) {
	switch (-rc) {
	case ERR_GENERAL_ERROR:
		return("ERR_GENERAL_ERROR");
	case ERR_BAD_INPUT_PARAMETER:
		return("ERR_BAD_INPUT_PARAMETER");
	case ERR_NOT_CODED:
		return("ERR_NOT_CODED");
	case ERR_BUFFER_OVERFLOW:
		return("ERR_BUFFER_OVERFLOW");
	case ERR_RANGE_ERROR:
		return("ERR_RANGE_ERROR");
	case ERR_TIMER_ACTIVE:
		return("ERR_TIMER_ACTIVE");
	case ERR_NO_RESOURCES:
		return("ERR_NO_RESOURCES");
	case ERR_ADDRESS_RANGE:
		return("ERR_ADDRESS_RANGE");
	case ERR_NOT_READY:
		return("ERR_NOT_READY");
	case ERR_CAN_ERROR:
		return("ERR_CAN_ERROR");
	case ERR_CAN_NOT_CONNECTED:
		return("ERR_CAN_NOT_CONNECTED");
	case ERR_CAN_BITRATE_LOW:
		return("ERR_CAN_BITRATE_LOW");
	case ERR_CAN_BITRATE_HIGH:
		return("ERR_CAN_BITRATE_HIGH");
	case ERR_CAN_INVALID_BAUDRATE:
		return("ERR_CAN_INVALID_BAUDRATE");
	case ERR_CAN_NO_FREE_BUFFER:
		return("ERR_CAN_NO_FREE_BUFFER");
	case ERR_UNINITIALISED:
		return("ERR_UNINITIALISED");
	case ERR_BUSY:
		return("ERR_BUSY");
	case ERR_NOTHING_TO_DO:
		return("ERR_NOTHING_TO_DO");
	case ERR_NO_RESPONSE:
		return("ERR_NO_RESPONSE");
	case ERR_NOT_MASTER:
		return("ERR_NOT_MASTER");
	case ERR_NOT_SLAVE:
		return("ERR_NOT_SLAVE");
	case ERR_BAD_STATE:
		return("ERR_BAD_STATE");
	case ERR_IM_A_TEAPOT:
		return("ERR_IM_A_TEAPOT");
	default:
		return("Unknown Error");
	}
	return("Unknown Error");
}
