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
 * @file mtudetect.h
 * @brief Functions and return values for MTU detection.
 */
#ifndef __MTUDETECT_H_
#define __MTUDETECT_H_

#define PING_MTU_OK 0
#define PING_MTU_TOO_BIG 1
#define PING_FORBIDDEN 2
#define GENERAL_PROBLEM 3

#define WAIT_MICROS 100000

int checkMTU(const char* dstIp, unsigned int packetLen);
int searchMTU(const char* dstIp, unsigned int maxMTU);
const char* getReturnValueText(int returnValue);

#endif /*__MTUDETECT_H_*/
