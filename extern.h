/*
 *    Copyright (C) 1987, 1988 Chuck Simmons
 * 
 * See the file COPYING, distributed with empire, for restriction
 * and warranty information.
 *
 * Portions of this file Copyright (C) 1998 Jim Wise
 *
 * $Id: extern.h,v 1.64 1998/08/08 17:21:28 jwise Exp $
 */

/*
extern.h -- define global non-constant storage.
*/

/* user-supplied parameters */
extern int SMOOTH;        /* number of times to smooth map */
extern int WATER_RATIO;   /* percentage of map that is water */
extern int MIN_CITY_DIST; /* cities must be at least this far apart */
extern int delay_time;
extern int save_interval; /* turns between autosaves */

extern real_map_t map[MAP_SIZE]; /* the way the world really looks */
extern view_map_t comp_map[MAP_SIZE]; /* computer's view of the world */
extern view_map_t user_map[MAP_SIZE]; /* user's view of the world */

extern city_info_t city[NUM_CITY]; /* city information */

/*
There is one array to hold all allocated objects no matter who
owns them.  Objects are allocated from the array and placed on
a list corresponding to the type of object and its owner.
*/

extern piece_info_t *free_list; /* index to free items in object list */
extern piece_info_t *user_obj[NUM_OBJECTS]; /* indices to user lists */
extern piece_info_t *comp_obj[NUM_OBJECTS]; /* indices to computer lists */
extern piece_info_t object[LIST_SIZE]; /* object list */

/* Display information. */
extern int lines; /* lines on screen */
extern int cols; /* columns on screen */

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
extern char city_char[];

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
extern long date; /* number of game turns played */
extern char automove; /* TRUE iff user is in automove mode */
extern char resigned; /* TRUE iff computer resigned */
extern char debug; /* TRUE iff in debugging mode */
extern char print_debug; /* TRUE iff we print debugging stuff */
extern char print_vmap; /* TRUE iff we print view maps */
extern char trace_pmap; /* TRUE if we are tracing pmaps */
extern int win; /* set when game is over */
extern char jnkbuf[STRSIZE]; /* general purpose temporary buffer */
extern char save_movie; /* TRUE iff we should save movie screens */
extern int user_score; /* "score" for user and computer */
extern int comp_score;

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
	
#define panic(why)      emp_panic(__FILE__, __LINE__, (why))

/* global routines */
void	attack (piece_info_t *, long);	/* attack.c */
void	comp_move (int);	/* compmove.c */
void    edit(long);             /* edit.c */
void    empire (void);          /* empire.c */
void	user_move (void);	/* usermove.c */

/* display routines (display.c) */
void	map_init (void);
int	cur_sector (void);
void	display_loc (int, view_map_t[], long);
void	display_locx (int, view_map_t[], long);
void    display_score (void);
void    help (char **, int);
void	kill_display (void);
int	move_cursor (long *, int);
void    print_movie_screen(char *);
void	print_pzoom (char *, path_map_t *, view_map_t *);
void	print_sector (char, view_map_t[], int);
void	print_zoom (view_map_t *);
void    sector_change (void);

/* edit routines (edit.c) */
void    e_city_attack (city_info_t *, int);
void	e_city_explore (city_info_t *, int);
void    e_city_fill (city_info_t *, int);
void    e_city_random (city_info_t *, int);
void    e_city_repair (city_info_t *, int);
void    e_city_stasis (city_info_t *, int);
void    e_city_wake (city_info_t *, int);

/* game routines (game.c) */
void	init_game (void);
void	replay_movie (void);
int	restore_game (void);
void	save_game (void);
void	save_movie_screen (void);
int	select_cities (void);

/* map routines (map.c)*/
int	rmap_shore (long);
int	vmap_at_sea (view_map_t *, long);
void	vmap_cont (int *, view_map_t *, long, char);
scan_counts_t	vmap_cont_scan (int *, view_map_t *);
long	vmap_find_aobj (path_map_t[], view_map_t *, long, move_info_t *);
long	vmap_find_dest (path_map_t[], view_map_t[], long, long, int, int);
long	vmap_find_dir (path_map_t[], view_map_t *, long, char *, char *);
long	vmap_find_lobj (path_map_t[], view_map_t *, long, move_info_t *);
long	vmap_find_lwobj (path_map_t[], view_map_t *, long, move_info_t *, int);
long	vmap_find_wobj (path_map_t[], view_map_t *, long, move_info_t *);
long	vmap_find_wlobj (path_map_t[], view_map_t *, long, move_info_t *);
void	vmap_mark_adjacent (path_map_t[], long);
void	vmap_mark_near_path (path_map_t[], long);
void	vmap_mark_path (path_map_t *, view_map_t *, long);
void	vmap_mark_up_cont (int *, view_map_t *, long, char);
void	vmap_prune_explore_locs (view_map_t *);

/* math routines (math.c) */
long	dist (long, long);
long	rand_long (long);
int	isqrt (int);
void	rand_init (void);

/* object routines (object.c) */
void	describe_obj (piece_info_t *);
void	disembark (piece_info_t *);
void	embark (piece_info_t *, piece_info_t *);
city_info_t     *find_city (long);
int	find_nearest_city ( long, int, long *);
piece_info_t	*find_nfull (piece_type_t, long);
piece_info_t	*find_obj (piece_type_t, long);
piece_info_t	*find_obj_at_loc (long);
long	find_transport (int, long);
int	get_piece_name (void);
int	good_loc (piece_info_t *, long);
void	kill_city (city_info_t *);
void	kill_obj (piece_info_t *, long);
void	move_obj (piece_info_t *, long);
void	move_sat (piece_info_t *);
int	obj_capacity (piece_info_t *);
int	obj_moves (piece_info_t *);
void	produce (city_info_t *);
void	scan (view_map_t[], long);
void	set_prod (city_info_t *);

/* terminal routines (term.c) */
void	alert (void);
void	error (const char *, ...);
char    get_chx (void);
char    get_cq (void);
int	get_int (char *, int, int);
int     getyn (const char *);
void    get_str (char *, const int);
void	huh (void);
void	info (const char *, ...);
void    redraw (void);
void    term_clear (void);
void	term_end (void);
void    term_init (void);
void	prompt (const char *, ...);

/* utility routines (util.c) */
void    check (void);
void	emp_panic (const char *file, const int line, const char *);
