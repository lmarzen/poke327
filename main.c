#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "heap.h"

// Printed region dimensions 
#define MAX_ROW 21
#define MAX_COL 80

// World dimensions, world is made up of regions (WORLD_SIZE * WORLD_SIZE)
#define WORLD_SIZE 399

// The number of biomes that will populate each region
#define MIN_SEEDS_PER_REGION 6 // The first 2 will be grass the next 2 will be clearings after that it is randomized.
#define MAX_SEEDS_PER_REGION 12

// The number of trainers that will populate each region
#define NUM_TRAINERS -1 // -1 is used to indicate random number of trainers
#define MIN_TRAINERS 6
#define MAX_TRAINERS 12
#define FRAMETIME 250000 // in microseconds
#define TICKS_PER_SEC 10
#define FRAMES_PER_SEC (1000000/FRAMETIME)
#define TICKS_PER_FRAME (TICKS_PER_SEC/FRAMES_PER_SEC)

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

typedef enum terrain {
  ter_border,
  ter_boulder,
  ter_tree,
  ter_center,
  ter_mart,
  ter_path,
  ter_grass,
  ter_clearing,
  ter_mountain,
  ter_forest,
  ter_mixed
} terrain_t;

typedef enum trainer {
  tnr_pc,
  tnr_hiker,
  tnr_rival,
  tnr_pacer,
  tnr_wanderer,
  tnr_stationary,
  tnr_rand_walker
} trainer_t;

typedef enum direction {
  dir_n,
  dir_ne,
  dir_e,
  dir_se,
  dir_s,
  dir_sw,
  dir_w,
  dir_nw
} direction_t;

typedef struct tile {
  terrain_t ter;
} tile_t;

typedef struct seed {
  int32_t i, j;
  terrain_t ter;
} seed_t;

typedef struct pos {
  int32_t i, j;
} pos_t;

typedef struct character {
  trainer_t tnr; 
  int32_t pos_i, pos_j;
  heap_node_t *hn;
  int32_t movetime;
  direction_t dir;

} character_t;

typedef struct region {
  tile_t tile_arr[MAX_ROW][MAX_COL];
  int32_t N_exit_j, E_exit_i, S_exit_j, W_exit_i;
  int32_t num_npc;
  character_t *npc_arr; // pointer to an array that will be dynamically allocated
} region_t;

typedef struct path {
  heap_node_t *hn;
  int32_t pos_i, pos_j;
  int32_t from_i, from_j;
  int32_t cost;
} path_t;

// Global variables
// 2D array of pointers, each pointer points to one of the regions the world
region_t *region_ptr[WORLD_SIZE][WORLD_SIZE] = {NULL};
int32_t dist_map_hiker[MAX_ROW][MAX_COL];
int32_t dist_map_rival[MAX_ROW][MAX_COL];
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

static const int32_t dir_offsets[8][2] = {
            /* { i, j} */
  /* dir_n  */ {-1, 0},
  /* dir_ne */ {-1, 1},
  /* dir_e  */ { 0, 1},
  /* dir_se */ { 1, 1},
  /* dir_s  */ { 1, 0},
  /* dir_sw */ { 1,-1},
  /* dir_w  */ { 0,-1},
  /* dir_nw */ {-1,-1}
};

void usage(const char *argv0) {
  fprintf(stderr, "Usage: %s [--numtrainers|--seed] <int>\n", argv0);
  exit(-1);
}

/*
 * returns the distance between 2 points
 */
double dist(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
   return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

/*
 * returns the manhatten distance between 2 points
 */
int32_t m_dist(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
   return abs(x2 - x1) + abs(y2 - y1);
}

/*
 * Will decide if an event occurs given its probability
 * returns 1 if an event should happen, otherwise 0.
 */
int32_t rand_outcome(double probability) {
   return rand() < probability * ((double)RAND_MAX + 1.0);
}

/*
 * Print a region
 */
void print_region (region_t *region, character_t *pc) {
  char char_arr[MAX_ROW][MAX_COL];
  
  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      terrain_t ter = region->tile_arr[i][j].ter;
      switch (ter) {
        case ter_border:
          char_arr[i][j] = CHAR_BORDER;
          break;
        case ter_clearing:
          char_arr[i][j] = CHAR_CLEARING;
          break;
        case ter_grass:
          char_arr[i][j] = CHAR_GRASS;
          break;
        case ter_boulder:
          char_arr[i][j] = CHAR_BOULDER;
          break;
        case ter_tree:
          char_arr[i][j] = CHAR_TREE;
          break;
        case ter_mountain:
          char_arr[i][j] = CHAR_MOUNTAIN;
          break;
        case ter_forest:
          char_arr[i][j] = CHAR_FOREST;
          break;
        case ter_path:
          char_arr[i][j] = CHAR_PATH;
          break;
        case ter_center:
          char_arr[i][j] = CHAR_CENTER;
          break;
        case ter_mart:
          char_arr[i][j] = CHAR_MART;
          break;
        default:
          char_arr[i][j] = CHAR_UNDEFINED;
      }
    }
  }

  character_t *p = (region->npc_arr);
  for (int32_t k = 0; k < region->num_npc; k++, p++) {
    switch (p->tnr) {
      case tnr_hiker:
        char_arr[p->pos_i][p->pos_j] = CHAR_HIKER;
        break;
      case tnr_rival:
        char_arr[p->pos_i][p->pos_j]  = CHAR_RIVAL;
        break;
      case tnr_pacer:
        char_arr[p->pos_i][p->pos_j]  = CHAR_PACER;
        break;
      case tnr_wanderer:
        char_arr[p->pos_i][p->pos_j]  = CHAR_WANDERER;
        break;
      case tnr_stationary:
        char_arr[p->pos_i][p->pos_j]  = CHAR_STATIONARY;
        break;
      case tnr_rand_walker:
        char_arr[p->pos_i][p->pos_j]  = CHAR_RAND_WALKER;
        break;
      default:
        char_arr[p->pos_i][p->pos_j]  = CHAR_UNDEFINED;
    }
  }
  
  char_arr[pc->pos_i][pc->pos_j] = CHAR_PC;

  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      putchar(char_arr[i][j]);
    }
    putchar('\n');
  }
}

