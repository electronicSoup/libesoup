/**
 *
 * \file libesoup/morse.morse.c
 *
 * This file contains code for dealing Morse code
 *
 * Copyright 2017 - 2018 electronicSoup Limited
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

#ifdef SYS_MORSE

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "Morse";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#include "libesoup/timers/timers.h"

struct character
{
	char ch;
	struct character *previous;
	struct character *dash;
	struct character *dot;
};

// Todo add special characters
static struct character alphabet['z' -'a' + 1];  // Add one for the root node of the tree

#define IDLE_STATE             0x00
#define DOT_STATE              0x01
#define DASH_STATE             0x02
#define ELEMENT_SPACE_STATE    0x03
#define CHARACTER_SPACE_STATE  0x04
#define WORD_SPACE_STATE       0x05

#define INDEX(x)  ((x - 'a') + 1)                // Add one for the root node of the tree

#ifdef MORSE_TX
static struct character *tx_current_character;
static uint8_t tx_current_state;

static void (*morse_tone_on)(void)  = (void (*)(void))NULL;
static void (*morse_tone_off)(void) = (void (*)(void))NULL;

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
static char     tx_char_buffer[MORSE_TX_BUFFER_SIZE];
static uint16_t tx_char_buffer_write_index = 0;
static uint16_t tx_char_buffer_read_index = 0;
static uint16_t tx_char_buffer_count = 0;
static es_timer tx_timer;

/*
 * Buffer for the elemets which have to be transmitted for the current character
 */
static uint8_t  tx_elements;
static uint8_t  tx_num_elements;
#endif // MORSE_TX

#if defined (MORSE_TX) || defined (MORSE_RX)
static void morse_init(void)
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
#endif  // defined (MORSE_TX) || defined (MORSE_RX)

#if defined (MORSE_RX)
void morse_rx_init(void (*process_string)(char *))
{
}

void morse_rx_on(void)
{
}

void morse_rx_off(void)
{
}
#endif

#ifdef MORSE_TX
void morse_tx_init(void (*on)(void), void (*off)(void))
{
	if(!no || !off) {
		/*
		 * Todo - Error Condition
		 * Need two valid functions to enable transmission
		 */
		return;
	}
	
	morse_tone_on  = on;
	morse_tone_off = off;

	morse_init();

	tx_char_buffer_write_index = 0;
	tx_char_buffer_read_index = 0;
	tx_char_buffer_count = 0;
}
#endif // MORSE_TX

#ifdef MORSE_TX
static void tx_char(uint8_t ch)
{
	struct character *previous_character;

	/*
	 * Make sure that the mini buffer for character elements is clear to use
	 */
	if(tx_element_pos != 0) {
		/*
		 * Todo - Handle an error condition.
		 */
		return;
	}
	
	if(ch == ' ') {
		tx_start_word_space();
	}
	
	tx_current_character = &alphabet[INDEX(ch)];

	previous_character = tx_current_character->previous;

	/*
	 * We have at lest one element so initialise the position we're going to 
	 * push the next element to
	 */
	tx_elements = 0x00;
	tx_element_pos = 0x01;
	
	while(previous_character != alphabet[0]) {
		if (previous_character->dot == tx_current_character) {
			/*
			 * Element is alreay cleared by initialisation just shuffle
			 * the position.
			 */
			tx_element_pos = tx_element_pos << 1;
		} else if (previous_character->dash == tx_current_character) {
			/*
			 * Or the bit high at the current position for a dash
			 */
			tx_elements = tx_elements | tx_element_pos;
			tx_element_pos = tx_element_pos << 1;
		}

		/*
		 * Walk back up the binary tree to the preious node.
		 */
		previous_character = previous_character->previous;
	}

	/*
	 * Shuffle the position back one position as we'll have gone over by one
	 */
	tx_element_pos = tx_element_pos >> 1;
	
	/*
	 * Elements for the current character are now loaded into the element byte
	 * so start transmitting them.
	 */
	if(tx_element_pos != 0x00) {
		tx_element();
	}
}
#endif // MORSE_TX

#ifdef MORSE_TX
static void tx_element(void)
{
	if (tx_elements & tx_element_pos) {
		tx_start_dash();
	} else {
		tx_start_dot();
	}
	/*
	 * Shuffle the position
	 */
	tx_element_pos = tx_element_pos >> 1;
}
#endif

#ifdef MORSE_TX
void morse_tx(char *msg)
{
	char *ptr;

	ptr = msg;

	if (tx_buffer_count == 0) {
		tx_char(*ptr++);
	}
	
	while((*ptr) && (tx_buffer_count < MORSE_TX_BUFFER_SIZE)) {
		tx_buffer[tx_buffer_write_index] = *ptr++;
		tx_buffer_write_index = (tx_buffer_write_index + 1) % MORSE_TX_BUFFER_SIZE;
		tx_buffer_count++;
	}

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Leaving morse_tx() buffer count is %d\n\r", tx_buffer_count);
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
void rx_start_dot(void)
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
void rx_start_dash(void)
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
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("LETTER VALID -%c-\n\r", current_letter->ch);
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

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("timer_on(%d)\n\r", MILLI_SECONDS_TO_TICKS(DOT_TIME * duration));
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
		if(tx_element_pos != 0x00) {
			tx_element();
		} else {
			tx_start_character_space();
		}
	} else if (   (tx_current_state == CHARACTER_SPACE_STATE)
	           || (tx_current_state == WORD_SPACE_STATE)) {

		// TODO Would be next character
		if (tx_buffer_count > 0) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Tx '%c'\n\r", tx_buffer[tx_buffer_read_index]);
#endif
			tx_char(tx_buffer[tx_buffer_read_index]);
			tx_buffer_read_index = (tx_buffer_read_index + 1) % MORSE_TX_BUFFER_SIZE;
			tx_buffer_count--;
		} else {
			tx_current_state = IDLE_STATE;
		}
	}
}
#endif // MORSE_TX

#endif // SYS_MORSE
