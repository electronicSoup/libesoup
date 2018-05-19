/**
 * @file libesoup/processors/crc/crc.h
 *
 * @author John Whitmore
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

#ifdef SYS_CRC

/**
 * polynomial_length is the length - 1!!!!
 * data_word_width is the width -1 !!!
 */
#ifdef SYS_DEBUG_BUILD
extern result_t crc_test(void);
#endif
extern result_t crc_reserve(uint32_t polynomial, uint8_t polynomial_length, uint8_t data_word_width, boolean little_endian);
extern result_t crc_sum_byte(uint8_t data);
extern result_t crc_sum_result(uint32_t *result);
extern result_t crc_sum_reset(void);
extern result_t crc_release(void);

#endif  // CRC
