/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * $Id: data.c,v 1.13 2003/10/23 16:31:16 jwise Exp $
 */

/*
 * Static data.
 * 
 * One of our hopes is that changing the types of pieces that
 * exist in the game is mostly a matter of modifying this file.
 * However, see also the help routine, empire.h, and empire.doc.
 */

#include "empire.h"

/*
 * Piece attributes.  Notice that the Transport is allowed only one hit.
 * In the previous version of this game, the user could easily win simply
 * by building armies and troop transports.  We attempt to alleviate this
 * problem by making transports far more fragile.  We have also increased
 * the range of a fighter from 20 to 30 so that fighters will be somewhat
 * more useful.
 */

const piece_attr_t piece_attr[NUM_OBJECTS] = {
	{'A', 		/* character for printing piece */
	 "army", 	/* name of piece		*/ 
	 "army", 	/* nickname			*/
	 "an army", 	/* name with preceding article	*/
	 "armies", 	/* plural			*/
	 "+",		/* terrain			*/
	 5,		/* units to build		*/
	 1,		/* strength			*/
	 1,		/* max hits			*/
	 1,		/* movement			*/
	 0,		/* capacity			*/
	 INFINITY},	/* range			*/

	/*
	 * For fighters, the range is set to an even multiple of the speed.
	 * This allows user to move fighter, say, two turns out and two
	 * turns back.
	 */
	 
	{'F', "fighter", "fighter", "a fighter", "fighters",
		".+", 10, 1,  1, 8, 0, 32},

	{'P', "patrol boat", "patrol", "a patrol boat", "patrol boats",
		".",  15, 1,  1, 4, 0, INFINITY},
		
	{'D', "destroyer", "destroyer", "a destroyer", "destroyers",
		".",  20, 1,  3, 2, 0, INFINITY},

	{'S', "submarine", "submarine", "a submarine", "submarines",
		".",  20, 3,  2, 2, 0, INFINITY},

	{'T', "troop transport", "transport", "a troop transport", "troop transports",
		".",  30, 1,  1, 2, 6, INFINITY},

	{'C', "aircraft carrier", "carrier", "an aircraft carrier", "aircraft carriers",
		".",  30, 1,  8, 2, 8, INFINITY},

	{'B', "battleship", "battleship", "a battleship", "battleships",
		".",  40, 2, 10, 2, 0, INFINITY},
		
	{'Z', "satellite", "satellite", "a satellite", "satellites",
		".+", 50, 0, 1, 10, 0, 500}
};

/* Direction offsets. */

const int dir_offset [] = {
		-MAP_WIDTH,	/* north	*/
		-MAP_WIDTH+1,	/* northeast	*/
		1,		/* east		*/
		MAP_WIDTH+1,	/* southeast	*/
		MAP_WIDTH,	/* south	*/
		MAP_WIDTH-1,	/* southwest	*/
		-1,		/* west		*/
		-MAP_WIDTH-1	/* northwest	*/
};

/* Names of movement functions. */

const char *func_name[] = {
		"none", "random", "sentry", "fill", "land",
		"explore", "load", "attack", "load", "repair",
		"transport",
		"W", "E", "D", "C", "X", "Z", "A", "Q"
};

/* The order in which pieces should be moved. */
const int move_order[NUM_OBJECTS] = {SATELLITE, TRANSPORT, CARRIER, BATTLESHIP, PATROL, SUBMARINE, DESTROYER, ARMY, FIGHTER};

/* types of pieces, in declared order */
const char type_chars[] = "AFPDSTCBZ";

/* types of cities */
const char city_char[] = {'*', 'O', 'X'};

/* Lists of attackable objects if object is adjacent to moving piece. */
const char tt_attack[] = "T";
const char army_attack[] = "O*TACFBSDP";
const char fighter_attack[] = "TCFBSDPA";
const char ship_attack[] = "TCBSDP";

/* Define various types of objectives */
const move_info_t tt_explore = {		/* water objectives */
	COMP,	/* home city	*/
	" ",	/* objectives	*/
	{1}	/* weights	*/
};
const move_info_t tt_load = {COMP, "$", {1}};	/* land objectives */

/*
 * Rationale for 'tt_unload':
 * 
 *      Any continent with four or more cities is extremely attractive,
 * and we should grab it quickly.  A continent with three cities is
 * fairly attractive, but we are willing to go slightly out of our
 * way to find a better continent.  Similarily for two cities on a
 * continent.  At one city on a continent, things are looking fairly
 * unattractive, and we are willing to go quite a bit out of our way
 * to find a better continent.
 * 
 *      Cities marked with a '0' are on continents where we already
 * have cities, and these cities will likely fall to our armies anyway,
 * so we don't need to dump armies on them unless everything else is
 * real far away.  We would prefer to use unloading transports for
 * taking cities instead of exploring, but we are willing to explore
 * if interesting cities are too far away.
 * 
 *      It has been suggested that continents containing one city
 * are not interesting.  Unfortunately, most of the time what the
 * computer sees is a single city on a continent next to lots of
 * unexplored territory.  So it dumps an army on the continent to
 * explore the continent and the army ends up attacking the city
 * anyway.  So, we have decided we might as well send the tt to
 * the city in the first place and increase the speed with which
 * the computer unloads its tts.
 */

