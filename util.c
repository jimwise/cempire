/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: util.c,v 1.11 1998/02/27 00:02:32 jim Exp $
 */

/*
util.c -- various utility routines.
*/

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <curses.h>
#include <ctype.h>
#include "empire.h"
#include "extern.h"

void    check (void);
void	check_cargo (piece_info_t *, int);
void	check_obj (piece_info_t **, int);
void	check_obj_cargo (piece_info_t **);
void    clear_screen (void);
void	close_disp (void);
void    clreol(int, int);
void    delay (void);
void    empend (void);
void	emp_panic (char *, int);
void    pos_str (int, int, char *, ...);
void    redraw (void);
void    ttinit (void);
void    tupper (uchar *);
char    upper (uchar);
void	vaddprintf (char *, va_list);
void    ver (void);

/*
Convert a string to uppercase.
Shirley this is defined elsewhere?
*/

void
tupper (uchar *str)
{
	while (*str) {
		if (islower (*str)) *str = upper (*str);
		str++;
	}
}

/*
Convert a character to uppercase (if it is lowercase)
*/

char
upper (uchar c)
{
	if (islower (c))
		return toupper (c);
	else return c;
}

/*
Clear the end of a specified line starting at the specified column.
*/

void
clreol(int linep, int colp)
{
	(void) move (linep, colp);
	(void) clrtoeol();
}

/*
Initialize the terminal.
*/

void
ttinit (void)
{
	(void) initscr();
	(void) noecho();
	(void) crmode();
#ifdef USE_COLOR
	init_colors();
#endif /* USE_COLOR */
	lines = LINES;
	cols = COLS;
	if (lines > MAP_HEIGHT + NUMTOPS + 1)
		lines = MAP_HEIGHT + NUMTOPS + 1;
	if (cols > MAP_WIDTH + NUMSIDES)
		cols = MAP_WIDTH + NUMSIDES;
}


/*
Clear the screen.  We must also kill information maintained about the
display.
*/

void
clear_screen (void)
{
	clear ();
	refresh ();
	kill_display ();
}

/*
Redraw the screen.
*/

void
redraw (void)
{
	clearok (curscr, TRUE);
	refresh ();
}

/*
Wait a little bit to give user a chance to see a message.  We refresh
the screen and pause for a few milliseconds.
*/

void
delay (void)
{
	refresh ();
	napms (delay_time); /* pause a bit */
}


/*
Clean up the display.  This routine gets called as we leave the game.
*/

void
close_disp (void)
{
	(void) move (LINES - 1, 0);
	(void) clrtoeol ();
	(void) refresh ();
	(void) endwin ();
}

/*
Position the cursor and output a string.
*/

void
pos_str (int row, int col, char *str, ...)
{
	va_list ap;

	va_start(ap, str);

	move (row, col);
	vaddprintf (str, ap);

	va_end(ap);
}

void
vaddprintf (char *str, va_list ap)
{
	char junkbuf[STRSIZE];
	
	vsprintf (junkbuf, str, ap);
	addstr (junkbuf);
}

/*
End the game by cleaning up the display.
*/

void
empend (void)
{
	close_disp ();
	exit (0);
}

void
ver (void)
{
        (void) addstr ("EMPIRE, Version 5.00 site Amdahl 1-Apr-1988");
}

/*
Here is a little routine to perform consistency checking on the
database.  I'm finding that my database is becoming inconsistent,
and I've no idea why.  Possibly this will help.

We perform the following functions:

1)  Make sure no list contains loops.

2)  Make sure every object is in either the free list with 0 hits,
or it is in the correct object list and a location list with non-zero hits,
and an appropriate owner.

3)  Make sure every city is on the map.

4)  Make sure every object is in the correct location and that
objects on the map have non-zero hits.

5)  Make sure every object in a cargo list has a ship pointer.

6)  Make sure every object with a ship pointer is in that ship's
cargo list.
*/

static int in_free[LIST_SIZE]; /* TRUE if object in free list */
static int in_obj[LIST_SIZE]; /* TRUE if object in obj list */
static int in_loc[LIST_SIZE]; /* TRUE if object in a loc list */
static int in_cargo[LIST_SIZE]; /* TRUE if object in a cargo list */

