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
#ifndef PERFORMANCE
  int result = searchMTU(argv[1], 1500);
#else
  int result = searchMTU(argv[1], 65536);
#endif
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

