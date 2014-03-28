/**
 *
 * \file es_can/logger/net.h
 *
 * Definitions for the Network Logger
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
#ifndef NET_LOGGER_H
#define NET_LOGGER_H

extern void net_log(log_level_t level, char *msg);

/*
 * net_log_reg_as_handler
 *
 * This function takes a function pointer which is going to handle the received
 * debug messages and the minimum level that we're expecting to receive.
 */
extern result_t net_logger_local_register(void (*handler)(u8, log_level_t, char *), log_level_t level);
extern result_t net_logger_local_cancel(void);

/*
 * If another node on the network registers/unregisters as the network logger 
 * these two functions are used.
 */
extern void net_logger_foreign_register(u8 address, log_level_t level);
extern void net_logger_foreign_cancel(u8 address);

#endif // NET_LOGGER_H