/*
 * Initialize a region
 *
 * Path exit points will be generated at the locations specified.
 * For random path exit point specify -1
 *
 * place_center and place_mart parameters specify whether or not to place a 
 * poke center and/or a poke mart in a region. 0 to not place a building, 
 * 1 to place a building
 */
void init_region (region_t *region, 
                  int32_t N_exit_j, int32_t E_exit_i,
                  int32_t S_exit_j, int32_t W_exit_i,
                  int32_t place_center, int32_t place_mart,
                  int32_t numtrainers_opt) {
  int32_t randy; // note... int num = (rand() % (upper - lower + 1)) + lower;

  // create a random number of random seeds
  int32_t num_seeds = (rand() % (MAX_SEEDS_PER_REGION - MIN_SEEDS_PER_REGION + 1)) 
                      + MIN_SEEDS_PER_REGION;

  // allocate memory for seeds, each seed has x and y
  seed_t (*seed_arr)[num_seeds] = malloc(sizeof(*seed_arr) * num_seeds);
  
  // initialize each seed with a random set of cordinates
  // at least 2 grass and 2 clearings seeds. (req)
  seed_arr[0]->i = rand() % MAX_ROW;
  seed_arr[0]->j = rand() % MAX_COL;
  seed_arr[0]->ter = ter_clearing;
  seed_arr[1]->i = rand() % MAX_ROW;
  seed_arr[1]->j = rand() % MAX_COL;
  seed_arr[1]->ter = ter_clearing;
  seed_arr[2]->i = rand() % MAX_ROW;
  seed_arr[2]->j = rand() % MAX_COL;
  seed_arr[2]->ter = ter_grass; 
  seed_arr[3]->i = rand() % MAX_ROW;
  seed_arr[3]->j = rand() % MAX_COL;
  seed_arr[3]->ter = ter_grass;

  //  remaining seeds get random terrain type
  for (int32_t i = 4; i < num_seeds; i++) {
    randy = rand() % 100; 
    seed_arr[i]->i = rand() % MAX_ROW;
    seed_arr[i]->j = rand() % MAX_COL;
    if (randy >= 0 && randy < 25) {
      seed_arr[i]->ter = ter_grass;
    } else if (randy >= 25 && randy < 50) {
      seed_arr[i]->ter = ter_clearing;
    } else if (randy >= 50 && randy < 70) {
      seed_arr[i]->ter = ter_mixed;
    } else if (randy >= 70 && randy < 85) {
      seed_arr[i]->ter = ter_mountain;
    } else if (randy >= 85 && randy < 100) {
      seed_arr[i]->ter = ter_forest;
    }
  }

  // populate each tile by assigning it the terrain type of the closest seed
  // does not calculate for the outer most boarder because this space will 
  // be boulders or path.
  for (int32_t i = 0; i < MAX_ROW ; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      if (i == 0 || i == MAX_ROW - 1 || j == 0 || j == MAX_COL - 1) {
        region->tile_arr[i][j].ter = ter_border;
      } else {
        seed_t *closest_seed = seed_arr[0];
        double closest_dist = dist(seed_arr[0]->j, seed_arr[0]->i, j, i);

        for (int32_t k = 1; k < num_seeds; k++) {
          double temp_dist = dist(seed_arr[k]->j, seed_arr[k]->i, j, i);
          if (temp_dist < closest_dist) {
            closest_seed = seed_arr[k];
            closest_dist = temp_dist;
          }
        }

        region->tile_arr[i][j].ter = closest_seed->ter;
        if (region->tile_arr[i][j].ter == ter_mixed) {
          randy = rand() % 10;
          if (randy <= 3) {// 40%  grass
            region->tile_arr[i][j].ter = ter_grass;
          } else if (randy >= 4 && randy <= 6) { //30% clearing
            region->tile_arr[i][j].ter = ter_clearing;
          } else if (randy >= 7 && randy <= 8) { // 20% tree
            region->tile_arr[i][j].ter = ter_tree;
          }  else { // 10% boulder
            region->tile_arr[i][j].ter = ter_boulder;
          }
        }

      }
    }
  }

  // set path exit points
  // generate random exits if specified exit is -1.
  // exit cannot be a corner
  if (N_exit_j == -1) {
    region->N_exit_j = (rand() % (MAX_COL - 2)) + 1;
  } else {
    region->N_exit_j = N_exit_j;
  }
  region->tile_arr[0][region->N_exit_j].ter = ter_path;
  if (E_exit_i == -1) {
    region->E_exit_i = (rand() % (MAX_ROW - 2)) + 1;
  } else {
    region->E_exit_i = E_exit_i;
  }
  region->tile_arr[region->E_exit_i][MAX_COL - 1].ter = ter_path;
  if (S_exit_j == -1) {
    region->S_exit_j = (rand() % (MAX_COL - 2)) + 1;
  } else {
    region->S_exit_j = S_exit_j;
  }
  region->tile_arr[MAX_ROW - 1][region->S_exit_j].ter = ter_path;
  if (W_exit_i == -1) {
    region->W_exit_i = (rand() % (MAX_ROW - 2)) + 1;
  } else {
    region->W_exit_i = W_exit_i;
  }
  region->tile_arr[region->W_exit_i][0].ter = ter_path;

  // W->E path
  // prefers to generate paths between ter_types
  int32_t path_i = region->W_exit_i;
  int32_t path_j = 1;
  region->tile_arr[path_i][path_j].ter = ter_path;
  while (path_j != MAX_COL - 2) {
    // find the closest seed
    seed_t *closest_seed = seed_arr[0];
    double closest_dist = dist(seed_arr[0]->j, seed_arr[0]->i, path_j, path_i);
    for (int32_t k = 1; k < num_seeds; k++) {
      double temp_dist = dist(seed_arr[k]->j, seed_arr[k]->i, path_j, path_i);
      if (temp_dist < closest_dist) {
        closest_seed = seed_arr[k];
        closest_dist = temp_dist;
      }
    }

    // step the path in a direction furthest from the closest seed
    // direction is weighted to head towards the exit especially near the end
    // do not go to other path tiles or backwards. 
    // (only sideways and forward progress is allowed)
    double E_path_weight;
    double N_path_weight = INT32_MIN;
    double S_path_weight = INT32_MIN;
    double dist_to_seed = dist(closest_seed->j, closest_seed->i, path_j, path_i);
    double dist_to_exit = dist(MAX_COL - 1, region->E_exit_i, path_j, path_i);
    E_path_weight = 0.2*(dist(closest_seed->j, closest_seed->i, path_j + 1, path_i) - dist_to_seed) // prefer terrain boarders
                  + 0.5*(rand() % 10); // ensure random progress is made towards exit
    if (path_i - 1 != 0 && 
        region->tile_arr[path_i - 1][path_j].ter != ter_path) {
      N_path_weight = 0.2*(dist(closest_seed->j, closest_seed->i, path_j,  path_i - 1) - dist_to_seed) // prefer terrain boarders
                    + 0.05*path_j*(dist_to_exit - dist(MAX_COL - 1, region->E_exit_i, path_j, path_i - 1)) // head towards the exit especially near the end
                    + 0.05*path_i; // dont hug walls
    }
    if (path_i + 1 != MAX_ROW - 1 &&
        region->tile_arr[path_i + 1][path_j].ter != ter_path) {
      S_path_weight = 0.2*(dist(closest_seed->j, closest_seed->i, path_j, path_i + 1) - dist_to_seed) // prefer terrain boarders
                    + 0.05*path_j*(dist_to_exit - dist(MAX_COL - 1, region->E_exit_i, path_j, path_i + 1)) //  head towards the exit especially near the end
                    + 0.05*(MAX_ROW - path_i); // dont hug walls
    }

    if(E_path_weight >= N_path_weight && E_path_weight >= S_path_weight) {
      ++path_j;
    } else if (N_path_weight >= S_path_weight) {
      --path_i;
    } else {
      ++path_i;
    }
    region->tile_arr[path_i][path_j].ter = ter_path;
  }

  while (path_i > region->E_exit_i) {
    --path_i;
    region->tile_arr[path_i][path_j].ter = ter_path;
  }
  while (path_i < region->E_exit_i) {
    ++path_i;
    region->tile_arr[path_i][path_j].ter = ter_path;
  }

  // N->S path
  path_i = 1;
  path_j = region->N_exit_j;
  region->tile_arr[path_i][path_j].ter = ter_path;
  while (path_i != MAX_ROW - 2) {
    // find the closest seed
    seed_t *closest_seed = seed_arr[0];
    double closest_dist = dist(seed_arr[0]->j, seed_arr[0]->i, path_j, path_i);
    for (int32_t k = 1; k < num_seeds; k++) {
      double temp_dist = dist(seed_arr[k]->j, seed_arr[k]->i, path_j, path_i);
      if (temp_dist < closest_dist) {
        closest_seed = seed_arr[k];
        closest_dist = temp_dist;
      }
    }

    // step the path in a direction furthest from the closest seed
    // direction is weighted to head towards the exit especially near the end
    // do not go to other path tiles or backwards. 
    // (only sideways and forward progress is allowed)
    double S_path_weight = INT32_MIN;
    double E_path_weight = INT32_MIN;
    double W_path_weight = INT32_MIN;
    double dist_to_seed = dist(closest_seed->j, closest_seed->i, path_j, path_i);
    double dist_to_exit = dist(region->S_exit_j, MAX_ROW - 1, path_j, path_i);
    S_path_weight = 0.2*(dist(closest_seed->j, closest_seed->i, path_j, path_i + 1) - dist_to_seed)
                  + 0.5*(rand() % 10);
    if (path_j + 1 != MAX_COL - 1 && 
        region->tile_arr[path_i][path_j + 1].ter != ter_path) {
      E_path_weight = 0.2*(dist(closest_seed->j, closest_seed->i, path_j + 1,  path_i) - dist_to_seed)
                    + 0.1*path_i*(dist_to_exit - dist(region->S_exit_j, MAX_ROW - 1, path_j + 1, path_i))
                    + 0.05*(MAX_COL - path_j); // dont hug walls;;
    }
    if (path_j - 1 != 0 &&
        region->tile_arr[path_i][path_j - 1].ter != ter_path) {
      W_path_weight = 0.2*(dist(closest_seed->j, closest_seed->i, path_j - 1, path_i) - dist_to_seed)
                    + 0.1*path_i*(dist_to_exit - dist(region->S_exit_j, MAX_ROW - 1, path_j - 1, path_i))
                    + 0.05*path_j; // dont hug walls;
    }

    if(S_path_weight >= E_path_weight && S_path_weight >= W_path_weight) {
      ++path_i;
    } else if (E_path_weight >= W_path_weight) {
      ++path_j;
    } else {
      --path_j;
    }

    // if we interest the W->E path, the follow it for a random amount of tiles
    if (region->tile_arr[path_i][path_j].ter == ter_path) {
      int32_t num_tiles_to_trace = rand() % (MAX_COL/2);
      // follow either E or W, whatever will lead us closer the the S exit
      int32_t heading = 1; // 1 is E, -1 is W
      if (path_j > S_exit_j) {
        heading = -1;
      }
      while (num_tiles_to_trace != 0 && path_j > 1 && path_j < MAX_COL - 2 && path_i != MAX_ROW - 3) {
        if (region->tile_arr[path_i][path_j + heading].ter  == ter_path) {
          path_j += heading;
          --num_tiles_to_trace;
        } else if (region->tile_arr[path_i + 1][path_j].ter  == ter_path) {
          ++path_i;
          --num_tiles_to_trace;
        } else if (region->tile_arr[path_i - 1][path_j].ter  == ter_path) {
            --path_i;
            --num_tiles_to_trace;
        }
      }
    }

    region->tile_arr[path_i][path_j].ter = ter_path;
  }

  while (path_j > region->S_exit_j) {
    --path_j;
    region->tile_arr[path_i][path_j].ter = ter_path;
  }
  while (path_j < region->S_exit_j) {
    ++path_j;
    region->tile_arr[path_i][path_j].ter = ter_path;
  }

  free(seed_arr);

  // randomly select tiles and place a poke center if location is valid
  // poke centers must be placed next to a path'
  while (place_center != 0) {
    pos_t c_seed;
    c_seed.i = (rand() % (MAX_ROW - 4)) + 1;
    c_seed.j = (rand() % (MAX_COL - 4)) + 1;

    if ( region->tile_arr[c_seed.i][c_seed.j].ter != ter_path
      && region->tile_arr[c_seed.i + 1][c_seed.j].ter != ter_path
      && region->tile_arr[c_seed.i][c_seed.j + 1].ter != ter_path
      && region->tile_arr[c_seed.i + 1][c_seed.j + 1].ter != ter_path
      && 
      (    region->tile_arr[c_seed.i - 1][c_seed.j].ter == ter_path
        || region->tile_arr[c_seed.i - 1][c_seed.j + 1].ter == ter_path
        || region->tile_arr[c_seed.i][c_seed.j - 1].ter == ter_path
        || region->tile_arr[c_seed.i + 1][c_seed.j - 1].ter == ter_path
        || region->tile_arr[c_seed.i][c_seed.j + 2].ter == ter_path
        || region->tile_arr[c_seed.i + 1][c_seed.j + 2].ter == ter_path
        || region->tile_arr[c_seed.i + 2][c_seed.j].ter == ter_path
        || region->tile_arr[c_seed.i + 2][c_seed.j + 1].ter == ter_path
      ) ) {
      region->tile_arr[c_seed.i][c_seed.j].ter = ter_center;
      region->tile_arr[c_seed.i + 1][c_seed.j].ter = ter_center;
      region->tile_arr[c_seed.i][c_seed.j + 1].ter = ter_center;
      region->tile_arr[c_seed.i + 1][c_seed.j + 1].ter = ter_center;
      place_center = 0;
    }
  }

  while (place_mart != 0) {
    pos_t c_seed;
    c_seed.i = (rand() % (MAX_ROW - 4)) + 1;
    c_seed.j = (rand() % (MAX_COL - 4)) + 1;

    if ( region->tile_arr[c_seed.i][c_seed.j].ter != ter_path
      && region->tile_arr[c_seed.i + 1][c_seed.j].ter != ter_path
      && region->tile_arr[c_seed.i][c_seed.j + 1].ter != ter_path
      && region->tile_arr[c_seed.i + 1][c_seed.j + 1].ter != ter_path
      && region->tile_arr[c_seed.i][c_seed.j].ter != ter_center
      && region->tile_arr[c_seed.i + 1][c_seed.j].ter != ter_center
      && region->tile_arr[c_seed.i][c_seed.j + 1].ter != ter_center
      && region->tile_arr[c_seed.i + 1][c_seed.j + 1].ter != ter_center
      && 
      (    region->tile_arr[c_seed.i - 1][c_seed.j].ter == ter_path
        || region->tile_arr[c_seed.i - 1][c_seed.j + 1].ter == ter_path
        || region->tile_arr[c_seed.i][c_seed.j - 1].ter == ter_path
        || region->tile_arr[c_seed.i + 1][c_seed.j - 1].ter == ter_path
        || region->tile_arr[c_seed.i][c_seed.j + 2].ter == ter_path
        || region->tile_arr[c_seed.i + 1][c_seed.j + 2].ter == ter_path
        || region->tile_arr[c_seed.i + 2][c_seed.j].ter == ter_path
        || region->tile_arr[c_seed.i + 2][c_seed.j + 1].ter == ter_path
      ) ) {
      region->tile_arr[c_seed.i][c_seed.j].ter = ter_mart;
      region->tile_arr[c_seed.i + 1][c_seed.j].ter = ter_mart;
      region->tile_arr[c_seed.i][c_seed.j + 1].ter = ter_mart;
      region->tile_arr[c_seed.i + 1][c_seed.j + 1].ter = ter_mart;
      place_mart = 0;
    }
  }

  // Populate npc trainers
  if (numtrainers_opt < 0) {
    // generate a random number of npcs to spawn
    region->num_npc = (rand() % (MAX_TRAINERS - MIN_TRAINERS + 1)) + MIN_TRAINERS;
  } else {
    region->num_npc = numtrainers_opt;
  }
  
  character_t *new_npc_arr = malloc(region->num_npc * sizeof(*(new_npc_arr)));
  for (int32_t m = 0; m < region->num_npc; m++) {
    int32_t spawn_attempts = 5;
    new_npc_arr[m].movetime = 0;
    while (spawn_attempts != 0) {
      int32_t ti = (rand() % (MAX_ROW - 2)) + 1;
      int32_t tj = (rand() % (MAX_COL - 2)) + 1;
      trainer_t tt;
      if (m == 0) {
        tt = tnr_rival;
      } else if (m == 1) {
        tt = tnr_hiker;
      } else {
        tt = (rand() % (tnr_rand_walker - tnr_hiker + 1)) + tnr_hiker;
      }
      int32_t is_valid = 1;

      // verify this npc can move on the tile on to the tile it spawns on
      if (travel_times[region->tile_arr[ti][tj].ter][tt] == INT_MAX) {
        is_valid = 0;
      }

      // verify no other npcs occupy this space
      if (is_valid) {
        for (int32_t n = 0; n < m; n++) {
          if (new_npc_arr[n].pos_i == ti && new_npc_arr[n].pos_j == tj) {
            is_valid = 0;
            break;
          }
        }
      }

      if (is_valid) {
        new_npc_arr[m].pos_i = ti;
        new_npc_arr[m].pos_j = tj;
        new_npc_arr[m].tnr = tt;
        if (tt == tnr_pacer || tnr_wanderer) {
          new_npc_arr[m].dir = rand() % 8;
        }
        spawn_attempts = 0;
      } else {
        --spawn_attempts;
      }

    }
  }

  region->npc_arr = new_npc_arr;
  return;
}

