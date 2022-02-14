#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "heap.h"

// Printed region dimensions 
#define MAX_ROW 21
#define MAX_COL 80

// World dimensions, world is made up of regions (WORLD_SIZE * WORLD_SIZE)
#define WORLD_SIZE 399

#define MIN_SEEDS_PER_REGION 6 // at least 2 grass and 2 clearings seeds. (req)
#define MAX_SEEDS_PER_REGION 12

#define CHAR_BORDER    '%';
#define CHAR_BOULDER   '%';
#define CHAR_TREE      '^';
#define CHAR_CENTER    'C';
#define CHAR_MART      'M';
#define CHAR_PATH      '#';
#define CHAR_GRASS     ':';
#define CHAR_CLEARING  '.';
#define CHAR_MOUNTAIN  '%';
#define CHAR_FOREST    '^';
#define CHAR_UNDEFINED 'U';
#define CHAR_PC        '@';

enum terrain {
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
};

enum trainer {
  tnr_hiker,
  tnr_rival,
  tnr_pc,
  tnr_other
};

static int32_t travel_times[11][4] = {
                  /*    Hiker,   Rival,      PC,  Others */
  /* ter_border   */ {10000, 10000, 10000, 10000},
  /* ter_boulder  */ {10000, 10000, 10000, 10000},
  /* ter_tree     */ {10000, 10000, 10000, 10000},
  /* ter_center   */ {10000, 10000,      10, 10000},
  /* ter_mart     */ {10000, 10000,      10, 10000},
  /* ter_path     */ {     10,      10,      10,      10},
  /* ter_grass    */ {     15,      20,      20,      20},
  /* ter_clearing */ {     10,      10,      10,      10},
  /* ter_mountain */ {     15, 10000, 10000, 10000},
  /* ter_forest   */ {     15, 10000, 10000, 10000},
  /* ter_mixed    */ {10000, 10000, 10000, 10000}
};

// static int32_t travel_times[11][4] = {
//                   /*    Hiker,   Rival,      PC,  Others */
//   /* ter_border   */ {INT_MAX, INT_MAX, INT_MAX, INT_MAX},
//   /* ter_boulder  */ {INT_MAX, INT_MAX, INT_MAX, INT_MAX},
//   /* ter_tree     */ {INT_MAX, INT_MAX, INT_MAX, INT_MAX},
//   /* ter_center   */ {INT_MAX, INT_MAX,      10, INT_MAX},
//   /* ter_mart     */ {INT_MAX, INT_MAX,      10, INT_MAX},
//   /* ter_path     */ {     10,      10,      10,      10},
//   /* ter_grass    */ {     15,      20,      20,      20},
//   /* ter_clearing */ {     10,      10,      10,      10},
//   /* ter_mountain */ {     15, INT_MAX, INT_MAX, INT_MAX},
//   /* ter_forest   */ {     15, INT_MAX, INT_MAX, INT_MAX},
//   /* ter_mixed    */ {INT_MAX, INT_MAX, INT_MAX, INT_MAX}
// };

typedef struct tile {
  enum terrain ter;
} tile_t;

typedef struct seed {
  int32_t i, j;
  enum terrain ter;
} seed_t;

typedef struct region {
  tile_t tile_arr[MAX_ROW][MAX_COL];
  int32_t N_exit_j, E_exit_i, S_exit_j, W_exit_i;
} region_t;

typedef struct pos {
  int32_t i, j;
} pos_t;

typedef struct player_character {
  int32_t region_x, region_y;
  int32_t pos_i, pos_j;
} player_character_t;

typedef struct path {
  heap_node_t *hn;
  int32_t pos_i, pos_j;
  int32_t from_i, from_j;
  int32_t cost;
} path_t;

// Global variables
// 2D array of pointers, each pointer points to one of the regions the world
region_t *region_ptr[WORLD_SIZE][WORLD_SIZE] = {NULL};

