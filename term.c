/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * $Id: term.c,v 1.75 2009/09/15 15:11:40 jwise Exp $
 */

/*
 * term.c -- this file contains various routines used to control the
 * user communications area of the terminal.  This area consists of
 * the top 3 lines of the terminal where messages are displayed to the
 * user and input is acquired from the user.
 * 
 * There are two types of output in this area.  One type is interactive
 * output.  This consists of a prompt line and an error message line.
 * The other type of output is informational output.  The user must
 * be given time to read informational output.
 * 
 * Whenever input is received, the top three lines are cleared and the
 * screen refreshed as the user has had time to read these lines.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "empire.h"
#include "extern.h"

#ifdef USE_NCURSES
#include <ncurses.h>
#else 
#include <curses.h>
#endif 
#include <ctype.h>

void	alert (void);
void    error (char *, ...);
char	get_chx (void);
int	get_int (char *, int, int);
void	get_str (char *, const int);
int	getyn (char *message);
void    huh (void);
void	info (char *, ...);
void    prompt (char *, ...);
void    redraw (void);
void	term_clear (void);
void    term_end (void);
void    term_init (void);

#ifdef USE_COLOR
static void color_pairs(void);
short color_of(view_map_t *);
#endif


static WINDOW *statuswin, *infowin;

/*
 * Here are routines that handle printing to the top few lines of the
 * screen.
 */

/* Print a prompt on the status line. */

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

	scrollok(infowin, TRUE);
	scroll(infowin);
	wmove(infowin, NUMINFO-1, 0);
	vwprintw(infowin, fmt, ap);
	wrefresh(infowin);
	scrollok(infowin, FALSE);

	va_end(ap);
}

/* Print an error message on the second message line. */

void
error (char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	scrollok(statuswin, TRUE);
	wscrl(statuswin, 1);
	wmove(statuswin, 0, 0);
	vwprintw(statuswin, fmt, ap);
	wrefresh(statuswin);
	beep();
	napms(2000);
	wscrl(statuswin, -1);
	wrefresh(statuswin);
	scrollok(statuswin, FALSE);

	va_end(ap);
}

/* Print out a generic error message. */

void
huh (void)
{
	error ("Type ? for Help.");
}

/* Get a string from the user, echoing characters all the while. */

void
#if defined(USE_NCURSES) || !defined(__GNUC__)
/* ARGSUSED */
get_str (char *buf, const int sizep)
#else
/* ARGSUSED */
get_str (char *buf, const int sizep  __attribute__((__unused__)))
#endif
{
	echo();
	nocrmode();
	wgetnstr(statuswin, buf, sizep);
	crmode();
	noecho();
}

/* Get a character from the user and convert it to uppercase. */

char
get_chx (void)
{
        char c;

	crmode();
        c = wgetch(statuswin);
        nocrmode();

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
 * Input a yes or no response from the user.  We loop until we get
 * a valid response.  We return TRUE iff the user replies 'y'.
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
 * Clear the screen.  We must also kill information maintained about the
 * display.
 */

void
term_clear (void)
{
	wclear (stdscr);
	wrefresh(stdscr);
	kill_display ();
}

/* Redraw the screen. */

void
redraw (void)
{
	wrefresh(stdscr);
}

/* Clean up the display.  This routine gets called as we leave the game. */

void
term_end (void)
{
	wmove(stdscr, LINES - 1, 0);
	wclrtoeol (stdscr);
	wrefresh(stdscr);
	endwin ();
}

/* Initialize the terminal. */

void
term_init (void)
{
	initscr();
#ifdef USE_COLOR
        if (color) {
          start_color();
          color_pairs();
        }
#endif        
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

#ifdef USE_COLOR

#define PAIR_UNKNOWN 1
#define PAIR_LAND_UNOWNED 2
#define PAIR_SEA_UNOWNED 3
#define PAIR_LAND_USER 4
#define PAIR_SEA_USER 5
#define PAIR_LAND_COMP 6
#define PAIR_SEA_COMP 7
/* hack -- need to rework, so we know terrain type */
#define PAIR_AIR_USER 8
#define PAIR_AIR_COMP 9
#define PAIR_LAND_EMPTY 10
#define PAIR_SEA_EMPTY 11
static int cur_color = 0;

static void
color_pairs(void)
{
  init_pair(PAIR_UNKNOWN, COLOR_BLACK, COLOR_BLACK);
  init_pair(PAIR_LAND_UNOWNED, COLOR_BLACK, COLOR_GREEN);
  init_pair(PAIR_SEA_UNOWNED, COLOR_BLACK, COLOR_CYAN);
  init_pair(PAIR_LAND_USER, COLOR_BLUE, COLOR_GREEN);
  init_pair(PAIR_SEA_USER, COLOR_BLUE, COLOR_CYAN);
  init_pair(PAIR_LAND_COMP, COLOR_RED, COLOR_GREEN);
  init_pair(PAIR_SEA_COMP, COLOR_RED, COLOR_CYAN);
  init_pair(PAIR_AIR_USER, COLOR_BLUE, COLOR_WHITE);
  init_pair(PAIR_AIR_COMP, COLOR_RED, COLOR_WHITE);
  init_pair(PAIR_LAND_EMPTY, COLOR_GREEN, COLOR_GREEN);
  init_pair(PAIR_SEA_EMPTY, COLOR_CYAN, COLOR_CYAN);
}

short
color_of(view_map_t *vp)
{
  switch(vp->contents) {
  case ' ':
    cur_color = PAIR_UNKNOWN;
    break;
  case 'O':
  case 'A':
    cur_color = PAIR_LAND_USER;
    break;
  case 'P':
  case 'D':
  case 'S':
  case 'T':
  case 'C':
  case 'B':
    cur_color = PAIR_SEA_USER;
    break;
  case 'X':
  case 'a':
    cur_color = PAIR_LAND_COMP;
    break;
  case 'p':
  case 'd':
  case 's':
  case 't':
  case 'c':
  case 'b':
    cur_color = PAIR_SEA_COMP;
    break;
  case 'F':
    cur_color = PAIR_AIR_USER;
    /* if (map[loc].contents == '.') */
    /*   cur_color = PAIR_SEA_USER; */
    /* else */
    /*   cur_color = PAIR_LAND_USER; */
    break;
  case 'f':
    cur_color = PAIR_AIR_COMP;
    /* if (map[loc].contents == '.') */
    /*   cur_color = PAIR_SEA_COMP; */
    /* else */
    /*   cur_color = PAIR_LAND_COMP; */
    break;
  case '+':
    cur_color = PAIR_LAND_EMPTY;
    break;
  case '*':
    cur_color = PAIR_LAND_UNOWNED;
    break;
  case '.':
    cur_color = PAIR_SEA_EMPTY;
    break;
  }
  return cur_color;
}
#endif