/*
 * Process user input
 * Update which region is being displayed, generate new regions as needed.
 */
void process_input (int32_t *region_x, int32_t *region_y, uint32_t *running) {
  char user_in = 0; 
  int32_t tmp_x = INT32_MIN;
  int32_t tmp_y = INT32_MIN;
  scanf("%c", &user_in);
  switch (user_in) {
    case 'n':
      if ((*region_y) < WORLD_SIZE - 1) {
        ++(*region_y);
      } else {
        printf("Illegal input, out of bounds.\n");
      }
      break;
    case 'e':
      if ((*region_x) < WORLD_SIZE - 1) {
        ++(*region_x);
      } else {
        printf("Illegal input, out of bounds.\n");
      }
      break;
    case 's':
      if ((*region_y) > 0) {
        --(*region_y);
      } else {
        printf("Illegal input, out of bounds.\n");
      }
      break;
    case 'w':
      if ((*region_x) > 0) {
        --(*region_x);
      } else {
        printf("Illegal input, out of bounds.\n");
      }
      break;
    case 'f':
      scanf(" %d %d", &tmp_x, &tmp_y);
      if (abs(tmp_x) <= WORLD_SIZE/2 && abs(tmp_y) <= WORLD_SIZE/2) {
        *region_x = tmp_x + WORLD_SIZE/2;
        *region_y = tmp_y + WORLD_SIZE/2;
      } else {
        printf("Illegal input, coordinates entered may be out of bounds.\n");
      }
      break;
    case 'q':
      *running = 0;
    case ' ':
    case 10:
      // do nothing, ignore spaces and linefeed characters
      break;
    default:
      printf("Invalid input.\n");
      break;
  }
  return;
}