player_character_t pc;


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
void print_region (region_t *region) {
  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      char terrain_char;
      
      if (pc.pos_i == i && pc.pos_j == j) {
        terrain_char = CHAR_PC;
      } else {
      enum terrain tmp = region->tile_arr[i][j].ter;
        switch (tmp) {
          case ter_border:
            terrain_char = CHAR_BORDER;
            break;
          case ter_clearing:
            terrain_char = CHAR_CLEARING;
            break;
          case ter_grass:
            terrain_char = CHAR_GRASS;
            break;
          case ter_boulder:
            terrain_char = CHAR_BOULDER;
            break;
          case ter_tree:
            terrain_char = CHAR_TREE;
            break;
          case ter_mountain:
            terrain_char = CHAR_MOUNTAIN;
            break;
          case ter_forest:
            terrain_char = CHAR_FOREST;
            break;
          case ter_path:
            terrain_char = CHAR_PATH;
            break;
          case ter_center:
            terrain_char = CHAR_CENTER;
            break;
          case ter_mart:
            terrain_char = CHAR_MART;
            break;
          default:
            terrain_char = CHAR_UNDEFINED;
        }
      }
      printf("%c", terrain_char);
    }
    printf("\n");
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
                  int32_t place_center, int32_t place_mart) {
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

void init_pc () {
  int32_t found_location = 0;
  while (found_location != 1) {
    pc.pos_i = (rand() % (MAX_ROW - 2)) + 1;
    pc.pos_j = (rand() % (MAX_COL - 2)) + 1;
    if (region_ptr[pc.region_x][pc.region_y]->tile_arr[pc.pos_i][pc.pos_j].ter == ter_path) {
      found_location = 1;
    }
  }
}

void load_region(int32_t region_x, int32_t region_y) {
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
    init_region(region_ptr[region_x][region_y], N_exit, E_exit, S_exit, W_exit, place_center, place_mart);
  
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
  
  init_pc();

  printf("Current region (%d,%d)", region_x - WORLD_SIZE/2, region_y - WORLD_SIZE/2);
  printf(", player at (%d,%d)\n", pc.pos_i, pc.pos_j);
  print_region(region_ptr[region_x][region_y]);
}

/*
 * Free all memomry allocated to regions
 */
void free_all_regions() {
  for (int32_t i = 0; i < WORLD_SIZE; i++) {
    for (int32_t j = 0; j < WORLD_SIZE; j++) {
      if (region_ptr[i][j] != NULL) {
        free(region_ptr[i][j]);
      }
    }
  }
}

static int32_t path_cmp(const void *key, const void *with) {
  return ((path_t *) key)->cost - ((path_t *) with)->cost;
}

static void a_star(region_t *region, enum trainer tnr,
                   int32_t from_i, int32_t from_j, 
                   int32_t to_i,   int32_t to_j)
{
  static path_t path[MAX_ROW][MAX_COL], *p;
  heap_t h;
  uint32_t i, j;
  enum terrain ter;
  int32_t neighbor_cost;

  // Assign costs
  for (i = 0; i < MAX_ROW; i++) {
    for (j = 0; j < MAX_COL; j++) {
      path[i][j].pos_i = i;
      path[i][j].pos_j = j;
      path[i][j].cost = INT_MAX;
    }
  }
  path[from_i][from_j].cost = 0;

  heap_init(&h, path_cmp, NULL);

  for (i = 1; i < MAX_ROW - 1; i++) {
    for (j = 1; j < MAX_COL - 1; j++) {
      path[i][j].hn = heap_insert(&h, &path[i][j]);
    }
  }

  while ((p = heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos_i == to_i) && p->pos_j == to_j) {
      for (i = to_i, j = to_j;
           (i != from_i) || (j != from_j);
           p = &path[i][j], i = p->from_i, j = p->from_j) {
        region->tile_arr[i][j].ter = ter_center;
      }
      heap_delete(&h);
      return;
    }

/*
    if tentative_gScore < gScore[neighbor]
    // This path to neighbor is better than any previous one. Record it!
    cameFrom[neighbor] := current
    gScore[neighbor] := tentative_gScore
    fScore[neighbor] := tentative_gScore + h(neighbor)
    if neighbor not in openSet
        openSet.add(neighbor)
*/

    // North
    ter = region->tile_arr[p->pos_i - 1][p->pos_j    ].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i - 1][p->pos_j    ].hn) && 
        (path[p->pos_i - 1][p->pos_j    ].cost > neighbor_cost)) {
      // cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
      //        INT_MAX : p->cost + travel_times[ter][tnr];
      path[p->pos_i - 1][p->pos_j    ].cost = neighbor_cost;
      path[p->pos_i - 1][p->pos_j    ].from_i = p->pos_i;
      path[p->pos_i - 1][p->pos_j    ].from_j = p->pos_j;
      heap_decrease_key_no_replace(&h, path[p->pos_i - 1][p->pos_j    ].hn);
    }
    // South
    ter = region->tile_arr[p->pos_i + 1][p->pos_j    ].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i + 1][p->pos_j    ].hn) &&
        (path[p->pos_i + 1][p->pos_j    ].cost > neighbor_cost)) {
      path[p->pos_i + 1][p->pos_j    ].cost = neighbor_cost;
      path[p->pos_i + 1][p->pos_j    ].from_i = p->pos_i;
      path[p->pos_i + 1][p->pos_j    ].from_j = p->pos_j;
      heap_decrease_key_no_replace(&h, path[p->pos_i + 1][p->pos_j    ].hn);
    }
    // East
    ter = region->tile_arr[p->pos_i    ][p->pos_j + 1].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i    ][p->pos_j + 1].hn) &&
        (path[p->pos_i    ][p->pos_j + 1].cost > neighbor_cost)) {
      path[p->pos_i    ][p->pos_j + 1].cost = neighbor_cost;
      path[p->pos_i    ][p->pos_j + 1].from_i = p->pos_i;
      path[p->pos_i    ][p->pos_j + 1].from_j = p->pos_j;
      heap_decrease_key_no_replace(&h, path[p->pos_i    ][p->pos_j + 1].hn);
    }
    // West
    ter = region->tile_arr[p->pos_i    ][p->pos_j - 1].ter;
    neighbor_cost = (p->cost == INT_MAX || travel_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + travel_times[ter][tnr];
    if ((path[p->pos_i    ][p->pos_j - 1].hn) &&
        (path[p->pos_i    ][p->pos_j - 1].cost > neighbor_cost)) {
      path[p->pos_i    ][p->pos_j - 1].cost = neighbor_cost;
      path[p->pos_i    ][p->pos_j - 1].from_i = p->pos_i;
      path[p->pos_i    ][p->pos_j - 1].from_j = p->pos_j;
      heap_decrease_key_no_replace(&h, path[p->pos_i    ][p->pos_j - 1].hn);
    }
    
    // if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
    //     (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
    //      ((p->cost + heightpair(p->pos)) *
    //       edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1)))) {
    //   path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
    //     ((p->cost + heightpair(p->pos)) *
    //      edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1));
    //   path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
    //   path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
    //   heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
    //                                        [p->pos[dim_x]    ].hn);
    // }
    // if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
    //     (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
    //      ((p->cost + heightpair(p->pos)) *
    //       edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y])))) {
    //   path[p->pos[dim_y]][p->pos[dim_x] - 1].cost =
    //     ((p->cost + heightpair(p->pos)) *
    //      edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y]));
    //   path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
    //   path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
    //   heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
    //                                        [p->pos[dim_x] - 1].hn);
    // }
    // if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
    //     (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
    //      ((p->cost + heightpair(p->pos)) *
    //       edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y])))) {
    //   path[p->pos[dim_y]][p->pos[dim_x] + 1].cost =
    //     ((p->cost + heightpair(p->pos)) *
    //      edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y]));
    //   path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
    //   path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
    //   heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
    //                                        [p->pos[dim_x] + 1].hn);
    // }
    // if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
    //     (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
    //      ((p->cost + heightpair(p->pos)) *
    //       edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1)))) {
    //   path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
    //     ((p->cost + heightpair(p->pos)) *
    //      edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1));
    //   path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
    //   path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
    //   heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
    //                                        [p->pos[dim_x]    ].hn);
    // }
  }
  printf("done with while");
}

