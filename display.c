/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: display.c,v 1.9 1998/02/26 02:16:23 jim Exp $
 */

/*
display.c -- This file contains routines for displaying sectors and
moving the cursor about in a sector.  We need to remember the following
information:

	the current map portion displayed on the screen;

	whether the displayed portion is from the user's or the computer's
	point of view;
*/

#ifdef SYSV
#include <string.h>
#else
#include <strings.h>
#endif

#include <curses.h>
#include "empire.h"
#include "extern.h"

long    cur_cursor (void);
int     cur_sector (void);
void    display_loc (int, view_map_t[], long);
void    display_locx (int, view_map_t[], long);
void    display_score (void);
void    display_screen (view_map_t[]);
void    disp_square(view_map_t *);
#ifdef USE_COLOR
void	init_colors(void);
#endif
void	kill_display (void);
int     move_cursor (long *, int);
int     on_screen (long);
void    print_sector (char, view_map_t[], int);
void    print_pzoom (char *, path_map_t *, view_map_t *);
void    print_pzoom_cell (path_map_t *, view_map_t *, int, int, int, int);
void    print_xzoom (view_map_t *);
void    print_zoom (view_map_t *);
void    print_zoom_cell (view_map_t *, int, int, int, int);
void	sector_change (void);
void	show_loc (view_map_t[], long);

static int whose_map = UNOWNED; /* user's or computer's point of view */
static int ref_row; /* map loc displayed in upper-left corner */
static int ref_col;
static int save_sector; /* the currently displayed sector */
static int save_cursor; /* currently displayed cursor position */
static int change_ok = TRUE; /* true if new sector may be displayed */

#ifdef USE_COLOR
void init_colors(void)
{
    start_color();

    init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
}
#endif /* USE_COLOR */

/*
This routine is called when the current display has been
trashed and no sector is shown on the screen.
*/

void kill_display (void)
{
	whose_map = UNOWNED;
}

/*
This routine is called when a new sector may be displayed on the
screen even if the location to be displayed is already on the screen.
*/

void sector_change (void)
{
	change_ok = TRUE;
}

/*
Return the currently displayed user sector, if any.  If a user
sector is not displayed, return -1.
*/

int cur_sector (void)
{
	if (whose_map != USER) return (-1);
	return (save_sector);
}

/*
Return the current position of the cursor.  If the user's map
is not on the screen, we return -1.
*/

long cur_cursor (void)
{
	if (whose_map != USER) return (-1);
	return (save_cursor);
}

/*
Display a location on the screen. We figure out the sector the
location is in and display that sector.  The cursor is left at
the requested location.

We redisplay the sector only if we either have been requested to
redisplay the sector, or if the location is not on the screen.
*/

void
display_loc (int whose, view_map_t vmap[], long loc)
{
	if (change_ok || whose != whose_map || !on_screen (loc))
		print_sector (whose, vmap, loc_sector (loc));
		
	show_loc (vmap, loc);
}

/*
Display a location iff the location is on the screen.
*/

void
display_locx (int whose, view_map_t vmap[], long loc)
{
	if (whose == whose_map && on_screen (loc))
		show_loc (vmap, loc);
}

/*
Display a location which exists on the screen.
*/

void
show_loc (view_map_t vmap[], long loc)
{
	int r, c;
	
	r = loc_row (loc);
	c = loc_col (loc);
	(void) move (r-ref_row+NUMTOPS, c-ref_col);
	disp_square(&vmap[loc]);
	save_cursor = loc; /* remember cursor location */
	(void) move (r-ref_row+NUMTOPS, c-ref_col);
}

/*
Print a sector of the user's on the screen.  If it is already displayed,
we do nothing.  Otherwise we redraw the screen.  Someday, some intelligence
in doing this might be interesting.  We heavily depend on curses to update
the screen in a reasonable fashion.

If the desired sector
is not displayed, we clear the screen.  We then update the screen
to reflect the current map.  We heavily depend on curses to correctly
optimize the redrawing of the screen.

When redrawing the screen, we figure out where the
center of the sector is in relation to the map.  We then compute
the screen coordinates where we want to display the center of the
sector.  We will remember the sector displayed, the map displayed,
and the map location that appears in the upper-left corner of the
screen.
*/
 
