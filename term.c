/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: term.c,v 1.14 1998/02/27 01:39:19 jim Exp $
 */

/*
term.c -- this file contains various routines used to control the
user communications area of the terminal.  This area consists of
the top 3 lines of the terminal where messages are displayed to the
user and input is acquired from the user.

There are two types of output in this area.  One type is interactive
output.  This consists of a prompt line and an error message line.
The other type of output is informational output.  The user must
be given time to read informational output.

Whenever input is received, the top three lines are cleared and the
screen refreshed as the user has had time to read these lines.  We
also clear the 'need_delay' flag, saying that the user has read the
information on the screen.

When information is to be displayed, if the 'need_delay' flag is set,
we refresh the screen and pause momentarily to give the user a chance
to read the lines.  The new information is then displayed, and the
'need_delay' flag is set.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <ctype.h>

#include "empire.h"
#include "extern.h"

void	clear_screen (void);
void    close_disp (void);
void    clreol (int, int);
void    comment (char *, ...);
void    delay (void);
void	empend (void);
void    error (char *, ...);
void    extra (char *, ...);
int	getint (char *message);
int	getyn (char *message);
char	get_chx (void);
char	get_c (void);
char    get_cq (void);
int     get_range (char *, int, int);
void	get_str (char *, int);
void    get_strq (char *, int);
void    help (char **, int);
void    huh (void);
void    info (char *, char *, char *);
void    pdebug (char *, ...);
void    pos_str (int, int, char *, ...);
void    prompt (char *, ...);
void    redraw (void);
void    set_need_delay (void);
void    topini (void);
void    topmsg(int, char *, ...);
void    ttinit (void);
void    vaddprintf (char *, va_list);
void	vcomment (char *, va_list);
void	vtopmsg(int, char *, va_list);

static int need_delay;

void
pdebug (char *s, ...)
{
	va_list ap;

	va_start(ap, s);

	if (!print_debug)
		return;

	vcomment (s, ap);

	va_end(ap);
}

/*
Here are routines that handle printing to the top few lines of the
screen.  'topini' should be called at initialization, and whenever
we finish printing information to the screen.
*/

void
topini (void)
{
	info (0, 0, 0);
}
/*
Write a message to one of the top lines.
*/

void
topmsg (int linep, char *buf, ...)
{
	va_list ap;

	va_start(ap, buf);

	vtopmsg(linep, buf, ap);

	va_end(ap);
}

void
vtopmsg (int linep, char *buf, va_list ap)
{
        if ((linep < 1) || (linep > NUMTOPS))
                linep = 1;

        move (linep - 1, 0);

        if (buf != NULL && strlen (buf) > 0)
                vaddprintf (buf, ap);

        clrtoeol ();
}

/*
Print a prompt on the first message line.
*/

void
prompt (char *buf, ...)
{
	va_list ap;

	va_start(ap, buf);

	vtopmsg (1, buf, ap);

	va_end(ap);
}

/*
Print an error message on the second message line.
*/

void
error (char *buf, ...)
{
	va_list ap;

	va_start(ap, buf);

	vtopmsg (2, buf, ap);

	va_end(ap);
}

/*
Print out extra information.
*/

void
extra (char *buf, ...)
{
	va_list ap;

	va_start(ap, buf);

	vtopmsg (3, buf, ap);

	va_end(ap);
}

/*
Print out a generic error message.
*/

void
huh (void)
{
	error ("Type H for Help.");
}

/*
Display information on the screen.  If the 'need_delay' flag is set,
we force a delay, then print the information.  After we print the
information, we set the need_delay flag.
*/

void
info (char *a, char *b, char *c)
{
	if (need_delay) delay ();
	topmsg (1, a);
	topmsg (2, b);
	topmsg (3, c);
	need_delay = (a || b || c);
}

void
set_need_delay (void)
{
	need_delay = 1;
}

void
comment (char *buf, ...)
{
	va_list ap;

	va_start(ap, buf);

	vcomment(buf, ap);

	va_end(ap);
}

void
vcomment (char *buf, va_list ap)
{
	if (need_delay) delay ();
	topmsg (1, 0);
	topmsg (2, 0);
	vtopmsg (3, buf, ap);
	need_delay = (buf != 0);
}
	
