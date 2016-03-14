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
 * @file main.c
 *
 * @brief Implementation of the main application
 */
#include <stdlib.h>
#include <stdio.h>

#include <syslog.h>

#include <sys/types.h>		// for umask
#include <sys/stat.h>
#include <sys/resource.h> 	// for rlimit

#include <fcntl.h>		// for open
#include <unistd.h>		// for fork, close

#include <errno.h>

#include <sys/socket.h>         // socket()
#include <arpa/inet.h>          // Funktionen wie inet_addr()
#include <netinet/in.h>         // IP Protokolle, sockaddr_in Struktur und Funktionen wie htons()

#include "mtudetect.h"

/**
 * Prints the last error and exits the application.
 */
void quitError(const char* mesg)
{
//  perror(mesg);
  syslog(LOG_ERR, "%s (%d)", mesg, errno); 
  exit(1);
}

void closeAllFiledescriptors()
{
  struct rlimit rl;
  if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
  {
    quitError("getrlimit");
  }
  if(rl.rlim_max == RLIM_INFINITY)
  {
    rl.rlim_max = 1024;
  }

  int fd;
  for(fd=0;fd<rl.rlim_max;fd++)
  {
    close(fd);
  }
}

void redirectStdInOutErr()
{
  int fd0 = open("/dev/null", O_RDWR);
  int fd1 = dup(0);
  int fd2 = dup(0);
  if(fd0 != 0 || fd1 != 1 || fd2 != 2 )
  {
    quitError("redirectStdInOutErr");
  }
}

/**
 * The main programm
 * @param argc The number of commandline parameters including the programname.
 * @param argv An array of commandline parameters.
 * @return The exitcode. 0 if everything ok.
 */
int main( int argc, char *argv[] )
{
  if( argc != 3 )
  {
    fprintf(stderr, "Usage: mtudetect <ip> <mtu>\n");
    return 1;
  }

  umask(0);
  closeAllFiledescriptors();
  redirectStdInOutErr();

  int sock = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
  if(sock == -1 )
  {
    quitError("socket");
  }

  int one = 1;
  if(setsockopt( sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one) ) != 0 )
  {
    quitError("setsocketopt");
  }

  // Become a normal user
  if(seteuid(1) < 0)
  {
    quitError("seteuid");
  }

  // fork parent
  pid_t pid = fork();
  if(pid < 0)
  {
    // error
    quitError("fork");
  }
  else
  if(pid == 0)
  {
    // child
  }
  else
  {
    // parent
    close(sock);
    exit(0);
  }


  // Create a new session and process group...
  setsid();


  // Change directory to root not to block other filesystems
  if(chdir("/") < 0)
  {
    quitError("chdir");
  }


  openlog(argv[0], LOG_CONS, LOG_DAEMON);
  syslog(LOG_INFO, "Initialized daemon");

  if(close(sock) < 0)
  {
    quitError("close");
  }

  int result = checkMTU(argv[1], atoi(argv[2]));
  fprintf(stdout, "Result: %s(%d)\n", getReturnValueText(result), result);


  //sleep(1000);

  syslog(LOG_INFO, "Shutdown daemon");
  closelog();
  exit(result);
}
