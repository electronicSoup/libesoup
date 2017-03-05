/**
 *
 * \file es_lib/morse.morse.c
 *
 * This file contains code for dealing Morse code
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
#define DEBUG_FILE TRUE
#define TAG "Morse"

#include "system.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/timers/timers.h"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif

struct character
{
  char ch;
  struct character *previous;
  struct character *dash;
  struct character *dot;
};

static struct character alphabet['z' -'a' + 1];

#define IDLE_STATE             0x00
#define DOT_STATE              0x01
#define DASH_STATE             0x02
#define ELEMENT_SPACE_STATE    0x03
#define CHARACTER_SPACE_STATE  0x04
#define WORD_SPACE_STATE       0x05

#define INDEX(x)  ((x - 'a') + 1)

#ifdef MORSE_TX
static struct character *tx_current_character;
static uint8_t tx_current_state;

extern void morse_tone_on(void);
extern void morse_tone_off(void);

static void tx_start_dot(void);
static void tx_start_dash(void);
static void tx_start_element_space(void);
static void tx_start_character_space(void);
static void tx_start_word_space(void);

static void timer_on(uint8_t duration);
static void exp_function(timer_t timer_id, union sigval data);

/*
 * Tx Buffer is a circular buffer
 */
static char   tx_buffer[MORSE_TX_BUFFER_SIZE];
static uint16_t tx_buffer_write_index = 0;
static uint16_t tx_buffer_read_index = 0;
static uint16_t tx_buffer_count = 0;
static es_timer     tx_timer;

#endif // MORSE_TX

void morse_init(void)
{
	uint8_t loop;

	/*
	 * Set up out alphabet
	 */
	alphabet[0].ch = 0x00;
	for(loop = 'a'; loop <= 'z'; loop++) {
		alphabet[INDEX(loop)].ch       = (char)loop;
		alphabet[INDEX(loop)].previous = NULL;
		alphabet[INDEX(loop)].dot      = NULL;
		alphabet[INDEX(loop)].dash     = NULL;
	}

	alphabet[0].dot  = &alphabet[INDEX('e')];
	alphabet[0].dash = &alphabet[INDEX('t')];

	alphabet[INDEX('a')].previous = &alphabet[INDEX('e')];
	alphabet[INDEX('a')].dot      = &alphabet[INDEX('r')];
	alphabet[INDEX('a')].dash     = &alphabet[INDEX('w')];

	alphabet[INDEX('b')].previous = &alphabet[INDEX('d')];

	alphabet[INDEX('c')].previous = &alphabet[INDEX('k')];

	alphabet[INDEX('d')].previous = &alphabet[INDEX('n')];
	alphabet[INDEX('d')].dot      = &alphabet[INDEX('b')];
	alphabet[INDEX('d')].dash     = &alphabet[INDEX('x')];

	alphabet[INDEX('e')].previous = &alphabet[0];
	alphabet[INDEX('e')].dot      = &alphabet[INDEX('i')];
	alphabet[INDEX('e')].dash     = &alphabet[INDEX('a')];

	alphabet[INDEX('f')].previous = &alphabet[INDEX('u')];

	alphabet[INDEX('g')].previous = &alphabet[INDEX('m')];
	alphabet[INDEX('g')].dot      = &alphabet[INDEX('q')];
	alphabet[INDEX('g')].dash     = &alphabet[INDEX('z')];

	alphabet[INDEX('h')].previous = &alphabet[INDEX('s')];

	alphabet[INDEX('i')].previous = &alphabet[INDEX('e')];
	alphabet[INDEX('i')].dot      = &alphabet[INDEX('s')];
	alphabet[INDEX('i')].dash     = &alphabet[INDEX('u')];

	alphabet[INDEX('j')].previous = &alphabet[INDEX('w')];

	alphabet[INDEX('k')].previous = &alphabet[INDEX('n')];
	alphabet[INDEX('k')].dot      = &alphabet[INDEX('c')];
	alphabet[INDEX('k')].dash     = &alphabet[INDEX('y')];

	alphabet[INDEX('l')].previous = &alphabet[INDEX('r')];

	alphabet[INDEX('m')].previous = &alphabet[INDEX('t')];
	alphabet[INDEX('m')].dot      = &alphabet[INDEX('g')];
	alphabet[INDEX('m')].dash     = &alphabet[INDEX('o')];

	alphabet[INDEX('n')].previous = &alphabet[INDEX('t')];
	alphabet[INDEX('n')].dot      = &alphabet[INDEX('d')];
	alphabet[INDEX('n')].dash     = &alphabet[INDEX('k')];

	alphabet[INDEX('o')].previous = &alphabet[INDEX('m')];

	alphabet[INDEX('p')].previous = &alphabet[INDEX('w')];

	alphabet[INDEX('q')].previous = &alphabet[INDEX('g')];

	alphabet[INDEX('r')].previous = &alphabet[INDEX('a')];
	alphabet[INDEX('r')].dot      = &alphabet[INDEX('l')];
	alphabet[INDEX('r')].dash     = NULL;

	alphabet[INDEX('s')].previous = &alphabet[INDEX('i')];
	alphabet[INDEX('s')].dot      = &alphabet[INDEX('h')];
	alphabet[INDEX('s')].dash     = &alphabet[INDEX('v')];

	alphabet[INDEX('t')].previous = &alphabet[0];
	alphabet[INDEX('t')].dot      = &alphabet[INDEX('n')];
	alphabet[INDEX('t')].dash     = &alphabet[INDEX('m')];

	alphabet[INDEX('u')].previous = &alphabet[INDEX('i')];
	alphabet[INDEX('u')].dot      = &alphabet[INDEX('f')];
	alphabet[INDEX('u')].dash     = NULL;

	alphabet[INDEX('v')].previous = &alphabet[INDEX('s')];

	alphabet[INDEX('w')].previous = &alphabet[INDEX('a')];
	alphabet[INDEX('w')].dot      = &alphabet[INDEX('p')];
	alphabet[INDEX('w')].dash     = &alphabet[INDEX('j')];

	alphabet[INDEX('x')].previous = &alphabet[INDEX('d')];

	alphabet[INDEX('y')].previous = &alphabet[INDEX('k')];

	alphabet[INDEX('z')].previous = &alphabet[INDEX('g')];
}

