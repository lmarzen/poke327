#ifndef CONFIG_H
#define CONFIG_H

#include <limits.h>
#include <ncurses.h>

// Controls
#define CTRL_N     key == 'k' || key == '8'
#define CTRL_NE    key == 'u' || key == '9'
#define CTRL_E     key == 'l' || key == '6'
#define CTRL_SE    key == 'n' || key == '3'
#define CTRL_S     key == 'j' || key == '2'
#define CTRL_SW    key == 'b' || key == '1'
#define CTRL_W     key == 'h' || key == '4'
#define CTRL_NW    key == 'y' || key == '7'
#define CTRL_PASS  key == ' ' || key == '5' || key == '.'
#define CTRL_ENTER_BLDG     key == '>'
#define CTRL_EXIT_BLDG      key == '<'
#define CTRL_TNR_LIST_SHOW  key == 't'
#define CTRL_TNR_LIST_HIDE  key == 27 // ESC
#define CTRL_SCROLL_UP      key == KEY_UP
#define CTRL_SCROLL_DOWN    key == KEY_DOWN
#define CTRL_QUIT_GAME      key == 'Q'
#define CTRL_LEAVE_BATTLE   key == 27 // ESC

// World dimensions, world is made up of regions (WORLD_SIZE * WORLD_SIZE)
#define WORLD_SIZE 399

#define FRAMETIME 125000 // in microseconds
#define TICKS_PER_SEC 10
#define FRAMES_PER_SEC (1000000/FRAMETIME)
#define TICKS_PER_FRAME (TICKS_PER_SEC/FRAMES_PER_SEC)

// Printed region dimensions 
#define MAX_ROW 21
#define MAX_COL 80

// The number of biomes that will populate each region
#define MIN_SEEDS_PER_REGION 6 // The first 2 will be grass the next 2 will be clearings after that it is randomized.
#define MAX_SEEDS_PER_REGION 12

// Symbols
#define CHAR_BORDER       '%'
#define CHAR_BOULDER      '%'
#define CHAR_TREE         '^'
#define CHAR_CENTER       'C'
#define CHAR_MART         'M'
#define CHAR_PATH         '#'
#define CHAR_GRASS        ':'
#define CHAR_CLEARING     '.'
#define CHAR_MOUNTAIN     '%'
#define CHAR_FOREST       '^'
#define CHAR_PC           '@'
#define CHAR_HIKER        'h'
#define CHAR_RIVAL        'r'
#define CHAR_PACER        'p'
#define CHAR_WANDERER     'w'
#define CHAR_STATIONARY   's'
#define CHAR_RAND_WALKER  'n'
#define CHAR_UNDEFINED    'U'

// Colors
#define CHAR_COLOR_BORDER       COLOR_WHITE
#define CHAR_COLOR_BOULDER      COLOR_WHITE
#define CHAR_COLOR_TREE         COLOR_GREEN 
#define CHAR_COLOR_CENTER       COLOR_RED
#define CHAR_COLOR_MART         COLOR_BLUE
#define CHAR_COLOR_PATH         COLOR_YELLOW
#define CHAR_COLOR_GRASS        COLOR_GREEN
#define CHAR_COLOR_CLEARING     COLOR_GREEN
#define CHAR_COLOR_MOUNTAIN     COLOR_WHITE
#define CHAR_COLOR_FOREST       COLOR_GREEN
#define CHAR_COLOR_PC           COLOR_RED
#define CHAR_COLOR_HIKER        COLOR_YELLOW 
#define CHAR_COLOR_RIVAL        COLOR_RED
#define CHAR_COLOR_PACER        COLOR_CYAN
#define CHAR_COLOR_WANDERER     COLOR_MAGENTA
#define CHAR_COLOR_STATIONARY   COLOR_CYAN
#define CHAR_COLOR_RAND_WALKER  COLOR_MAGENTA
#define CHAR_COLOR_UNDEFINED    COLOR_RED
#define CHAR_COLOR_BACKGROUND   COLOR_BLACK

// The number of trainers that will populate each region
#define NUM_TRAINERS -1 // -1 is used to indicate random number of trainers
#define MIN_TRAINERS 6
#define MAX_TRAINERS 12

static const int32_t travel_times[11][7] = {
                  /*       PC,   Hiker,   Rival,   Pacer, Wandere, Station,  Walker*/
  /* ter_border   */ {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_boulder  */ {INT_MAX,      10, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_tree     */ {INT_MAX,      10, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_center   */ {     10, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_mart     */ {     10, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_path     */ {     10,      10,      10,      10,      10,      10,      10},
  /* ter_grass    */ {     20,      15,      20,      20,      20,      20,      20},
  /* ter_clearing */ {     10,      10,      10,      10,      10,      10,      10},
  /* ter_mountain */ {INT_MAX,      15, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_forest   */ {INT_MAX,      15, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_mixed    */ {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX}
};

#endif