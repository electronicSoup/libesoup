/**
 * @file libesoup/boards/stereo_guitar/stereo_guitar_dsPIC33EP128GS702.c
 *
 * @author John Whitmore
 *
 * @brief Board specific code for the Stereo Guitar project.
 *
 * The file contains the implementation of the board_init() function which
 * will be called as part of the library initialisation via libesoup_init()
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
#if defined(__dsPIC33EP128GS702__)// Platform specific code

#include "libesoup_config.h"

#include "libesoup/errno.h"

result_t board_init(void)
{
	return(SUCCESS);
}
#endif // uC
