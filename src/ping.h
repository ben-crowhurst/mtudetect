/*
 * Copyright (C) 2016 by Tim Leerhoff <tleerhof@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


/**
 * @file ping.h
 * @brief Functions for ping send/receive.
 */
#ifndef __PING_H_
#define __PING_H_

#define PING_MTU_OK 0
#define PING_MTU_TOO_BIG 1
#define PING_FORBIDDEN 2
#define GENERAL_PROBLEM 3

#define RECEIVE_SIZE 65536
#define WAIT_MICROS 100000

int sendPing(int sock, const char* dstIp, unsigned int packetLen);
int receivePingAnswer(int sock, int waitTime, int* type, int* code);

#endif /*__PING_H_*/
