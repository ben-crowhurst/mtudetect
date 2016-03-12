/**
 * @file ping.h
 *
 * @brief Functions and return values for MTU detection.
 * 
 * @author Tim Leerhoff <tleerhof@web.de>, (C) 2016
 */
#ifndef __MTUDETECT_H_
#define __MTUDETECT_H_

#define GENERAL_PROBLEM -1
#define PING_MTU_OK 0
#define PING_MTU_TOO_BIG 1

int checkMTU(const char* dstIp, unsigned int packetLen);
int searchMTU(const char* dstIp, unsigned int maxMTU);

#endif /*__MTUDETECT_H_*/
