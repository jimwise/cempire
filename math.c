/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: math.c,v 1.11 1998/03/01 01:48:27 jim Exp $
 */

/*
math.c -- various mathematical routines.

This file contains routines used to create random integers.  The
initialization routine 'rand_init' should be called at program startup.
The flavors of random integers that can be generated are:

    irand(n) -- returns a random integer in the range 0..n-1

Other routines include:

    dist (a, b) -- returns the straight-line distance between two locations.
*/

#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include "empire.h"
#include "extern.h"

long	dist (long, long);
long	irand (long);
int	isqrt (int);
void	rand_init (void);

void
rand_init (void)
{
	srand48((long)(time(0) & 0xFFFF));
}

long
irand (long high)
{
	if (high < 2)
		return 0;

	return (lrand48() % high);
}

/*
Return the distance between two locations.  This is simply
the max of the absolute differnce between the x and y coordinates.
*/

#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))

long
dist (long a, long b)
{
	long ax, ay, bx, by;

	ax = loc_row (a);
	ay = loc_col (a);
	bx = loc_row (b);
	by = loc_col (b);

	return (MAX (ABS (ax-bx), ABS (ay-by)));
}

/*
Find the square root of an integer.  We actually return the floor
of the square root using Newton's method.
*/

int
isqrt (int n)
{
	int guess;
	
	assert (n >= 0); /* can't take sqrt of negative number */

	if (n <= 1) return (n); /* do easy cases and avoid div by zero */
		
	guess = 2; /* gotta start somewhere */
	guess = (guess + n/guess) / 2;
	guess = (guess + n/guess) / 2;
	guess = (guess + n/guess) / 2;
	guess = (guess + n/guess) / 2;
	guess = (guess + n/guess) / 2;
	
	if (guess * guess > n) guess -= 1; /* take floor */
	return (guess);
}