static int32_t path_cmp(const void *key, const void *with) {
  return ((path_t *) key)->cost - ((path_t *) with)->cost;
}

/*
 * Uses dijkstra's algorithm to find an optimal path to a specified location
 *
 * Returns distance in number of steps between the points.
 * INT_MAX for no valid route.
 */
static void dijkstra(region_t *region, trainer_t tnr,
                   int32_t pc_i, int32_t pc_j) {

  static path_t path[MAX_ROW][MAX_COL], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t i, j;
  terrain_t ter;
  int32_t neighbor_cost;


  if (!initialized) {
    for (i = 0; i < MAX_ROW; i++) {
      for (j = 0; j < MAX_COL; j++) {
        path[i][j].pos_i = i;
        path[i][j].pos_j = j;
      }
    }
    initialized = 1;
  }

  for (i = 0; i < MAX_ROW; i++) {
    for (j = 0; j < MAX_COL; j++) {
      path[i][j].cost = INT_MAX;
    }
  }

  path[pc_i][pc_j].cost = 0;

  heap_init(&h, path_cmp, NULL);

  for (i = 1; i < MAX_ROW - 1; i++) {
    for (j = 1; j < MAX_COL - 1; j++) {
      if (travel_times[region->tile_arr[i][j].ter][tnr]  != INT_MAX) {
        path[i][j].hn = heap_insert(&h, &path[i][j]);
      } else {
        path[i][j].hn = NULL;
      }
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    // North
    ter = region->tile_arr[p->pos_i - 1][p->pos_j    ].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i - 1][p->pos_j    ].hn) && 
        (path[p->pos_i - 1][p->pos_j    ].cost > neighbor_cost)) {
      path[p->pos_i - 1][p->pos_j    ].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i - 1][p->pos_j    ].hn);
    }
    // South
    ter = region->tile_arr[p->pos_i + 1][p->pos_j    ].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i + 1][p->pos_j    ].hn) &&
        (path[p->pos_i + 1][p->pos_j    ].cost > neighbor_cost)) {
      path[p->pos_i + 1][p->pos_j    ].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i + 1][p->pos_j    ].hn);
    }
    // East
    ter = region->tile_arr[p->pos_i    ][p->pos_j + 1].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i    ][p->pos_j + 1].hn) &&
        (path[p->pos_i    ][p->pos_j + 1].cost > neighbor_cost)) {
      path[p->pos_i    ][p->pos_j + 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i    ][p->pos_j + 1].hn);
    }
    // West
    ter = region->tile_arr[p->pos_i    ][p->pos_j - 1].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i    ][p->pos_j - 1].hn) &&
        (path[p->pos_i    ][p->pos_j - 1].cost > neighbor_cost)) {
      path[p->pos_i    ][p->pos_j - 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i    ][p->pos_j - 1].hn);
    }
    // North East
    ter = region->tile_arr[p->pos_i - 1][p->pos_j + 1].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : (p->cost + travel_times[ter][tnr]);
    if ((path[p->pos_i - 1][p->pos_j + 1].hn) && 
        (path[p->pos_i - 1][p->pos_j + 1].cost > neighbor_cost)) {
      path[p->pos_i - 1][p->pos_j + 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i - 1][p->pos_j + 1].hn);
    }
    // North West
    ter = region->tile_arr[p->pos_i - 1][p->pos_j - 1].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i - 1][p->pos_j - 1].hn) && 
        (path[p->pos_i - 1][p->pos_j - 1].cost > neighbor_cost)) {
      path[p->pos_i - 1][p->pos_j - 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i - 1][p->pos_j - 1].hn);
    }
    // South East
    ter = region->tile_arr[p->pos_i + 1][p->pos_j + 1].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i + 1][p->pos_j + 1].hn) && 
        (path[p->pos_i + 1][p->pos_j + 1].cost > neighbor_cost)) {
      path[p->pos_i + 1][p->pos_j + 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i + 1][p->pos_j + 1].hn);
    }
    // South West
    ter = region->tile_arr[p->pos_i + 1][p->pos_j - 1].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i + 1][p->pos_j - 1].hn) && 
        (path[p->pos_i + 1][p->pos_j - 1].cost > neighbor_cost)) {
      path[p->pos_i + 1][p->pos_j - 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i + 1][p->pos_j - 1].hn);
    }
  }

  if (tnr == tnr_hiker) {
    for (int32_t i = 0; i < MAX_ROW; i++) {
      for (int32_t j = 0; j < MAX_COL; j++) {
        dist_map_hiker[i][j] = path[i][j].cost;
      }
    }
  } else if (tnr == tnr_rival) {
    for (int32_t i = 0; i < MAX_ROW; i++) {
      for (int32_t j = 0; j < MAX_COL; j++) {
        dist_map_rival[i][j] = path[i][j].cost;
      }
    }
  }

  heap_delete(&h);
  return;
}

