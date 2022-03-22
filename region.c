#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "region.h"

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
 * LEGACY CODE
 * See buffer_region() in global_events.c
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
        new_npc_arr[m].defeated = 0;
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