void
check (void)
{
	long i, j;
	piece_info_t *p;
	
	/* nothing in any list yet */
	for (i = 0; i < LIST_SIZE; i++) {
		in_free[i] = 0;
		in_obj[i] = 0;
		in_loc[i] = 0;
		in_cargo[i] = 0;
	}
		
	/* Mark all objects in free list.  Make sure objects in free list
	have zero hits. */
	
	for (p = free_list; p != NULL; p = p->piece_link.next) {
		i = p - object;
		assert (!in_free[i]);
		in_free[i] = 1;
		assert (p->hits == 0);
		if (p->piece_link.prev)
			assert (p->piece_link.prev->piece_link.next == p);
	}
	
	/* Mark all objects in the map.
	Check that cities are in corect location.
	Check that objects are in correct location,
	have a good owner, and good hits. */
	
	for (i = 0; i < MAP_SIZE; i++) {
		if (map[i].cityp) assert (map[i].cityp->loc == i);
		
		for (p = map[i].objp; p != NULL; p = p->loc_link.next) {
			assert (p->loc == i);
			assert (p->hits > 0);
			assert (p->owner == USER || p->owner == COMP);
				
			j = p - object;
			assert (!in_loc[j]);
			in_loc[j] = 1;
			
			if (p->loc_link.prev)
				assert (p->loc_link.prev->loc_link.next == p);
		}
	}

	/* make sure all cities are on map */

	for (i = 0; i < NUM_CITY; i++)
		assert (map[city[i].loc].cityp == &(city[i]));

	/* Scan object lists. */
	
	check_obj (comp_obj, COMP);
	check_obj (user_obj, USER);
	
	/* Scan cargo lists. */
	
	check_cargo (user_obj[TRANSPORT], ARMY);
	check_cargo (comp_obj[TRANSPORT], ARMY);
	check_cargo (user_obj[CARRIER], FIGHTER);
	check_cargo (comp_obj[CARRIER], FIGHTER);
	
	/* Make sure all objects with ship pointers are in cargo. */

	check_obj_cargo (comp_obj);
	check_obj_cargo (user_obj);
	
	/* Make sure every object is either free or in loc and obj list. */

	for (i = 0; i < LIST_SIZE; i++)
		assert (in_free[i] != (in_loc[i] && in_obj[i]));
}

/*
Check object lists.  We look for:

1)  Loops and bad prev pointers.

2)  Dead objects.

3)  Invalid types.

4)  Invalid owners.
*/

void
check_obj (piece_info_t **list, int owner)
{
	long i, j;
	piece_info_t *p;
	
	for (i = 0; i < NUM_OBJECTS; i++)
	for (p = list[i]; p != NULL; p = p->piece_link.next) {
		assert (p->owner == owner);
		assert (p->type == i);
		assert (p->hits > 0);
		
		j = p - object;
		assert (!in_obj[j]);
		in_obj[j] = 1;
	
		if (p->piece_link.prev)
			assert (p->piece_link.prev->piece_link.next == p);
	}
}

/*
Check cargo lists.  We assume object lists are valid.
as we will place bits in the 'in_cargo' array that are used by
'check_obj'.

Check for:

1)  Number of items in list is same as cargo count.

2)  Type of cargo is correct.

3)  Location of cargo matches location of ship.

4)  Ship pointer of cargo points to correct ship.

5)  There are no loops in cargo list and prev ptrs are correct.

6)  All cargo is alive.
*/

void
check_cargo (piece_info_t *list, int cargo_type)
{
	piece_info_t *p, *q;
	long j, count;
	
	for (p = list; p != NULL; p = p->piece_link.next) {
		count = 0;
		for (q = p->cargo; q != NULL; q = q->cargo_link.next) {
			count += 1; /* count items in list */
			assert (q->type == cargo_type);
			assert (q->owner == p->owner);
			assert (q->hits > 0);
			assert (q->ship == p);
			assert (q->loc == p->loc);
			
			j = q - object;
			assert (!in_cargo[j]);
			in_cargo[j] = 1;

			if (p->cargo_link.prev)
				assert (p->cargo_link.prev->cargo_link.next == p);
                }
		assert (count == p->count);
        }
}

/*
Scan through object lists making sure every object with a ship
pointer appears in a cargo list.  We assume object and cargo
lists are valid.
*/

void
check_obj_cargo (piece_info_t **list)
{
	piece_info_t *p;
	long i;

	for (i = 0; i < NUM_OBJECTS; i++)
	for (p = list[i]; p != NULL; p = p->piece_link.next) {
		if (p->ship) assert (in_cargo[p-object]);
	}
}

/*
 * Print a message and then exit.
 */

void
emp_panic (char *file, int line)
{
	printf("empire panic: file %s, line %d\n", file, line);
	
	abort();
}