void init_pc (character_t *pc, region_t *region) {
  pc->movetime = 0;
  pc->tnr = tnr_pc;

  int32_t found_location = 0;
  while (found_location != 1) {
    pc->pos_i = (rand() % (MAX_ROW - 2)) + 1;
    pc->pos_j = (rand() % (MAX_COL - 2)) + 1;
    if (region->tile_arr[pc->pos_i][pc->pos_j].ter == ter_path) {
      found_location = 1;
      for (int32_t i = 0; i < region->num_npc; i++) {
        if (region->npc_arr[i].pos_i == pc->pos_i && 
            region->npc_arr[i].pos_j == pc->pos_j) {
          found_location = 0;
          break;
        }
      }
    }
  }
}

void print_dist_map(int32_t dist_map[][MAX_COL]) {
  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      if (dist_map[i][j] != INT_MAX) {
        printf("%02d ", dist_map[i][j] % 100);
      } else {
        printf("   ");
      }
    }
    printf("   \n");
  }
}

void load_region(int32_t region_x, int32_t region_y, int32_t num_tnr) {
  // If the region we are in is uninitialized, then generate the region.
  if (region_ptr[region_x][region_y] == NULL) {
    region_t *new_region = malloc(sizeof(*new_region));
    region_ptr[region_x][region_y] = new_region;
    int32_t N_exit, E_exit, S_exit, W_exit;
    N_exit = -1;
    E_exit = -1;
    S_exit = -1;
    W_exit = -1;

    // determine if new region should generate with poke center and/or mart
    // (-45d/200 + 50) / 100 => -0.45*d/200 + 0.50
    int32_t d = m_dist(region_x, region_y, WORLD_SIZE/2, WORLD_SIZE/2);
    double p = (-0.45*d)/200 + 0.50;
    int32_t place_center = rand_outcome(p);
    int32_t place_mart = rand_outcome(p);

    if (region_y + 1 < WORLD_SIZE) {
      if (region_ptr[region_x][region_y + 1] != NULL) {
        N_exit = region_ptr[region_x][region_y + 1]->S_exit_j; /* North Region, South Exit */
      }
    }
    if (region_x + 1 < WORLD_SIZE) {
      if (region_ptr[region_x + 1][region_y] != NULL) {
        E_exit = region_ptr[region_x + 1][region_y]->W_exit_i; /* East Region, West Exit */
      }
    }
    if (region_y - 1 >= 0) {
      if (region_ptr[region_x][region_y - 1] != NULL) {
        S_exit = region_ptr[region_x][region_y - 1]->N_exit_j; /* South Region, North Exit */
      }
    }
    if (region_x - 1 >= 0) {
      if (region_ptr[region_x - 1][region_y] != NULL) {
        W_exit = region_ptr[region_x - 1][region_y]->E_exit_i; /* West Region, East Exit */
      }
    }
    init_region(region_ptr[region_x][region_y], N_exit, E_exit, S_exit, W_exit, place_center, place_mart, num_tnr);
  
    // If on the edge of the world, block exits with boulders so that player cannot
    // fall out of the world
    if (region_y == WORLD_SIZE - 1) {
      N_exit = region_ptr[region_x][region_y]->N_exit_j;
      region_ptr[region_x][region_y]->tile_arr[0][N_exit].ter = ter_border;
    }
    if (region_x == WORLD_SIZE - 1) {
      E_exit = region_ptr[region_x][region_y]->E_exit_i;
      region_ptr[region_x][region_y]->tile_arr[E_exit][MAX_COL-1].ter = ter_border;
    }
    if (region_y == 0) {
      S_exit = region_ptr[region_x][region_y]->S_exit_j;
      region_ptr[region_x][region_y]->tile_arr[MAX_ROW-1][S_exit].ter = ter_border;
    }
    if (region_x == 0) {
      W_exit = region_ptr[region_x][region_y]->W_exit_i;
      region_ptr[region_x][region_y]->tile_arr[W_exit][0].ter = ter_border;
    }
  }
}

