/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: term.c,v 1.41 1998/03/03 13:57:48 jim Exp $
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
void	empend (void);
void	emp_beep (void);
void    error (char *, ...);
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
	wrefresh(infowin);
	getstr (buf);
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
	wrefresh(infowin);
	c = getch ();
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
	piece_type_t j;
	int text_lines;

	text_lines = (nlines + 1) / 2; /* lines of text */

	term_clear();

	pos_str (NUMTOPS, 1, text[0]); /* mode */
	pos_str (NUMTOPS, 41, "See empire.doc for more information.");

	for (i = 1; i < nlines; i++) {
		if (i > text_lines)
			pos_str (i - text_lines + NUMTOPS + 1, 41, text[i]);
		else pos_str (i + NUMTOPS + 1, 1, text[i]);
	}

	pos_str (text_lines + NUMTOPS + 2,  1, "--Piece---Yours-Enemy-Moves-Hits-Cost");
	pos_str (text_lines + NUMTOPS + 2, 41, "--Piece---Yours-Enemy-Moves-Hits-Cost");

	for (j = ARMY; j < NUM_OBJECTS; j++)
	{
		if (j >= (NUM_OBJECTS+1)/2) {
			r = j - (NUM_OBJECTS+1)/2;
			c = 41;
		}
		else
		{
			r = j;
			c = 1;
		}
		pos_str (r + text_lines + NUMTOPS + 3, c, 
			"%-12s%c     %c%6d%5d%6d",
			piece_attr[j].nickname,
			piece_attr[j].sname,
			tolower (piece_attr[j].sname),
			piece_attr[j].speed,
			piece_attr[j].max_hits,
			piece_attr[j].build_time);

	}
	wrefresh(infowin);
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
#ifdef USE_COLOR
	init_colors();
#endif /* USE_COLOR */
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

/*
End the game by cleaning up the display.
*/

void
empend (void)
{
        term_end();
        exit (0);
}

void
emp_beep (void)
{
	beep();
	flash();
}