void print_distance_map(int32_t region_x, int32_t region_y, enum trainer tnr) {
  a_star(region_ptr[region_x][region_y], tnr, 1, 1, pc.pos_i, pc.pos_j);
}

int main (int argc, char *argv[])
{
  // Handle random seed
  struct timeval t;
  uint32_t seed;
  if (argc == 2) {
    seed = atoi(argv[1]);
  } else {
    gettimeofday(&t, NULL);
    seed = (t.tv_usec ^ (t.tv_sec << 20)) & 0xffffffff;
  }
  printf("Using seed: %u\n", seed);
  srand(seed);

  // start in center of the world. 
  // The center of the world may also be referred to as (0,0)
  pc.region_x = WORLD_SIZE/2;
  pc.region_y = WORLD_SIZE/2;
  int32_t prev_region_x = pc.region_x;
  int32_t prev_region_y = pc.region_y;
  // Allocate memory for and generate the starting region
  region_t *new_region = malloc(sizeof(*new_region));
  region_ptr[pc.region_x][pc.region_y] = new_region;
  init_region(region_ptr[pc.region_x][pc.region_y], -1, -1, -1, -1, 1, 1);
  init_pc();
  printf("Current region (%d,%d)", pc.region_x - WORLD_SIZE/2, pc.region_y - WORLD_SIZE/2);
  printf(", player at (%d,%d)\n", pc.pos_i, pc.pos_j);
  print_region(region_ptr[pc.region_x][pc.region_y]);

  // Run game
  uint32_t running = 1;
  while(running) { 
    if (pc.region_x != prev_region_x || pc.region_y != prev_region_y) {
      load_region(pc.region_x, pc.region_y);

      print_distance_map(pc.region_x, pc.region_y, tnr_hiker);
      print_region(region_ptr[pc.region_x][pc.region_y]);
      prev_region_x = pc.region_x;
      prev_region_y = pc.region_y;
    }
    process_input(&pc.region_x, &pc.region_y, &running); 
  }

  free_all_regions();

  return 0;
}