/*
 * Free all memomry allocated to regions
 */
void free_all_regions() {
  for (int32_t i = 0; i < WORLD_SIZE; i++) {
    for (int32_t j = 0; j < WORLD_SIZE; j++) {
      if (region_ptr[i][j] != NULL) {
        free(region_ptr[i][j]->npc_arr);
        free(region_ptr[i][j]);
      }
    }
  }
}

static int32_t movetime_cmp(const void *key, const void *with) {
  return ((character_t *) key)->movetime - ((character_t *) with)->movetime;
}

/*
 * Initializes the priority queue with the player and trainers from a region
 */
void init_trainer_pq(heap_t *queue, character_t *pc, region_t *region) {
  heap_init(queue, movetime_cmp, NULL);
  pc->hn = heap_insert(queue, pc);
  for (int32_t i = 0; i < region->num_npc; i++) {
    region->npc_arr[i].hn = heap_insert(queue, &(region->npc_arr[i]));
  }
}

/*
 * Performs the all the nessesary checks to ensure a location is a valid spot to move to.
 * Returns 1 if the location is valid, 0 otherwise.
 */
int is_valid_location(int32_t to_i, int32_t to_j, 
                      trainer_t tnr, region_t *region, character_t *pc) {
  if (travel_times[region->tile_arr[to_i][to_j].ter][tnr] == INT_MAX) {
    return 0;
  }
  if (pc->pos_i == to_i
   && pc->pos_j == to_j) {
    return 0;
  }
  for (int32_t k = 0; k < region->num_npc; ++k) {
    if (region->npc_arr[k].pos_i == to_i
     && region->npc_arr[k].pos_j == to_j) {
        return 0;
      }
  }
  if (tnr != tnr_pc && ( 
     to_i <= 0 || to_i >= MAX_ROW - 1 ||
     to_j <= 0 || to_j >= MAX_COL - 1) ) {
    return 0;
  }
  return 1;
}