void
print_sector (char whose, view_map_t vmap[], int sector)
{
	int first_row, first_col, last_row, last_col;
	int display_rows, display_cols;
	int r, c;

	save_sector = sector; /* remember last sector displayed */
	change_ok = FALSE; /* we are displaying a new sector */

	display_rows = lines - NUMTOPS - 1; /* num lines to display */
	display_cols = cols - NUMSIDES;

	/* compute row and column edges of sector */
	first_row = sector_row (sector) * ROWS_PER_SECTOR;
	first_col = sector_col (sector) * COLS_PER_SECTOR;
	last_row = first_row + ROWS_PER_SECTOR - 1;
	last_col = first_col + COLS_PER_SECTOR - 1;

	if (!(whose == whose_map /* correct map is on screen? */
	   && ref_row <= first_row /* top row on screen? */
	   && ref_col <= first_col /* first col on screen? */
	   && ref_row + display_rows - 1 >= last_row /* bot row on screen? */
	   && ref_col + display_cols - 1 >= last_col)) /* last col on screen? */
	(void) clear (); /* erase current screen */

	/* figure out first row and col to print; subtract half
	   the extra lines from the first line */

	ref_row = first_row - (display_rows - ROWS_PER_SECTOR) / 2;
	ref_col = first_col - (display_cols - COLS_PER_SECTOR) / 2;

	/* try not to go past bottom of map */
	if (ref_row + display_rows - 1 > MAP_HEIGHT - 1)
		ref_row = MAP_HEIGHT - 1 - (display_rows - 1);

	/* never go past top of map */
        if (ref_row < 0) ref_row = 0;

	/* same with columns */
	if (ref_col + display_cols - 1 > MAP_WIDTH - 1)
		ref_col = MAP_WIDTH - 1 - (display_cols - 1);

	if (ref_col < 0) ref_col = 0;

        whose_map = whose; /* remember whose map is displayed */
	display_screen (vmap);

	/* print x-coordinates along bottom of screen */
	for (c = ref_col; c < ref_col + display_cols && c < MAP_WIDTH; c++)
	if (c % 10 == 0) {
		pos_str (lines-1, c-ref_col, "%d", c);
	}
	/* print y-coordinates along right of screen */
	for (r = ref_row; r < ref_row + display_rows && r < MAP_HEIGHT; r++) {
		if (r % 2 == 0)
			pos_str (r-ref_row+NUMTOPS, cols-NUMSIDES+1, "%2d", r);
		else pos_str (r-ref_row+NUMTOPS, cols-NUMSIDES+1, "  ");
	}
	/* print round number */
	(void) sprintf (jnkbuf, "Sector %d Round %ld", sector, date);
	for (r = 0; jnkbuf[r] != '\0'; r++) {
		if (r+NUMTOPS >= MAP_HEIGHT) break;
		(void) move (r+NUMTOPS, cols-NUMSIDES+4);
		(void) addch ((chtype)jnkbuf[r]);
	}
}

/*
Display the contents of a single map square.

Fancy color hacks are done here. At the moment this is kind of bogus,
because the color doesn't convey any extra information, it just looks
pretty.
*/


void
disp_square(view_map_t *vp)
{
#ifdef USE_COLOR
	switch(vp->contents)
	{
	case '+':
		attron(COLOR_PAIR(COLOR_GREEN));
		break;
	case '.':
		attron(COLOR_PAIR(COLOR_CYAN));
		break;
	case 'a':
	case 'f':
	case 'p':
	case 'd':
	case 'b':
	case 't':
	case 'c':
	case 's':
	case 'z':
	case 'X':
		attron(COLOR_PAIR(COLOR_RED));
		break;
	default:
		attron(COLOR_PAIR(COLOR_WHITE));
		break;
	}
#endif /* USE_COLOR */
	(void) addch ((chtype)vp->contents);
#ifdef USE_COLOR
	attrset(0);
	attron(COLOR_PAIR(COLOR_WHITE));
#endif /* USE_COLOR */
}


/*
Display the portion of the map that appears on the screen.
*/

void
display_screen (view_map_t vmap[])
{
	int display_rows, display_cols;
	int r, c;
	long t;

	display_rows = lines - NUMTOPS - 1; /* num lines to display */
	display_cols = cols - NUMSIDES;

	for (r = ref_row; r < ref_row + display_rows && r < MAP_HEIGHT; r++)
	for (c = ref_col; c < ref_col + display_cols && c < MAP_WIDTH; c++) {
		t = row_col_loc (r, c);
		(void) move (r-ref_row+NUMTOPS, c-ref_col);
		disp_square(&vmap[t]);
	}
}

/*
Move the cursor in a specified direction.  We return TRUE if the
cursor remains in the currently displayed screen, otherwise FALSE.
We display the cursor on the screen, if possible.
*/

int
move_cursor (long *cursor, int offset)
/* cursor == current cursor position */
/* offset == offset to add to cursor */
{
	long t;
	int r, c;
 
	t = *cursor + offset; /* proposed location */
	if (!map[t].on_board) return (FALSE); /* trying to move off map */
	if (!on_screen (t)) return (FALSE); /* loc is off screen */
	
	*cursor = t; /* update cursor position */
	save_cursor = *cursor;
	       
	r = loc_row (save_cursor);
	c = loc_col (save_cursor);
	(void) move (r-ref_row+NUMTOPS, c-ref_col);
       
	return (TRUE);
}

