/**
 * @file main.c
 *
 * @brief Implementation of the main application
 * 
 * @author Tim Leerhoff <tleerhof@web.de>, (C) 2016
 */
#include <stdlib.h>
#include <stdio.h>

#include "mtudetect.h"

/**
 * The main programm
 * @param argc The number of commandline parameters including the programname.
 * @param argv An array of commandline parameters.
 * @return The exitcode. 0 if everything ok.
 */
int main( int argc, char *argv[] )
{
  if( argc != 2 )
  {
    fprintf(stderr, "Usage: mtudetect <ip>\n");
    return 1;
  }
  int result = searchMTU(argv[1], 1500);
  if(result < 0)
  {
    fprintf(stderr, "An error occurred.\n");
    return 1;
  }
  else
  {
    fprintf(stdout, "%d\n", result);
    return 0;
  }
}

