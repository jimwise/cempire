/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: main.c,v 1.13 1998/08/09 00:14:55 jwise Exp $
 */

/*
 * main.c -- parse command line for empire
 * 
 * options:
 * 
 *    -w water: percentage of map that is water.  Must be in the range
 *             10..90.  Default is 70.
 *
 *    -s smooth: amount of smoothing performed to generate map.  Must
 * 	       be a nonnegative integer.  Default is 5.
 * 	       
 *    -S save_interval: sets turn interval between saves.
 * 	       default is 10
 */

#include <stdio.h>
#include <stdlib.h>
#include "empire.h"
#include "extern.h"

#define OPTFLAGS "w:s:d:S:"

int
main (int argc, char *argv[])
{
	int c;
	int errflg = 0;
	int wflg, sflg, Sflg;
	int land;
	
	wflg = 70; /* set defaults */
	sflg = 5;
	Sflg = 10;

	/* extract command line options */

	while ((c = getopt (argc, argv, OPTFLAGS)) != EOF) {
		switch (c) {
		case 'w':
			wflg = atoi (optarg);
			if (wflg < 10 || wflg > 90)
			{
				fprintf (stderr, "empire: -w argument must be in the range 0..90.\n");
				exit (1);
			}
			break;
		case 's':
			sflg = atoi (optarg);
			if (sflg < 0)
			{
        		        fprintf(stderr, "empire: -s argument must be greater than or equal to zero.\n");
				exit (1);
			}
			break;
		case 'S':
			Sflg = atoi (optarg);
			if (Sflg < 1)
			{
				fprintf(stderr, "empire: -S argument must be greater than or equal to zero.\n");
				exit (1);
			}
			break;
		case '?': /* illegal option */
			errflg++;
			break;
		}
	}

	if (errflg || ((argc - optind) != 0))
	{
		fprintf(stderr, "empire: usage: empire [-w water] [-s smooth] [-d delay] [-S save_interval]\n");
		exit (1);
	}

	SMOOTH = sflg;
	WATER_RATIO = wflg;
	save_interval = Sflg;

	/* compute min distance between cities */
	land = MAP_SIZE * (100 - WATER_RATIO) / 100;	/* available land		*/
	land /= NUM_CITY;				/* land per city		*/
	MIN_CITY_DIST = isqrt(land);			/* distance between cities	*/

	empire(); /* call main routine */
	return (0);
}
