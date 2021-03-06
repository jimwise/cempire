/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * $Id: empire.c,v 1.41 2003/10/23 15:40:44 jwise Exp $
 */

/*
 * empire.c -- this file contains initialization code, the main command
 * parser, and the simple commands.
 */

#include <stdio.h>
#include <stdlib.h>
#include "empire.h"
#include "extern.h"

void	empire (void);
static void	c_debug (char order);
static void	c_examine (void);
static void	c_give (void);
static void	c_map (void);
static void	c_movie (void);
static void	c_quit (void);
static void	c_sector (void);
static void	do_command (char);
static void	emp_end (void);
static void	emp_start (void);

void
empire (void)
{
	char order;
	int turn = 0;

	emp_start();

	term_clear(); /* nothing on screen */
	info(VERSION_STRING);

	if (!restore_game()) /* try to restore previous game */
		init_game(); /* otherwise init a new game */

	/* Command loop starts here. */

	while(1)
	{
		/* until user quits */
		if (automove)
		{
			/* don't ask for cmd in auto mode */
			user_move();
			comp_move();
			if (++turn % save_interval == 0)
				save_game();
	    	}
	   	else
		{
	        	prompt("Your orders? ");
	        	order = get_chx(); /* get a command */
			do_command(order);
		}
	}
}

/* Execute a command. */

static void
do_command (char orders)
{
	char e;
	int i, loc, ncycle;

	switch (orders) {
	case 'A': /* turn on auto move mode */
		automove = TRUE;
		info("Entering Auto-Mode");
		user_move();
		comp_move();
		save_game();
		break;

	case 'C': /* give a city to the computer */
		c_give();
		break;
	
	case 'D': /* display round number */
		error("Round #%d", date);
		break;

	case 'E': /* examine enemy map */
		if (resigned)
			c_examine();
		else
			huh(); /* illegal command */
		break;

	case 'F': /* print map to file */
		c_map();
		break;

	case 'G': /* give one free enemy move */
		comp_move();
		break;

	case '?': /* help */
		help(help_cmd, cmd_lines);
		break;

	case 'J': /* edit mode */
		loc = cur_sector ();
		if (loc == -1)
			loc = 0;
		edit(sector_loc(loc));
		break;

	case 'M': /* move */
		user_move();
		comp_move();
		save_game();
		break;

	case 'N': /* give enemy free moves */
		ncycle = get_int("Number of free enemy moves: ", 1, 1000);
		for (i = 0; i < ncycle; i++)
			comp_move();
		save_game();
		break;

	case 'P': /* print a sector */
		c_sector();
		break;

	case '\026': /* some interrupt */
	case 'Q': /* quit */
		c_quit();
		break;

	case 'R': /* restore game */
		term_clear();
		restore_game(); /* XXX only "fails" if it can't find file, so we go on */
		break;

	case 'S': /* save game */
		save_game();
		break;
	
	case 'T': /* trace: toggle save_movie flag */
		save_movie = !save_movie;
		if (save_movie)
			info("Saving movie screens to 'empmovie.dat'.");
		else
			info("No longer saving movie screens.");
		break;

	case 'W': /* watch movie */
		if (resigned || debug)
			replay_movie();
		else
			error("You cannot watch movie until computer resigns.");
		break;
	
	case 'Z': /* print compressed map */
		print_zoom(user_map);
		redraw();
		break;

	case '\014': /* redraw the screen */
		redraw();
		break;

	case '+': /* change debug state */
		e = get_chx();
		if ( e  ==  '+' )
			debug = TRUE;
		else if ( e  ==  '-' )
			debug = FALSE;
		else
			huh();
		break;

	default:
		if (debug)
			c_debug(orders); /* debug */
		else
			huh(); /* illegal command */
		break;
	}
}

/*
 * Give an unowned city (if any) to the computer.  We make
 * a list of unowned cities, choose one at random, and mark
 * it as the computers.
 */

static void
c_give (void)
{
	int unowned[NUM_CITY];
	long i, count;

	count = 0; /* nothing in list yet */
	for (i = 0; i < NUM_CITY; i++) {
		if (city[i].owner == UNOWNED) {
			unowned[count] = i; /* remember this city */
			count += 1;
		}
	}
	if (count == 0) {
		error ("There are no unowned cities.");
		return;
	}
	i = rand_long (count);
	i = unowned[i]; /* get city index */
	city[i].owner = COMP;
	city[i].prod = NOPIECE;
	city[i].work = 0;
	scan (comp_map, city[i].loc);
}

/*
 * Debugging commands should be implemented here.  
 * The order cannot be any legal command.
 */

static void
c_debug (char order)
{
	char e;

	switch (order) {
	case '#' : c_examine (); break;
	case '%' : c_movie (); break;
	
	case '@': /* change trace state */
		e = get_chx();
		if ( e  ==  '+' ) trace_pmap = TRUE;
		else if ( e  ==  '-' ) trace_pmap = FALSE;
		else huh ();
		break;

	case '$': /* change print_debug state */
		e = get_chx();
		if ( e  ==  '+' ) print_debug = TRUE;
		else if ( e  ==  '-' ) print_debug = FALSE;
		else huh ();
		break;

	case '&': /* change print_vmap state */
		print_vmap = get_chx();
		break;

	default: huh (); break;
	}
}

/* The quit command.  Make sure the user really wants to quit. */

static void
c_quit (void)
{
	if (getyn ("QUIT - Are you sure? ")) {
	    emp_end ();
	}
}

/* Read the sector number from the user and print that sector. */

static void
c_sector (void)
{
	int num;

	num = get_int("Sector number? ", 0, NUM_SECTORS-1);
	print_sector_u (num);
}

/*
 * Print the map to a file.  We ask for a filename, attempt to open the
 * file, and if successful, print out the user's information to the file.
 * We print the map sideways to make it easier for the user to print
 * out the map.
 */

static void
c_map (void)
{
	FILE *f;
	int i, j;
	char line[MAP_HEIGHT+2];

	prompt ("Filename? ");
	get_str (jnkbuf, STRSIZE);

	f = fopen (jnkbuf, "w");
	if (f == NULL)
	{
		error ("I can't open that file.");
		return;
	}

	for (i = 0; i < MAP_WIDTH; i++)
	{
		/* for each column */
		for (j = MAP_HEIGHT-1; j >= 0; j--)
		{
			/* for each row */
                        line[MAP_HEIGHT-1-j] = user_map[row_col_loc(j,i)].contents;
		}
		j = MAP_HEIGHT-1;
		while (j >= 0 && line[j] == ' ') /* scan off trailing blanks */
			j -= 1;
			
		line[j+1] = '\n';
		line[j+2] = 0; /* trailing null */
		fputs (line, f);
	}
	fclose (f);
}

/* Allow user to examine the computer's map. */

static void
c_examine (void)
{
	int num;

	num = get_int("Sector number? ", 0, NUM_SECTORS-1);
	print_sector_c (num);
}

/*
 * We give the computer lots of free moves and
 * Print a "zoomed" version of the computer's map.
 */

static void
c_movie (void)
{
	while (1)
	{
		comp_move();
		print_zoom(comp_map);
		save_game();
	}
}

/*This provides a single place for collecting all startup routines */

static void
emp_start (void)
{
	term_init();	/* init tty, and info and status windows */
	map_init();	/* init map window */
	rand_init();	/* init random number generator */
}

/* This provides a single place for collecting all cleanup routines */

static void
emp_end (void)
{
	term_end();
	exit(0);
}
