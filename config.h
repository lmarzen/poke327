#ifndef CONFIG_H
#define CONFIG_H

#include <limits.h>

// World dimensions, world is made up of regions (WORLD_SIZE * WORLD_SIZE)
#define WORLD_SIZE 399

#define FRAMETIME 250000 // in microseconds
#define TICKS_PER_SEC 10
#define FRAMES_PER_SEC (1000000/FRAMETIME)
#define TICKS_PER_FRAME (TICKS_PER_SEC/FRAMES_PER_SEC)

// Printed region dimensions 
#define MAX_ROW 21
#define MAX_COL 80

// The number of biomes that will populate each region
#define MIN_SEEDS_PER_REGION 6 // The first 2 will be grass the next 2 will be clearings after that it is randomized.
#define MAX_SEEDS_PER_REGION 12

#define CHAR_BORDER       '%';
#define CHAR_BOULDER      '%';
#define CHAR_TREE         '^';
#define CHAR_CENTER       'C';
#define CHAR_MART         'M';
#define CHAR_PATH         '#';
#define CHAR_GRASS        ':';
#define CHAR_CLEARING     '.';
#define CHAR_MOUNTAIN     '%';
#define CHAR_FOREST       '^';
#define CHAR_PC           '@';
#define CHAR_HIKER        'h';
#define CHAR_RIVAL        'r';
#define CHAR_PACER        'p';
#define CHAR_WANDERER     'w';
#define CHAR_STATIONARY   's';
#define CHAR_RAND_WALKER  'n';
#define CHAR_UNDEFINED    'U';

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