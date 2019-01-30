/**
 * @file libesoup/examples/main_barebones.c
 *
 * @author John Whitmore
 * 
 * @brief An example main.c file for the bare bones minimum required to use the
 * libesoup library of code.
 * 
 * The code is used in the example MPLAB-X project:
 * libesoup/examples/projects/microchip/BareBones.X
 *
 * Copyright 2018-2019 electronicSoup Limited
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
 *******************************************************************************
 *
 */
#include "libesoup_config.h"

int main()
{
	result_t  rc;
	
	rc = libesoup_init();
	if(rc < 0) {
		// ERROR
	}
	
	while(1) {
		libesoup_tasks();
	}
}