/*
 * Performs the all the nessesary checks to ensure a location is a valid spot to move to.
 * Returns 1 if the location is valid, 0 otherwise.
 */
int is_valid_gradient(int32_t to_i, int32_t to_j, 
                      region_t *region, int32_t dist_map[MAX_ROW][MAX_COL], character_t *pc) {
  if (dist_map[to_i][to_j] == INT_MAX) {
    return 0;
  }
  if (pc->pos_i == to_i
   && pc->pos_j == to_j) {
    return 0;
  }
  for (int32_t k = 0; k < region->num_npc; ++k) {
    if (region->npc_arr[k].pos_i == to_i
     && region->npc_arr[k].pos_j == to_j) {
        return 0;
    }
  }
  return 1;
}

/*
 * Move a trainer along the maximum gradient
 */
void move_along_gradient(character_t *c, region_t *region, int32_t dist_map[MAX_ROW][MAX_COL], character_t *pc) {
  int32_t next_i = 0;
  int32_t next_j = 0;
  int32_t max_gradient = INT_MAX;

  // North
  if (is_valid_gradient(c->pos_i - 1, c->pos_j    , region, dist_map, pc)
            && dist_map[c->pos_i - 1][c->pos_j    ] < max_gradient) {
    max_gradient = dist_map[c->pos_i - 1][c->pos_j    ];
    next_i = -1;
    next_j = 0;
  }
  // East
  if (is_valid_gradient(c->pos_i    , c->pos_j + 1, region, dist_map, pc)
            && dist_map[c->pos_i    ][c->pos_j + 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i    ][c->pos_j + 1];
    next_i = 0;
    next_j = 1;
  }
  // South
  if (is_valid_gradient(c->pos_i + 1, c->pos_j    , region, dist_map, pc)
            && dist_map[c->pos_i + 1][c->pos_j    ] < max_gradient) {
    max_gradient = dist_map[c->pos_i + 1][c->pos_j    ];
    next_i = 1;
    next_j = 0;
  }
  // West
  if (is_valid_gradient(c->pos_i    , c->pos_j - 1, region, dist_map, pc)
            && dist_map[c->pos_i    ][c->pos_j - 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i    ][c->pos_j - 1];
    next_i = 0;
    next_j = -1;
  }
  // North East
  if (is_valid_gradient(c->pos_i - 1, c->pos_j + 1, region, dist_map, pc)
            && dist_map[c->pos_i - 1][c->pos_j + 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i - 1][c->pos_j + 1];
    next_i = -1;
    next_j = 1;
  }
  // South East
  if (is_valid_gradient(c->pos_i + 1, c->pos_j + 1, region, dist_map, pc)
            && dist_map[c->pos_i + 1][c->pos_j + 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i + 1][c->pos_j + 1];
    next_i = 1;
    next_j = 1;
  }
  // South West
  if (is_valid_gradient(c->pos_i + 1, c->pos_j - 1, region, dist_map, pc)
            && dist_map[c->pos_i + 1][c->pos_j - 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i + 1][c->pos_j - 1];
    next_i = 1;
    next_j = -1;
  }
  // North West
  if (is_valid_gradient(c->pos_i - 1, c->pos_j - 1, region, dist_map, pc)
            && dist_map[c->pos_i - 1][c->pos_j - 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i - 1][c->pos_j - 1];
    next_i = -1;
    next_j = -1;
  }

  c->pos_i += next_i;
  c->pos_j += next_j;
  return;
}

/*
 * Process a trainer's movement
 */
void move_trainer(character_t *c, region_t *region, character_t *pc) {
  switch (c->tnr)
  {
  case tnr_pc:
    for (int32_t k = 0; k < region->num_npc; ++k) {
      if (region->npc_arr[k].pos_i <= pc->pos_i + 1
      && region->npc_arr[k].pos_i >= pc->pos_i - 1
      && region->npc_arr[k].pos_j <= pc->pos_j + 1
      && region->npc_arr[k].pos_j >= pc->pos_j - 1) {
          init_pc(c, region);
      }
    }
    break;
  case tnr_hiker:
    move_along_gradient(c, region, dist_map_hiker, pc);
    break;
  case tnr_rival:
    move_along_gradient(c, region, dist_map_rival, pc);
    break;
  case tnr_pacer:
    if (is_valid_location(c->pos_i + dir_offsets[c->dir][0], 
                          c->pos_j + dir_offsets[c->dir][1], 
                          c->tnr, region, pc)) {
      c->pos_i += dir_offsets[c->dir][0];
      c->pos_j += dir_offsets[c->dir][1];
    } else {
      c->dir = (c->dir + 4) % 8;
    }
    break;
  case tnr_wanderer:
    if ((region->tile_arr[ c->pos_i + dir_offsets[c->dir][0] ][ c->pos_j + dir_offsets[c->dir][1] ].ter
      == region->tile_arr[ c->pos_i                          ][ c->pos_j                          ].ter)
     && is_valid_location(c->pos_i + dir_offsets[c->dir][0], 
                          c->pos_j + dir_offsets[c->dir][1], 
                          c->tnr, region, pc)) {
      c->pos_i += dir_offsets[c->dir][0];
      c->pos_j += dir_offsets[c->dir][1];
    } else {
      c->dir = rand() % 8;
    }
    break;
  case tnr_stationary:
    // Do nothing
    break;
  case tnr_rand_walker:
    if (is_valid_location(c->pos_i + dir_offsets[c->dir][0], 
                          c->pos_j + dir_offsets[c->dir][1], 
                          c->tnr, region, pc)) {
      c->pos_i += dir_offsets[c->dir][0];
      c->pos_j += dir_offsets[c->dir][1];
    } else {
      c->dir = rand() % 8;
    }
    break;
  default:
    printf("Error: Movement for trainer type %d has not been implemented!\n",c->tnr);
    exit(1);
    break;
  }
  c->movetime = travel_times[ (region->tile_arr[c->pos_i][c->pos_j].ter) ][ c->tnr ];
  return;
}

