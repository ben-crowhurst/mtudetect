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
#include <ctype.h>

#include <syslog.h>

#include <signal.h>

#include <sys/types.h>		// for umask
#include <sys/stat.h>
#include <sys/resource.h> 	// for rlimit

#include <fcntl.h>		// for open
#include <unistd.h>		// for fork, close

#include <errno.h>

#include <sys/socket.h>         // socket()
#include <arpa/inet.h>          // Funktionen wie inet_addr()
#include <netinet/in.h>         // IP Protokolle, sockaddr_in Struktur und Funktionen wie htons()

#include "settings.h"
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
  closeAllFiledescriptors();
  int fd0 = open("/dev/null", O_RDWR);
  int fd1 = dup(0);
  int fd2 = dup(0);
  if(fd0 != 0 || fd1 != 1 || fd2 != 2 )
  {
    quitError("redirectStdInOutErr");
  }
}


void printUsage()
{
  fprintf(stderr, "Usage: mtudetect <ip> <mtu>\n");
}


void parseOptions(int argc, char *argv[], struct settings_t* settings )
{
  int c;
  while ((c = getopt (argc, argv, "dm:s:")) != -1)
  {
    switch (c)
    {
      case 'd':
        settings->daemonize = 0;
        break;
      case 'm':
        settings->check_mtu = atoi(optarg);
        break;
      case 's':
        settings->set_mtu = atoi(optarg);
        break;
      case '?':
        printUsage();
        abort();
        break;
      default:
        abort();
    }
  }

  if(settings->check_mtu == 0)
    settings->check_mtu = 1426;
  if(settings->set_mtu == 0)
    settings->set_mtu = 1312;

  if(!settings->daemonize)
  {
    fprintf(stdout, "check mtu = %d\n", settings->check_mtu);
    fprintf(stdout, "set mtu = %d\n", settings->set_mtu);
  }
}


void daemonize(struct settings_t* settings)
{
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
    close(settings->sock);
    exit(0);
  }

  // Create a new session and process group...
  setsid();

  // Change directory to root not to block other filesystems
  if(chdir("/") < 0)
  {
    quitError("chdir");
  }
}


int terminate = 0;
void hupHandler(int signum)
{
  fprintf(stdout, "kill hup");
  terminate = 1;
}

void doDetection()
{
  while(!terminate)
  {
    sleep(10);
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
  struct settings_t settings;
  settings.daemonize = 1;

  parseOptions(argc, argv, &settings);

  umask(0);

  if(settings.daemonize)
  {
    redirectStdInOutErr();
  }

  settings.sock = socket( AF_INET, SOCK_RAW, IPPROTO_ICMP );
  if(settings.sock == -1 )
  {
    quitError("socket");
  }

  int one = 1;
  if(setsockopt( settings.sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one) ) != 0 )
  {
    quitError("setsocketopt");
  }

  if(settings.daemonize)
  {
    daemonize(&settings);
  }

  __sighandler_t oldHupHandler = signal(SIGHUP, &hupHandler);

  openlog(argv[0], LOG_CONS, LOG_DAEMON);
  syslog(LOG_INFO, "Initialized daemon");

  //int result = checkMTU(argv[1], atoi(argv[2]));
  //fprintf(stdout, "Result: %s(%d)\n", getReturnValueText(result), result);

  doDetection();

  signal(SIGHUP, oldHupHandler);

  if(close(settings.sock) < 0)
  {
    quitError("close");
  }

  syslog(LOG_INFO, "Shutdown daemon");
  closelog();
  exit(0);
}