/*
See if a location is displayed on the screen.
*/

int
on_screen (long loc)
{
	int new_r, new_c;
	
	new_r = loc_row (loc);
	new_c = loc_col (loc);

	if (new_r < ref_row /* past top of screen */
	 || new_r - ref_row > lines - NUMTOPS - 1 /* past bot of screen? */
	 || new_c < ref_col /* past left edge of screen? */
	 || new_c - ref_col > cols - NUMSIDES) /* past right edge of screen? */
	return (FALSE);

	return (TRUE);
}

/* Print a view map for debugging. */

void
print_xzoom (view_map_t *vmap)
{
	print_zoom (vmap);
#if 0
	prompt ("Hit a key: ");
	(void) get_chx (); /* wait for user */
#endif
}

/*
Print a condensed version of the map.
*/

char zoom_list[] = "XO*tcbsdpfaTCBSDPFAzZ+. ";

void
print_zoom (view_map_t *vmap)
{
	int row_inc, col_inc;
	int r, c;

	kill_display ();

	row_inc = (MAP_HEIGHT + lines - NUMTOPS - 1) / (lines - NUMTOPS);
	col_inc = (MAP_WIDTH + cols - 1) / (cols - 1);

	for (r = 0; r < MAP_HEIGHT; r += row_inc)
	for (c = 0; c < MAP_WIDTH; c += col_inc)
	print_zoom_cell (vmap, r, c, row_inc, col_inc);

	pos_str (0, 0, "Round #%d", date);
	
	(void) refresh ();
}

/*
Print a single cell in condensed format.
*/

void
print_zoom_cell (view_map_t *vmap, int row, int col, int row_inc, int col_inc)
{
	int r, c;
	char cell;

	cell = ' ';
	for (r = row; r < row + row_inc; r++)
	for (c = col; c < col + col_inc; c++)
	if (strchr (zoom_list, vmap[row_col_loc(r,c)].contents)
		< strchr (zoom_list, cell))
	cell = vmap[row_col_loc(r,c)].contents;
	
	(void) move (row/row_inc + NUMTOPS, col/col_inc);
	(void) addch ((chtype)cell);
}

/*
Print a condensed version of a pathmap.
*/

void
print_pzoom (char *s, path_map_t *pmap, view_map_t *vmap)
{
	int row_inc, col_inc;
	int r, c;

	kill_display ();

	row_inc = (MAP_HEIGHT + lines - NUMTOPS - 1) / (lines - NUMTOPS);
	col_inc = (MAP_WIDTH + cols - 1) / (cols - 1);

	for (r = 0; r < MAP_HEIGHT; r += row_inc)
	for (c = 0; c < MAP_WIDTH; c += col_inc)
	print_pzoom_cell (pmap, vmap, r, c, row_inc, col_inc);

	prompt (s);
	(void) get_chx (); /* wait for user */
	
	(void) refresh ();
}

/*
Print a single cell of a pathmap in condensed format.
We average all squares in the cell and take the mod 10 value.
Squares with a value of -1 are printed with '-', squares with
a value of INFINITY/2 are printed with 'P', and squares with
a value of INFINITY are printed with 'Z'.  Squares with a value
between P and Z are printed as U.
*/

void
print_pzoom_cell (path_map_t *pmap, view_map_t *vmap, int row, int col, int row_inc, int col_inc)
{
	int r, c;
	int sum, d;
	char cell;

	sum = 0;
	d = 0; /* number of squares in cell */
	
	for (r = row; r < row + row_inc; r++)
	for (c = col; c < col + col_inc; c++) {
		sum += pmap[row_col_loc(r,c)].cost;
		d += 1;
	}
	sum /= d;
	
	if (pmap[row_col_loc(row,col)].terrain == T_PATH) cell = '-';
	else if (sum < 0) cell = '!';
	else if (sum == INFINITY/2) cell = 'P';
	else if (sum == INFINITY) cell = ' ';
	else if (sum > INFINITY/2) cell = 'U';
	else {
		sum %= 36;
		if (sum < 10) cell = sum + '0';
		else cell = sum - 10 + 'a';
	}
	
	if (cell == ' ')
		print_zoom_cell (vmap, row, col, row_inc, col_inc);
	else {
		(void) move (row/row_inc + NUMTOPS, col/col_inc);
		(void) addch ((chtype)cell);
	}
}

/*
Display the score off in the corner of the screen.
*/

void
display_score (void)
{
	pos_str (0, cols-12, " User  Comp");
	pos_str (1, cols-12, "%5d %5d", user_score, comp_score);
}