/*
Get a string from the user, echoing characters all the while.
*/

void
get_str (char *buf, int sizep)
{
	echo();
	get_strq(buf, sizep);
	noecho();
}

/*
Get a string from the user, ignoring the current echo mode.
*/

void
get_strq (char *buf, int sizep)
{
	sizep = sizep; /* size of buf, currently unused */

	nocrmode ();
	refresh ();
	getstr (buf);
	need_delay = FALSE;
	info (0, 0, 0);
	crmode ();
}

/*
Get a character from the user and convert it to uppercase.
*/

char
get_chx (void)
{
	uchar c;

	c = get_cq ();

	return (toupper(c));
}

/*
Input an integer from the user.
*/

int
getint (char *message)
{
	char buf[STRSIZE];
	char *p;

	for (;;) { /* until we get a legal number */
		prompt (message);
		get_str (buf, sizeof (buf));
		
		for (p = buf; *p; p++) {
			if (*p < '0' || *p > '9') {
				error ("Please enter an integer.");
				break;
			}
		}
		if (*p == 0) { /* no error yet? */
			if (p - buf > 7) /* too many digits? */
				error ("Please enter a small integer.");
			else return (atoi (buf));
		}
	}
}

/*
Input a character from the user with echoing.
*/

char
get_c (void)
{
	char c; /* one char and a null */

	echo ();
	c = get_cq ();
	noecho ();
	return (c);
}

/*
Input a character quietly.
*/

char
get_cq (void)
{
	char c;

	crmode ();
	refresh ();
	c = getch ();
	topini (); /* clear information lines */
	nocrmode ();
	return (c);
}

/*
Input a yes or no response from the user.  We loop until we get
a valid response.  We return TRUE iff the user replies 'y'.
*/

int
getyn (char *message)
{
	char c;

	for (;;) {
		prompt (message);
		c = get_chx ();

		if (c == 'Y') return (TRUE);
		if (c == 'N') return (FALSE);

		error ("Please answer Y or N.");
	}
}

/*
Input an integer in a range.
*/

int
get_range (char *message, int low, int high)
{
	int result;

	for (;;) {
		result = getint (message);

		if (result >= low && result <= high) return (result);

		error ("Please enter an integer in the range %d..%d.",
			low, high);
	}
}

/*
Print a screen of help information.
*/

void
help (char **text, int nlines)
{
	int i, r, c;
	int text_lines;

	text_lines = (nlines + 1) / 2; /* lines of text */

	clear_screen ();

	pos_str (NUMTOPS, 1, text[0]); /* mode */
	pos_str (NUMTOPS, 41, "See empire.doc for more information.");

	for (i = 1; i < nlines; i++) {
		if (i > text_lines)
			pos_str (i - text_lines + NUMTOPS + 1, 41, text[i]);
		else pos_str (i + NUMTOPS + 1, 1, text[i]);
	}

	pos_str (text_lines + NUMTOPS + 2,  1, "--Piece---Yours-Enemy-Moves-Hits-Cost");
	pos_str (text_lines + NUMTOPS + 2, 41, "--Piece---Yours-Enemy-Moves-Hits-Cost");

	for (i = 0; i < NUM_OBJECTS; i++) {
		if (i >= (NUM_OBJECTS+1)/2) {
			r = i - (NUM_OBJECTS+1)/2;
			c = 41;
		}
		else {
			r = i;
			c = 1;
		}
		pos_str (r + text_lines + NUMTOPS + 3, c, 
			"%-12s%c     %c%6d%5d%6d",
			piece_attr[i].nickname,
			piece_attr[i].sname,
			tolower (piece_attr[i].sname),
			piece_attr[i].speed,
			piece_attr[i].max_hits,
			piece_attr[i].build_time);

	}
	refresh ();
}

/*
Clear the end of a specified line starting at the specified column.
*/

void
clreol(int linep, int colp)
{
	move (linep, colp);
	clrtoeol();
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
	move (LINES - 1, 0);
	clrtoeol ();
	refresh ();
	endwin ();
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
Initialize the terminal.
*/

void
ttinit (void)
{
	initscr();
	noecho();
	crmode();
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
End the game by cleaning up the display.
*/

void
empend (void)
{
        close_disp ();
        exit (0);
}

