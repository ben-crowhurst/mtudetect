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
 * @file mtudetect.c
 * @brief Functions and return values for MTU detection.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>		// For strlen,...
#include <unistd.h>		// For close,...
#include <errno.h>

#include <sys/socket.h>		// socket()

#include <arpa/inet.h>		// Funktionen wie inet_addr()
#include <netinet/in.h>		// IP Protokolle, sockaddr_in Struktur und Funktionen wie htons()
#include <netinet/ip.h>		// IP Header Struktur
#include <netinet/ip_icmp.h>	// ICMP Header Struktur
#include <netinet/tcp.h>	// TCP Header Struktur

#include "ping.h"


/**
 * Create checksum for Ping.
 * @param checkSumData The data to generate the checksum from.
 * @param byteLen The length of the databuffer.
 * @return The checksum.
 */
static unsigned short calcCheckSum( const char* checkSumData, unsigned int byteLen )
{
  unsigned int checkSum = 0;
  unsigned int wordLen = byteLen / 2;
  unsigned int hasPadding = byteLen % 2;

  int i;
  for(i=0;i<wordLen + hasPadding;i++)
  {
    unsigned int value;
    if(i == wordLen)
    {
      // Letztes Wort ?
      if(hasPadding)
      {
        unsigned short sFirstByte = (unsigned short)(*checkSumData++);
        unsigned short sSecondByte = (unsigned short)(0);
        value = sFirstByte | sSecondByte;
      }
      else
      {
        unsigned short sFirstByte = (unsigned short)(*checkSumData++);
        unsigned short sSecondByte = (unsigned short)(*checkSumData++);
        value = (sFirstByte << 8) | sSecondByte;
      }
    }
    else
    {
      unsigned short sFirstByte = (unsigned short)(*checkSumData++);
      unsigned short sSecondByte = (unsigned short)(*checkSumData++);
      value = (sFirstByte << 8) | sSecondByte;
    }
    checkSum += value;
  }
  return ~(unsigned short)( ((checkSum >> 16) + checkSum) & 0x0000FFFF );
}



/**
 * Fills the given buffer with testdata.
 * It generates the aplhabet in uppercase and fills repeated the buffer.
 * @param dst The buffer to fill.
 * @param size The size of the buffer in bytes.
 */
static void fillInTestData( unsigned char* dst, unsigned int size )
{
  unsigned int i=0;
  for(i=0;i<size;i++)
  {
    *dst++ = (i%26 + 65);
  }
}



/**
 * Sends a ping with the defined size to the given server-ip
 * @param dstIp The ip address of the server.
 * @param the size of the packet to send.
 * @return 0, bei Erfolg.
 */
int sendPing( int sock, const char* dstIp, unsigned int packetLen )
{
  //printf("mtu: %d\n", packetLen);
  int errCode = 0;
  unsigned int hdrSize = sizeof(struct iphdr) + sizeof(struct icmphdr);
  unsigned int payloadLen = packetLen - hdrSize;
  if(payloadLen <= 0)
    return -1;
  unsigned char* packet = (unsigned char*)malloc(packetLen);
  memset( packet, 0, packetLen-1);

  // Calculate header addresses
  struct iphdr* ipHeader = (struct iphdr*)(packet);
  struct icmphdr* icmpHeader = (struct icmphdr*)(packet + sizeof(struct iphdr));
  unsigned char* payload = (packet + hdrSize);

  ipHeader->ihl = 5;
  ipHeader->version = 4;
  ipHeader->tos = 0;
  ipHeader->tot_len = htons(packetLen);
  ipHeader->id = htons(0xFFFF);
  ipHeader->frag_off = htons(0x4000);
  ipHeader->ttl = 255;
  ipHeader->protocol = 1; // ICMP
  ipHeader->check = 0;
  ipHeader->saddr = inet_addr("0.0.0.0");
  ipHeader->daddr = inet_addr(dstIp);

  icmpHeader->type = ICMP_ECHO;
  icmpHeader->code = 0;
  icmpHeader->checksum = 0;
  icmpHeader->un.echo.id = htons(1);
  icmpHeader->un.echo.sequence = htons(1);

  fillInTestData(payload, payloadLen);

  unsigned short sCheckSum = calcCheckSum( (const char*)icmpHeader, payloadLen + sizeof(struct icmphdr) );
  icmpHeader->checksum = htons(sCheckSum);

  struct sockaddr_in dst;
  memset( &dst, 0, sizeof(struct sockaddr_in));
  dst.sin_addr.s_addr = ipHeader->daddr;
  dst.sin_family = AF_INET;

  ssize_t lengthSent = sendto( sock, packet, packetLen, 0, (struct sockaddr*)&dst, sizeof(dst) );
  if( packetLen != lengthSent )
  {
    errCode = errno;
    perror("Error.send: sendTo");
    free(packet);
    return errCode;
  }

  free(packet);
  return errCode;
}





/**
 * Processes the icmp-header of the receiving packet to extract the error codes.
 * @param packet The packet to process.
 * @param type The icmp-type-field as output-parameter.
 * @param code The icmp-code-field as output-parameter.
 */
static void processPacket(unsigned char* packet, int* type, int* code)
{
  struct icmphdr* icmpHeader = (struct icmphdr*)(packet + sizeof(struct iphdr));
  *type = icmpHeader->type;
  *code = icmpHeader->code;
  fprintf(stdout, "type:%d, code:%d\n", icmpHeader->type, icmpHeader->code);
}



/**
 * Waits for the icmp-answer of the peer to detect fragmentation-problems.
 * @param type The icmp-type-field as output-parameter.
 * @param code The icmp-code-field as output-parameter.
 * @return If not zero there was an error. 
 */
int receivePingAnswer(int sock, int waitTime, int* type, int* code)
{
  int errCode = 0;
  unsigned char *packet = (unsigned char *)malloc(RECEIVE_SIZE);

  struct sockaddr saddr;
  socklen_t saddr_size = 0;

  ssize_t lengthReceived = recvfrom(sock, packet, RECEIVE_SIZE, 0 ,&saddr ,&saddr_size);
  if(lengthReceived <= 0)
  {
    errCode = errno;
    perror("Error.receive: recvfrom");
    fprintf(stderr, "errCode: %d\n", errCode);
    free(packet);
    return errCode;
  }
  else
  {
    processPacket(packet, type, code);
  }

  free(packet);
  return 0;
}