const move_info_t tt_unload     = {COMP, "9876543210 ", {1, 1, 1, 1, 1, 1, 11, 21, 41, 101, 61}};

/*
 * '$' represents loading tt must be first
 * 'x' represents tt producing city
 * '0' represents explorable territory
 */

const move_info_t army_fight	= {COMP, "O*TA ", {1, 1, 1, 1, 11}};		/* land objectives */
const move_info_t army_load	= {COMP, "$x", {1, W_TT_BUILD}};		/* water objectives */
const move_info_t fighter_fight = {COMP, "TCFBSDPA ", {1, 1, 5, 5, 5, 5, 5, 5, 9}};
const move_info_t ship_fight    = {COMP, "TCBSDP ", {1, 1, 3, 3, 3, 3, 21}};
const move_info_t ship_repair   = {COMP, "X", {1}};
const move_info_t user_army     = {USER, " ", {1}};
const move_info_t user_army_attack = {USER, "*Xa ", {1, 1, 1, 12}};
const move_info_t user_fighter	= {USER, " ", {1}};
const move_info_t user_ship	= {USER, " ", {1}};
const move_info_t user_ship_repair = {USER, "O", {1}};

/*
 * Various help texts.
 */

const char *help_cmd[] = {
	"COMMAND MODE",
	"a - enter Automove mode",
	"c - give City to computer",
	"d - print game Date (round)",
	"e - Examine enemy map",
	"f - print map to File",
	"g - Give move to computer",
	"h - display this Help text",
	"j - enter edit mode",
	"m - Move",
	"n - give N moves to computer",
	"p - Print a sector",
	"q - Quit game",
	"r - Restore game",
	"s - Save game",
	"t - save movie in empmovie.dat",
	"w - Watch movie",
	"z - display Zoomed out map",
	"<ctrl-L> - redraw screen"
};
const int cmd_lines = 19;

const char *help_user[] = {
	"USER MODE",
	"QWE",
	"A D - movement directions",
	"ZXC",
	"<space>:           skip",
	"b - change city production",
	"f - set func to Fill",
	"g - set func to explore",
	"h - display this Help text",
	"i <dir> - set func to dir",
	"j - enter edit mode",
	"k - set func to awake",
	"l - set func to Land",
	"o - get Out of automove mode",
	"p - redraw screen",
	"r - set func to Random",
	"s - set func to Sentry",
	"u - set func to repair",
	"v <piece> <func> - set city func",
	"y - set func to attack",
	"<ctrl-L> - redraw screen",
	"= - describe piece"
};
const int user_lines = 22;
	
const char *help_edit[] = {
	"EDIT MODE",
	"QWE",
	"A D - movement directions",
	"ZXC",
	"b - change city production",
	"f - set func to Fill",
	"g - set func to explore",
	"h - display this Help text",
	"i <dir> - set func to dir",
	"k - set func to awake",
	"l - set func to Land",
	"m - Mark piece",
	"n - set dest for marked piece",
	"o - get Out of automove mode",
	"p - redraw screen",
	"r - set func to Random",
	"s - set func to Sentry",
	"u - set func to repair",
	"v <piece> <func> - set city func",
	"y - set func to attack",
	"<ctrl-L> - redraw screen",
	"= - describe piece"
};
const int edit_lines = 22;

/* user-supplied parameters */
int SMOOTH;		/* number of times to smooth map */
int WATER_RATIO;	/* percentage of map that is water */
int MIN_CITY_DIST;	/* cities must be at least this far apart */
int save_interval;	/* turns between autosaves */
int traditional = 0;	/* use `traditional' UI */
int color = 1;		/* use color if available */

real_map_t map[MAP_SIZE];	/* the way the world really looks */
view_map_t comp_map[MAP_SIZE];	/* computer's view of the world */
view_map_t user_map[MAP_SIZE];	/* user's view of the world */

city_info_t city[NUM_CITY]; /* city information */

/*
 * There is one array to hold all allocated objects no matter who
 * owns them.  Objects are allocated from the array and placed on
 * a list corresponding to the type of object and its owner.
 */

piece_info_t *free_list;		/* index to free items in object list */
piece_info_t *user_obj[NUM_OBJECTS];	/* indices to user lists */
piece_info_t *comp_obj[NUM_OBJECTS];	/* indices to computer lists */
piece_info_t object[LIST_SIZE];		/* object list */

/* Display information. */
int lines;	/* lines on screen */
int cols;	/* columns on screen */

/* miscellaneous */
long date;		/* number of game turns played */
char automove;		/* TRUE iff user is in automove mode */
char resigned;		/* TRUE iff computer resigned */
char debug;		/* TRUE iff in debugging mode */
char print_debug;	/* TRUE iff we print debugging stuff */
char print_vmap;	/* TRUE iff we print view maps */
char trace_pmap;	/* TRUE if we are tracing pmaps */
int win;		/* set when game is over */
char jnkbuf[STRSIZE];	/* general purpose temporary buffer -- this should really just go away */
char save_movie;	/* TRUE iff we should save movie screens */
int user_score;		/* "score" for user and computer */
int comp_score;
