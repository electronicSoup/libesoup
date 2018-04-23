/**
 *
 * @file libesoup/boards/gauge/board.c
 *
 * @author John Whitmore
 *
 * Board initialisation code for the PIC18 based gauge board.
 *
 * Copyright 2018 electronicSoup Limited
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

#if defined(__18F4585)

#include "libesoup/errno.h"

result_t board_init(void)
{
	return(0);
}
#endif //  defined(__18F4585)
