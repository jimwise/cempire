/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: extern.h,v 1.8 1998/02/25 01:47:47 jim Exp $
 */

/*
extern.h -- define global non-constant storage.
*/

/* user-supplied parameters */
int SMOOTH;        /* number of times to smooth map */
int WATER_RATIO;   /* percentage of map that is water */
int MIN_CITY_DIST; /* cities must be at least this far apart */
int delay_time;
int save_interval; /* turns between autosaves */

real_map_t map[MAP_SIZE]; /* the way the world really looks */
view_map_t comp_map[MAP_SIZE]; /* computer's view of the world */
view_map_t user_map[MAP_SIZE]; /* user's view of the world */

city_info_t city[NUM_CITY]; /* city information */

/*
There is one array to hold all allocated objects no matter who
owns them.  Objects are allocated from the array and placed on
a list corresponding to the type of object and its owner.
*/

piece_info_t *free_list; /* index to free items in object list */
piece_info_t *user_obj[NUM_OBJECTS]; /* indices to user lists */
piece_info_t *comp_obj[NUM_OBJECTS]; /* indices to computer lists */
piece_info_t object[LIST_SIZE]; /* object list */

/* Display information. */
int lines; /* lines on screen */
int cols; /* columns on screen */

/* constant data */
extern piece_attr_t piece_attr[];
extern int dir_offset[];
extern char *func_name[];
extern int move_order[];
extern char type_chars[];
extern char tt_attack[];
extern char army_attack[];
extern char fighter_attack[];
extern char ship_attack[];

extern move_info_t tt_load;
extern move_info_t tt_explore;
extern move_info_t tt_unload;
extern move_info_t army_fight;
extern move_info_t army_load;
extern move_info_t fighter_fight;
extern move_info_t ship_fight;
extern move_info_t ship_repair;
extern move_info_t user_army;
extern move_info_t user_army_attack;
extern move_info_t user_fighter;
extern move_info_t user_ship;
extern move_info_t user_ship_repair;

extern char *help_cmd[];
extern char *help_edit[];
extern char *help_user[];
extern int cmd_lines;
extern int edit_lines;
extern int user_lines;

/* miscellaneous */
long date; /* number of game turns played */
char automove; /* TRUE iff user is in automove mode */
char resigned; /* TRUE iff computer resigned */
char debug; /* TRUE iff in debugging mode */
char print_debug; /* TRUE iff we print debugging stuff */
char print_vmap; /* TRUE iff we print view maps */
char trace_pmap; /* TRUE if we are tracing pmaps */
int win; /* set when game is over */
char jnkbuf[STRSIZE]; /* general purpose temporary buffer */
char save_movie; /* TRUE iff we should save movie screens */
int user_score; /* "score" for user and computer */
int comp_score;

/* Screen updating macros */
#define display_loc_u(loc) display_loc(USER,user_map,loc)
#define display_loc_c(loc) display_loc(COMP,comp_map,loc)
#define print_sector_u(sector) print_sector(USER,user_map,sector)
#define print_sector_c(sector) print_sector(COMP,comp_map,sector)
#define loc_row(loc) ((loc)/MAP_WIDTH)
#define loc_col(loc) ((loc)%MAP_WIDTH)
#define row_col_loc(row,col) ((long)((row)*MAP_WIDTH + (col)))
#define sector_row(sector) ((sector)%SECTOR_ROWS)
#define sector_col(sector) ((sector)/SECTOR_ROWS)
#define row_col_sector(row,col) ((int)((col)*SECTOR_ROWS+(row)))

#define loc_sector(loc) \
	row_col_sector(loc_row(loc)/ROWS_PER_SECTOR, \
                       loc_col(loc)/COLS_PER_SECTOR)
		       
#define sector_loc(sector) row_col_loc( \
		sector_row(sector)*ROWS_PER_SECTOR+ROWS_PER_SECTOR/2, \
		sector_col(sector)*COLS_PER_SECTOR+COLS_PER_SECTOR/2)
		
/* global routines */
void	empire (void);		/* empire.c */
void	attack (piece_info_t *, long);	/* attack.c */
void	comp_move (int);	/* compmove.c */
void	user_move (void);	/* usermove.c */
void	edit(long);		/* edit.c */

#if 0
/* map routines */
void vmap_cont();
void rmap_cont();
void vmap_mark_up_cont();
scan_counts_t vmap_cont_scan();
scan_counts_t rmap_cont_scan();
int map_cont_edge();
long vmap_find_aobj();
long vmap_find_wobj();
long vmap_find_lobj();
long vmap_find_lwobj();
long vmap_find_wlobj();
long vmap_find_dest();
void vmap_prune_explore_locs();
void vmap_mark_path();
void vmap_mark_adjacent();
void vmap_mark_near_path();
long vmap_find_dir();
int vmap_count_adjacent();
int vmap_shore();
int rmap_shore();
int vmap_at_sea();
int rmap_at_sea();

void kill_display(); /* display routines */
int cur_sector();
long cur_cursor();
void display_locx();
void print_sector();
void print_zoom();
void print_pzoom();
void print_xzoom();
void display_score();
#ifdef A_COLOR
void init_colors();
#endif /* A_COLOR */

void init_game(); /* game routines */
void save_game();
int restore_game();
void save_movie_screen();
void replay_movie();

void get_str(); /* input routines */
void get_strq();
int getint();
char get_c();
char get_cq();
int getyn();

void rndini(); /* math routines */
long irand();
int dist();
int isqrt();

/* object routines */
int find_nearest_city();
piece_info_t *find_nfull();
long find_transport();
piece_info_t *find_obj_at_loc();
int obj_moves();
int obj_capacity();
void kill_obj();
void kill_city();
void produce();
void move_obj();
void move_sat();
int good_loc();
void embark();
void disembark();
void scan();
void scan_sat();

/* terminal routines */
void pdebug();
void clreol();
void topmsg();
void comment();
void extra();
void set_need_delay();

/* from util.c but not used elsewhere */
void close_disp();
void tupper();
#endif

/* display routines (display.c) */
void	display_loc (int, view_map_t[], long);
int	move_cursor (long *, int);
void    sector_change (void);

/* input routines (input.c) */
char	get_chx (void);

/* object routines (object.c) */
void describe_obj (piece_info_t *);
city_info_t     *find_city (long);
piece_info_t	*find_obj (int, long);
piece_info_t	*find_obj_at_loc (long);
int	get_piece_name (void);
void	set_prod (city_info_t *);

/* terminal routines (term.c) */
void	error (char *, ...);
int	get_range (char *, int, int);
void	help (char **, int);
void	huh (void);
void	info (char *, char *, char *);
void	prompt (char *, ...);
void	topini (void);

/* utility routines (util.c) */
void	ttinit (void);
void	redraw (void);
void	clear_screen (void);
void	delay (void);
void	pos_str (int, int, char *, ...);
void	addprintf (char *, ...);
void	assert (char *, char *, int);
void	empend (void);
void	ver (void);
char	upper (char);
void	check (void);