/*
 * Step player character and npc movetimes by amount
 */
void step_all_movetimes(character_t *pc, region_t *region, int32_t amount) {
  if (amount == 0) {
    return;
  }
  pc->movetime -= amount;
  for (int32_t i = 0; i < region->num_npc; i++) {
    region->npc_arr[i].movetime -= amount;
  }
  return;
}

int main (int argc, char *argv[])
{
  int32_t seed;
  int32_t numtrainers_opt = NUM_TRAINERS;
  int32_t loaded_region_x = WORLD_SIZE/2;
  int32_t loaded_region_y = WORLD_SIZE/2;
  //int32_t prev_region_x = WORLD_SIZE/2;
  //int32_t prev_region_y = WORLD_SIZE/2;
  int32_t prev_pc_pos_i = -1;
  int32_t prev_pc_pos_j = -1;
  
   // generate random seed
  struct timeval t;
  gettimeofday(&t, NULL);
  seed = (t.tv_usec ^ (t.tv_sec << 20)) & 0xffffffff;

  // handle command line inputs
  if (argc != 1 && argc != 3) {
    usage(argv[0]);
  }
  if (argc == 3) {
    if (!strcmp(argv[1], "--numtrainers")) {
      numtrainers_opt = atoi(argv[2]);
    } else if (!strcmp(argv[1], "--seed")) {
      seed = atoi(argv[2]);
    } else {
      usage(argv[0]);
      return -1;
    }
  }
  srand(seed);

  // start in center of the world. 
  // The center of the world may also be referred to as (0,0)
  character_t pc;
  // Allocate memory for and generate the starting region
  region_t *new_region = malloc(sizeof(*new_region));
  region_ptr[loaded_region_x][loaded_region_y] = new_region;
  init_region(region_ptr[loaded_region_x][loaded_region_y], -1, -1, -1, -1, 1, 1, numtrainers_opt);

  init_pc(&pc, region_ptr[loaded_region_x][loaded_region_y]);
  
  heap_t move_queue;
  character_t *c;
  init_trainer_pq(&move_queue, &pc, region_ptr[loaded_region_x][loaded_region_y]);
  dijkstra(region_ptr[loaded_region_x][loaded_region_y], tnr_hiker, pc.pos_i, pc.pos_j);
  dijkstra(region_ptr[loaded_region_x][loaded_region_y], tnr_rival, pc.pos_i, pc.pos_j);

  print_region(region_ptr[loaded_region_x][loaded_region_y], &pc);

  // Run game
  uint32_t running = 1;
  while(running) { 
    // LEGACY MOVE REGION CODE
    // if (loaded_region_x != prev_region_x || loaded_region_y != prev_region_y) {
    //   load_region(loaded_region_x, loaded_region_y, numtrainers_opt);
    //   prev_region_x = loaded_region_x;
    //   prev_region_y = loaded_region_y;

    //   init_pc(&pc, region_ptr[loaded_region_x][loaded_region_y]);
    //   dijkstra(region_ptr[loaded_region_x][loaded_region_y], tnr_hiker, pc.pos_i, pc.pos_j);
    //   dijkstra(region_ptr[loaded_region_x][loaded_region_y], tnr_rival, pc.pos_i, pc.pos_j);
    //   print_dist_map(dist_map_hiker);
    //   print_dist_map(dist_map_rival);
    //   init_trainer_pq(&move_queue, &pc, region_ptr[loaded_region_x][loaded_region_y]);
    // }

    if (pc.pos_i != prev_pc_pos_i || pc.pos_j != prev_pc_pos_j) {
      dijkstra(region_ptr[loaded_region_x][loaded_region_y], tnr_hiker, pc.pos_i, pc.pos_j);
      dijkstra(region_ptr[loaded_region_x][loaded_region_y], tnr_rival, pc.pos_i, pc.pos_j);
      prev_pc_pos_i = loaded_region_x;
      prev_pc_pos_j = loaded_region_y;
    }

    int32_t ticks_since_last_frame = 0;
    while (ticks_since_last_frame <= TICKS_PER_FRAME) {
      int32_t step = ((character_t*)heap_peek_min(&move_queue))->movetime;
      if (step <= TICKS_PER_FRAME) {
        step_all_movetimes(&pc, region_ptr[loaded_region_x][loaded_region_y], step);
        while( ((character_t*)heap_peek_min(&move_queue))->movetime == 0) {
          c = heap_remove_min(&move_queue);
          move_trainer(c, region_ptr[loaded_region_x][loaded_region_y], &pc);
          c->hn = heap_insert(&move_queue, c);
        }
      } else {
        step = TICKS_PER_FRAME;
      }
      step_all_movetimes(&pc, region_ptr[loaded_region_x][loaded_region_y], step);
      ticks_since_last_frame += step;
    }
    usleep(FRAMETIME);
    print_region(region_ptr[loaded_region_x][loaded_region_y], &pc);

    //process_input(&loaded_region_x, &loaded_region_y, &running); 
  }

  // debug
  while ((c = heap_remove_min(&move_queue))) {
    printf("%d  %d\n", c->tnr, c->movetime);
  }

  


  heap_delete(&move_queue);
  free_all_regions();

  return 0;
}