#ifdef MORSE_TX
void morse_tx_init(void (*on)(void), void (*off)(void))
{
	morse_init();

	tx_buffer_write_index = 0;
	tx_buffer_read_index = 0;
	tx_buffer_count = 0;
}
#endif // MORSE_TX

#ifdef MORSE_TX
static void morse_tx_char(uint8_t ch)
{
	struct character *previous_character;

	if(ch == ' ') {
		tx_start_word_space();
	}
	tx_current_character = &alphabet[INDEX(ch)];

	previous_character = tx_current_character->previous;

	if(previous_character) {
		if (previous_character->dot == tx_current_character) {
			tx_start_dot();
		} else if (previous_character->dash == tx_current_character) {
			tx_start_dash();
		}

		tx_current_character = previous_character;

	} else {
		/*
		 * Character finished so start the inter character pause
		 */
		tx_start_character_space();
	}
}
#endif // MORSE_TX

#ifdef MORSE_TX
void morse_tx(char *msg)
{
	char *ptr;

	ptr = msg;

	if (tx_buffer_count == 0) {
		morse_tx_char(*ptr++);
	}
	
	while((*ptr) && (tx_buffer_count < MORSE_TX_BUFFER_SIZE)) {
		tx_buffer[tx_buffer_write_index] = *ptr++;
		tx_buffer_write_index = (tx_buffer_write_index + 1) % MORSE_TX_BUFFER_SIZE;
		tx_buffer_count++;
	}

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Leaving morse_tx() buffer count is %d\n\r", tx_buffer_count);
#endif
}
#endif  // if MORSE_TX

#ifdef MORSE_TX
void tx_start_dot(void)
{
	putchar('.');
	morse_tone_on();
	timer_on(1);
	tx_current_state = DOT_STATE;
}
#endif

#ifdef MORSE_RX
void start_dot(void)
{
	tone_on();
	timer_on(1);
	current_state = DOT_STATE;

	if(current_letter_valid) {
		if (current_letter->dot != NULL) {
			current_letter = current_letter->dot;
			current_letter_valid = 0x01;
		} else {
			current_letter_valid = 0x00;
		}
	}
}
#endif

#ifdef MORSE_TX
void tx_start_dash(void)
{
	putchar('_');
	morse_tone_on();
	timer_on(3);
	tx_current_state = DASH_STATE;
}
#endif

#ifdef MORSE_RX
void start_dash(void)
{
	putchar('_');
	tone_on();
	timer_on(3);
	current_state = DASH_STATE;

	if(current_letter_valid) {
		if (current_letter->dash != NULL) {
			current_letter = current_letter->dash;
			current_letter_valid = 0x01;
		} else {
			current_letter_valid = 0x00;
		}
	}
}
#endif

#ifdef MORSE_TX
void tx_start_element_space(void)
{
	putchar('/');
	morse_tone_off();
	timer_on(1);
	tx_current_state = ELEMENT_SPACE_STATE;
}
#endif

#ifdef MORSE_TX
void tx_start_character_space(void)
{
	putchar('>');
	morse_tone_off();
	timer_on(2);
	tx_current_state = CHARACTER_SPACE_STATE;
#ifdef MORSE_RX
	if(current_letter_valid) {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "LETTER VALID -%c-\n\r", current_letter->ch);
#endif
	}
	current_letter_valid = 0x01;
	current_letter = &alphabet[0];
#endif // MORSE_RX
}
#endif // MORSE_TX

#ifdef MORSE_TX
void tx_start_word_space(void)
{
	putchar('=');
	morse_tone_off();
	timer_on(4);
	tx_current_state = WORD_SPACE_STATE;
}
#endif

#ifdef MORSE_TX
void timer_on(uint8_t duration)
{
	result_t     rc;
	union sigval data;

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "timer_on(%d)\n\r", MILLI_SECONDS_TO_TICKS(DOT_TIME * duration));
#endif
	data.sival_int = 0x00;

	rc = timer_start(MILLI_SECONDS_TO_TICKS(DOT_TIME * duration), exp_function, data, &tx_timer);
}
#endif  // MORSE_TX

#ifdef MORSE_TX
void exp_function(timer_t timer_id, union sigval data)
{
	putchar('*');

	TIMER_INIT(tx_timer);

	if(tx_current_state == DOT_STATE) {
		tx_start_element_space();
	} else if (tx_current_state == DASH_STATE) {
		tx_start_element_space();
	} else if (tx_current_state == ELEMENT_SPACE_STATE) {
		morse_tx_char(tx_current_character->ch);
	} else if (   (tx_current_state == CHARACTER_SPACE_STATE)
	           || (tx_current_state == WORD_SPACE_STATE)) {
		// TODO Would be next character
		if (tx_buffer_count > 0) {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Tx '%c'\n\r", tx_buffer[tx_buffer_read_index]);
#endif
			morse_tx_char(tx_buffer[tx_buffer_read_index]);
			tx_buffer_read_index = (tx_buffer_read_index + 1) % MORSE_TX_BUFFER_SIZE;
			tx_buffer_count--;
		} else {
			tx_current_state = IDLE_STATE;
		}
	}
}
#endif // MORSE_TX
