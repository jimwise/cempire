/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * $Id: empire.h,v 1.35 2003/10/22 22:23:32 jwise Exp $
 */

/*
 * constants
 */

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define NUMTOPS 4	/* number of lines at top of screen for messages */
#define NUMINFO (NUMTOPS - 1)
#define NUMSIDES 6	/* number of lines at side of screen */
#define STRSIZE 80	/* number of characters in a string */

/* Information we maintain about cities. */

#define NUM_CITY 70
#define UNOWNED 0
#define USER 1
#define COMP 2

#define LIST_SIZE 5000	/* max number of pieces on board */

#define INFINITY 1000000 /* a large number */

#define T_UNKNOWN 0
#define T_PATH 1
#define T_LAND 2
#define T_WATER 4
#define T_AIR (T_LAND | T_WATER)

/* Define useful constants for accessing sectors. */

#define SECTOR_ROWS 5		/* number of vertical sectors */
#define SECTOR_COLS 2		/* number of horizontal sectors */
#define NUM_SECTORS (SECTOR_ROWS * SECTOR_COLS) /* total sectors */
#define ROWS_PER_SECTOR ((MAP_HEIGHT+SECTOR_ROWS-1)/SECTOR_ROWS)
#define COLS_PER_SECTOR ((MAP_WIDTH+SECTOR_COLS-1)/SECTOR_COLS)

#define VERSION_STRING "EMPIRE, Version 1.3_ALPHA3, February 1998"

/* directions one can move */
typedef enum
{
	NORTH=0,
	NORTHEAST,
	EAST,
	SOUTHEAST,
	SOUTH,
	SOUTHWEST,
	WEST,
	NORTHWEST
} direction_t;

/* piece types. */
typedef enum
{
	ARMY=0,
	FIGHTER,
	PATROL,
	DESTROYER,
	SUBMARINE,
	TRANSPORT,
	CARRIER,
	BATTLESHIP,
	SATELLITE,
	NOPIECE=255
} piece_type_t;
#define FIRST_OBJECT	ARMY
#define	NUM_OBJECTS	9

/*
 * Types of programmed movement.  Use negative numbers for special
 * functions, use positive numbers to move toward a specific location.
 */
enum
{
        NOFUNC=-1,      /* no programmed function			*/
        RANDOM=-2,      /* move randomly				*/
        SENTRY=-3,      /* sleep					*/
        FILL=-4,        /* fill transport				*/
        LAND=-5,        /* land fighter at city				*/
        EXPLORE=-6,     /* piece explores nearby			*/
        ARMYLOAD=-7,    /* army moves toward and boards a transport	*/
        ARMYATTACK=-8,  /* army looks for city to attack		*/
        TTLOAD=-9,      /* transport moves toward loading armies	*/
        REPAIR=-10,     /* ship moves toward port			*/
        WFTRANSPORT=-11,/* army boards a transport			*/
        MOVE_N=-12,     /* move north					*/
        MOVE_NE=-13,    /* move northeast				*/
        MOVE_E=-14,     /* move east					*/
        MOVE_SE=-15,    /* move southeast				*/
        MOVE_S=-16,     /* move south					*/
        MOVE_SW=-17,    /* move southwest				*/
        MOVE_W=-18,     /* move west					*/
        MOVE_NW=-19     /* move northwest				*/
};
typedef int    function_t;

typedef struct
{
	long		loc;			/* location of city		*/
	unsigned char	owner;			/* UNOWNED, USER, COMP		*/
	function_t	func[NUM_OBJECTS];	/* function for each object	*/
	long		work;			/* units of work performed	*/
	piece_type_t	prod;			/* item being produced		*/
} city_info_t;

/* Information we maintain about each piece. */

typedef struct
{
	/* ptrs for doubly linked list */
	struct piece_info *next;	/* pointer to next in list */
	struct piece_info *prev;	/* pointer to prev in list */
} link_t;

typedef struct piece_info
{
	link_t		piece_link;	/* linked list of pieces of this type	*/
	link_t		loc_link;	/* linked list of pieces at a location	*/
	link_t		cargo_link;	/* linked list of cargo pieces		*/
	unsigned char	owner;		/* owner of piece			*/
	piece_type_t	type;		/* type of piece			*/
	long		loc;		/* location of piece			*/
	function_t	func;		/* programmed type of movement		*/
	int		hits;		/* hits left				*/
	int		moved;		/* moves made				*/
	struct piece_info *ship;	/* pointer to containing ship		*/
	struct piece_info *cargo;	/* pointer to cargo list		*/
	int		count;		/* count of items on board		*/
	int		range;		/* current range (if applicable)	*/
} piece_info_t;

