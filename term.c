/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: term.c,v 1.54 1998/03/09 17:22:01 jim Exp $
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
screen refreshed as the user has had time to read these lines.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <ctype.h>

#include "empire.h"
#include "extern.h"

void    delay (void);
void	alert (void);
void    error (char *, ...);
int	getyn (char *message);
char	get_chx (void);
char    get_cq (void);
void	get_str (char *, int);
void    help (char **, int);
void    huh (void);
void	info (char *, ...);
void    pos_str (int, int, char *, ...);
void    prompt (char *, ...);
void    redraw (void);
void	term_clear (void);
void    term_end (void);
void    term_init (void);

static WINDOW *statuswin, *infowin;

/*
Here are routines that handle printing to the top few lines of the
screen.
*/

/*
Print a prompt on the status line.
*/

void
prompt (char *buf, ...)
{
	va_list ap;

	va_start(ap, buf);

	wmove(statuswin, 0, 0);
	wclrtoeol(statuswin);
	vwprintw(statuswin, buf, ap);
	wrefresh(statuswin);

	va_end(ap);
}

void
info (char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	idlok(infowin, TRUE);
	scrollok(infowin, TRUE);
	scroll(infowin);
	wmove(infowin, NUMINFO-1, 0);
	vwprintw(infowin, fmt, ap);
	wrefresh(infowin);
	scrollok(infowin, FALSE);

	va_end(fmt);
}

/*
Print an error message on the second message line.
*/

void
error (char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	idlok(statuswin, TRUE);
	scrollok(statuswin, TRUE);
	wscrl(statuswin, 1);
	wmove(statuswin, 0, 0);
	vwprintw(statuswin, fmt, ap);
	wrefresh(statuswin);
	beep();
	delay();
	wscrl(statuswin, -1);
	wrefresh(statuswin);
	scrollok(statuswin, FALSE);

	va_end(fmt);
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
Get a string from the user, echoing characters all the while.
*/

void
get_str (char *buf, int sizep)
{
	echo();
	nocrmode();
	wgetnstr(statuswin, buf, sizep);
	crmode();
	noecho();
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
 * Input an integer from the user.
 * low and high set bounds
 */

int
get_int (char *message, int low, int high)
{
	char	buf[STRSIZE], *end;
	long	l;

	while (1)
	{
		prompt(message);
		get_str(buf, sizeof(buf));

		l = strtol(buf, &end, 10);

		if (*end != '\0')
		{
			error ("Please enter an integer.");
			continue;
		}

		if ((l >= low) && (l <= high))
			return (l);
		else
			error ("Please enter an integer in the range %d..%d.", low, high);
	}
}

/*
Input a character quietly.
*/

char
get_cq (void)
{
	char c;

	crmode();
	c = wgetch(statuswin);
	nocrmode();
	return(c);
}

/*
Input a yes or no response from the user.  We loop until we get
a valid response.  We return TRUE iff the user replies 'y'.
*/

int
getyn (char *message)
{
	char c;

	while (1)
	{
		prompt (message);
		c = get_chx ();

		if (c == 'Y')
			return (TRUE);
		if (c == 'N')
			return (FALSE);

		error ("Please answer Y or N.");
	}
}

/*
Clear the screen.  We must also kill information maintained about the
display.
*/

void
term_clear (void)
{
	wclear (stdscr);
	wrefresh(stdscr);
	kill_display ();
}

/*
Redraw the screen.
*/

void
redraw (void)
{
	clearok (curscr, TRUE);
	wrefresh(stdscr);
}

/*
Wait a little bit to give user a chance to see a message.  We refresh
the screen and pause for a few milliseconds.
*/

void
delay (void)
{
	wrefresh(infowin);
	napms (delay_time); /* pause a bit */
}


/*
Clean up the display.  This routine gets called as we leave the game.
*/

void
term_end (void)
{
	wmove(stdscr, LINES - 1, 0);
	wclrtoeol (stdscr);
	wrefresh(stdscr);
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

	wmove(stdscr, row, col);
	vwprintw(stdscr, str, ap);
	wrefresh(stdscr);

	va_end(ap);
}

/*
Initialize the terminal.
*/

void
term_init (void)
{
	initscr();
	noecho();
	crmode();
	lines = LINES;
	cols = COLS;
	if (lines > MAP_HEIGHT + NUMTOPS + 1)
		lines = MAP_HEIGHT + NUMTOPS + 1;
	if (cols > MAP_WIDTH + NUMSIDES)
		cols = MAP_WIDTH + NUMSIDES;
	
	statuswin = newwin(1, cols-12, 0, 0);
	wattrset(statuswin, A_REVERSE);
	infowin = newwin(NUMINFO, cols-12, 1, 0);
}

void
alert (void)
{
	beep();
}
