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

#include "mtudetect.h"


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
        unsigned short sSecondByte = (unsigned short)(0) << 8;
        value = sFirstByte | sSecondByte;
      }
      else
      {
        unsigned short sFirstByte = (unsigned short)(*checkSumData++);
        unsigned short sSecondByte = (unsigned short)(*checkSumData++) << 8;
        value = sFirstByte | sSecondByte;
      }
    }
    else
    {
      unsigned short sFirstByte = (unsigned short)(*checkSumData++);
      unsigned short sSecondByte = (unsigned short)(*checkSumData++) << 8;
      value = sFirstByte | sSecondByte;
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
static int ping( const char* dstIp, unsigned int packetLen )
{
  //printf("mtu: %d\n", packetLen);
  int errCode = 0;
  unsigned int hdrSize = sizeof(struct iphdr) + sizeof(struct icmphdr);
  unsigned int payloadLen = packetLen - hdrSize;
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
  icmpHeader->checksum = sCheckSum;


  int sock = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
  if( sock == -1 )
  {
    errCode = errno;
    free(packet);
    return errCode;
  }

  int iOne = 1;
  if( setsockopt( sock, IPPROTO_IP, IP_HDRINCL, &iOne, sizeof(iOne) ) != 0 )
  {
    errCode = errno;
    free(packet);
    close(sock);
    return errCode;
  }

  struct sockaddr_in dst;
  memset( &dst, 0, sizeof(struct sockaddr_in)-1);
  dst.sin_addr.s_addr = ipHeader->daddr;
  dst.sin_family = AF_INET;

  ssize_t lengthSent = sendto( sock, packet, packetLen, 0, (struct sockaddr*)&dst, sizeof(dst) );
  if( packetLen != lengthSent )
  {
    errCode = errno;
    free(packet);
    close(sock);
    return errCode;
  }

  free(packet);
  close(sock);
  return errCode;
}




#define RECEIVE_SIZE 65536
#define WAIT_MICROS 100000


/**
 * Processes the icmp-header of the receiving packet to extract the error codes.
 * @param packet The packet to process.
 * @param lengthReceived The length of the received packet.
 * @param type The icmp-type-field as output-parameter.
 * @param code The icmp-code-field as output-parameter.
 */
static void processPacket(unsigned char* packet, ssize_t lengthReceived, int* type, int* code)
{
  struct icmphdr* icmpHeader = (struct icmphdr*)(packet + sizeof(struct iphdr));
  *type = icmpHeader->type;
  *code = icmpHeader->code;
//  printf("type:%d, code:%d, packetlength:%d\n", icmpHeader->type, icmpHeader->code, (int)lengthReceived);
}



/**
 * Waits for the icmp-answer of the peer to detect fragmentation-problems.
 * @param type The icmp-type-field as output-parameter.
 * @param code The icmp-code-field as output-parameter.
 * @return If not zero there was an error. 
 */
int waitForPingAnswer(int* type, int* code)
{
  int errCode = 0;
  unsigned char *packet = (unsigned char *)malloc(RECEIVE_SIZE);
  int sock = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
  if( sock == -1 )
  {
    errCode = errno;
    free(packet);
    close(sock);
    return errCode;
  }

  int iOne = 1;
  if( setsockopt( sock, IPPROTO_IP, IP_HDRINCL, &iOne, sizeof(iOne) ) < 0 )
  {
    errCode = errno;
    free(packet);
    close(sock);
    return errCode;
  }

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = WAIT_MICROS;
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
  {
    errCode = errno;
    free(packet);
    close(sock);
    return errCode;
  }

  struct sockaddr saddr;
  socklen_t saddr_size = 0;

  ssize_t lengthReceived = recvfrom(sock, packet, RECEIVE_SIZE, 0 ,&saddr ,&saddr_size);
  if(lengthReceived <= 0)
  {
    errCode = errno;
    free(packet);
    close(sock);
    return errCode;
  }
  else
  {
    processPacket(packet, lengthReceived, type, code);
  }

  free(packet);
  close(sock);
  return errCode;
}


/**
 * This function sends a ping to the server-ip with the specified packetsize.
 * The result is checked agaist some conditions indicating that the packetsize is
 * too big or ok.
 * @param dstIp The ip-address of the peer
 * @param packetLen The packet size to send in bytes.
 * @return 
 *   PING_MTU_TOO_BIG: If MTU is too big
 *   PING_MTU_OK: If Echo reply is received
 *   GENERAL_PROBLEM: If some problem occurs in the socket, or the peer does not respond
 */
int checkMTU( const char* dstIp, unsigned int packetLen )
{
  //printf("Checking MTU: %d\n", packetLen);
  int result = ping(dstIp, packetLen);
  if(result == 90)
  {
    // Message too long
    // The network-subsystem caches the result of the mtu-check.
    return PING_MTU_TOO_BIG;
  }

  int type = 0, code = 0;
  result = waitForPingAnswer(&type, &code);
  if(result != 0)
  {
    // Some problem occurred in the receive-code...
    return GENERAL_PROBLEM;
  }
  else
  if(type == 0 && code == 0)
  {
    // ECHO-REPLY is OK
    return PING_MTU_OK;
  }
  else
  if(type == 3 && code == 4)
  {
    // Fragmentation needed
    return PING_MTU_TOO_BIG;
  }
  else
  {
    return GENERAL_PROBLEM;
  }
}



/**
 * Searches the MTU by decreasing packet size and sending pings to a host.
 * @param dstIp The ip of the host.
 * @param maxMTU The MTU to decrease from.
 * @return The MTU or -1 when an error occurs.
 */
int searchMTU(const char* dstIp, unsigned int maxMTU)
{
  int mtu;
  for(mtu=maxMTU;mtu>0;mtu--)
  {
    int mtuResult = checkMTU(dstIp, mtu);
    if(mtuResult == PING_MTU_OK)
      return mtu;
  }
  return GENERAL_PROBLEM;
}





/*
int interval(const char* dstIp, unsigned int minMTU, unsigned int maxMTU)
{
  int lowerMTUResult = checkMTU(dstIp, minMTU);
  int upperMTUResult = checkMTU(dstIp, maxMTU);

  if(lowerMTUResult == GENERAL_PROBLEM)
    return -2;
  if(upperMTUResult == GENERAL_PROBLEM)
    return -2;

  if(lowerMTUResult == PING_MTU_TOO_BIG &&
     upperMTUResult == PING_MTU_TOO_BIG)
    return -1;

  if(lowerMTUResult == PING_MTU_OK &&
     upperMTUResult == PING_MTU_OK)
    return -1;

  if(minMTU+1 == maxMTU)
    return minMTU;

  int centerMTU = (minMTU + maxMTU) / 2;
  int part1 = interval(dstIp, minMTU, centerMTU);
  if(part1 > 0)
    return part1;
  int part2 = interval(dstIp, centerMTU, maxMTU);
  if(part2 > 0)
    return part2;
  return -1;
}
*/



/**
 * Searches the MTU by decreasing packet size and sending pings to a host.
 * @param dstIp The ip of the host.
 * @param maxMTU The MTU to decrease from.
 * @return The MTU or -1 when an error occurs.
 */
/*
int searchMTU(const char* dstIp, unsigned int maxMTU)
{
  int headerLen =  sizeof(struct iphdr) + sizeof(struct icmphdr);
  return interval(dstIp, headerLen+1, maxMTU);
}
*/