/*
 * We maintain attributes for each piece.  Attributes are currently constant,
 * but the way has been paved to allow user's to change attributes at the
 * beginning of a game.
 */

typedef struct
{
        char sname;			/* eg 'C'					*/
        char name[20];			/* eg "aircraft carrier"			*/
        char nickname[20];		/* eg "carrier"					*/
        char article[20];		/* eg "an aircraft carrier"			*/
        char plural[20];		/* eg "aircraft carriers"			*/
        char terrain[4];		/* terrain piece can pass over eg "."		*/
        unsigned char build_time;	/* time needed to build unit			*/
        unsigned char strength;		/* attack strength				*/
        unsigned char max_hits;		/* number of hits when completely repaired	*/
        unsigned char speed;		/* number of squares moved per turn		*/
        unsigned char capacity;		/* max objects that can be held			*/
        long range;		/* range of piece				*/
} piece_attr_t;

/*
 * There are 3 maps.  'map' describes the world as it actually
 * exists; it tells whether each map cell is land, water or a city;
 * it tells whether or not a square is on the board.
 * 
 * 'user_map' describes the user's view of the world.  'comp_map' describes
 * the computer's view of the world.
 */

#define MAP_WIDTH 100
#define MAP_HEIGHT 60
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)

typedef struct
{
        /* a cell of the actual map */
        char contents;		/* '+', '.', or '*'			*/
        unsigned char on_board;		/* TRUE iff on the board		*/
        city_info_t *cityp;	/* ptr to city at this location		*/
        piece_info_t *objp;	/* list of objects at this location	*/
} real_map_t;

typedef struct
{
        /* a cell of one player's world view */
        unsigned char contents;		/* '+', '.', '*', 'A', 'a', etc	*/
        long seen;		/* date when last updated	*/
} view_map_t;

/* Define information we maintain for a pathmap. */

typedef struct
{
        int cost;	/* total cost to get here		*/
        int inc_cost;	/* incremental cost to get here		*/
        char terrain;	/* T_LAND, T_WATER, T_UNKNOWN, T_PATH	*/
} path_map_t;

/* A record for counts we obtain when scanning a continent. */

typedef struct {
        int user_cities;		/* number of user cities on continent	*/
        int user_objects[NUM_OBJECTS];
        int comp_cities;
        int comp_objects[NUM_OBJECTS];
        int size;			/* size of continent in cells		*/
        int unowned_cities;		/* number of unowned cities		*/
        int unexplored;			/* unexplored territory			*/
} scan_counts_t;

/* Information we need for finding a path for moving a piece. */

typedef struct {
        unsigned char city_owner;		/* char that represents home city	*/
        const char *objectives;		/* list of objectives			*/
        int weights[11];		/* weight of each objective		*/
} move_info_t;

/* special cost for city building a tt */
#define W_TT_BUILD -1

/* List of cells in the perimeter of our searching for a path. */

typedef struct {
        long len;		/* number of items in list	*/
        long list[MAP_SIZE];	/* list of locations		*/
} perimeter_t;

/* function macros related to above structures */

/* Index to list of function names. */
#define FUNCI(x) (-(x)-1)

/* Macro to convert a movement function into a direction. */
#define MOVE_DIR(a) (-(a)+MOVE_N)

/* Macros to link and unlink an object from a doubly linked list. */

#define LINK(head,obj,list) \
{ \
	obj->list.prev = NULL; \
	obj->list.next = head; \
	if (head) head->list.prev = obj; \
	head = obj; \
}

#define UNLINK(head,obj,list) \
{ \
	if (obj->list.next) \
		obj->list.next->list.prev = obj->list.prev; \
        if (obj->list.prev) \
		obj->list.prev->list.next = obj->list.next; \
        else head = obj->list.next; \
	obj->list.next = NULL; /* encourage mem faults in buggy code */ \
	obj->list.prev = NULL; \
}

/* macros to set map and list of an object */
#define MAP(owner) ((owner) == USER ? user_map : comp_map)
#define LIST(owner) ((owner) == USER ? user_obj : comp_obj)

/* macro to step through adjacent cells */
#define FOR_ADJ(loc,new_loc,i) for (i=0; (i<8 ? new_loc=loc+dir_offset[i],1 : 0); i++)
#define FOR_ADJ_ON(loc,new_loc,i) FOR_ADJ(loc,new_loc,i) if (map[new_loc].on_board)

#ifdef USE_COLOR
short color_of(const view_map_t[], const long);
#endif
