#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_ROW 21
#define MAX_COL 80

#define MIN_SEEDS_PER_REGION 6 // at least 2 grass and 2 clearings seeds. (req)
#define MAX_SEEDS_PER_REGION 16

#define CHAR_BOULDER '%';
#define CHAR_TREE '^';
#define CHAR_CENTER 'C';
#define CHAR_MART 'M';
#define CHAR_PATH '#';
#define CHAR_GRASS ':';
#define CHAR_CLEARING '.';

enum terrain {
  ter_boulder,
  ter_tree,
  ter_center,
  ter_mart,
  ter_path,
  ter_grass0,
  ter_grass1,
  ter_grass2,
  ter_grass3,
  ter_clearing0,
  ter_clearing1,
  ter_clearing2,
  ter_clearing3,
  ter_mixed0,
  ter_mixed1,
  ter_mixed2,
  ter_mixed3
};

typedef struct tile {
  enum terrain ter;
  char ch;
} tile_t;

typedef struct seed {
  int32_t i;
  int32_t j;
  enum terrain ter;
} seed_t;

typedef struct pos {
  int32_t i;
  int32_t j;
} pos_t;

typedef struct region {
  tile_t tile_arr[MAX_ROW][MAX_COL];
  int32_t N_exit_j, E_exit_i, S_exit_j, W_exit_i;
} region_t;

/*
 * returns the distance between 2 points
 */
double distance(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
   return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

/*
 * Print a region
 */
void print_region (region_t *region) {
  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      printf("%c", region->tile_arr[i][j].ch);
    }
    printf("\n");
  }
}

/*
 * Initialize a region
 * Pathes exit points will be generated as specified.
 * For random path exit point specify -1
 */
void init_region (region_t *region, 
                  int32_t N_exit_j, int32_t E_exit_i,
                  int32_t S_exit_j, int32_t W_exit_i) {
  struct timeval t;
  gettimeofday(&t, NULL);
  srand(t.tv_usec * t.tv_sec);
  int32_t randy;

  // create a random number of random seeds
  int32_t num_seeds = (rand() % (MAX_SEEDS_PER_REGION - MIN_SEEDS_PER_REGION + 1)) 
                      + MIN_SEEDS_PER_REGION;

  // allocate memory for seeds, each seed has x and y
  seed_t (*seed_arr)[num_seeds] = malloc(sizeof(*seed_arr) * num_seeds);
  
  // initialize each seed with a random set of cordinates
  // at least 2 grass and 2 clearings seeds. (req)
  seed_arr[0]->i = rand() % MAX_ROW;
  seed_arr[0]->j = rand() % MAX_COL;
  seed_arr[0]->ter = ter_clearing0;
  seed_arr[1]->i = rand() % MAX_ROW;
  seed_arr[1]->j = rand() % MAX_COL;
  seed_arr[1]->ter = ter_clearing1;
  seed_arr[2]->i = rand() % MAX_ROW;
  seed_arr[2]->j = rand() % MAX_COL;
  seed_arr[2]->ter = ter_grass0; 
  seed_arr[3]->i = rand() % MAX_ROW;
  seed_arr[3]->j = rand() % MAX_COL;
  seed_arr[3]->ter = ter_grass1; 

  //  remaining seeds get random terrain type
  for (int32_t i = 4; i < num_seeds; i++) {
    seed_arr[i]->i = rand() % MAX_ROW;
    seed_arr[i]->j = rand() % MAX_COL;
    seed_arr[i]->ter = (rand() % (ter_mixed3 - ter_grass0 + 1)) + ter_grass0; 
  }

  // populate each tile by assigning it the terrain type of the closest seed
  // does not calculate for the outer most boarder because this space will 
  // be boulders or path.
  for (int32_t i = 0; i < MAX_ROW ; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      if (i == 0 || i == MAX_ROW - 1 || j == 0 || j == MAX_COL - 1) {
        region->tile_arr[i][j].ter = ter_boulder;
        region->tile_arr[i][j].ch = CHAR_BOULDER;
      } else {
        seed_t *closest_seed = seed_arr[0];
        double closest_dist = distance(seed_arr[0]->j, seed_arr[0]->i, j, i);

        for (int32_t k = 1; k < num_seeds; k++) {
          double temp_dist = distance(seed_arr[k]->j, seed_arr[k]->i, j, i);
          if (temp_dist < closest_dist) {
            closest_seed = seed_arr[k];
            closest_dist = temp_dist;
          }
        }

        region->tile_arr[i][j].ter = closest_seed->ter;
        switch (region->tile_arr[i][j].ter) {
          case ter_grass0:
          case ter_grass1:
          case ter_grass2:
          case ter_grass3:
            region->tile_arr[i][j].ch = CHAR_GRASS;
            break;
          case ter_clearing0:
          case ter_clearing1:
          case ter_clearing2:
          case ter_clearing3:
            region->tile_arr[i][j].ch = CHAR_CLEARING;
            break;
          case ter_mixed0:
          case ter_mixed1:
          case ter_mixed2:
          case ter_mixed3:
            randy = rand() % 10;
            if (randy <= 3) {// 40%  grass
              region->tile_arr[i][j].ch = CHAR_GRASS;
            } else if (randy >= 4 && randy <= 6) { //30% clearing
              region->tile_arr[i][j].ch = CHAR_CLEARING;
            } else if (randy >= 7 && randy <= 8) { // 20% tree
              region->tile_arr[i][j].ch = CHAR_TREE;
            }  else { // 10% boulder
              region->tile_arr[i][j].ch = CHAR_BOULDER;
            }
            break;
          default:
            printf("E"); // Undefined terrain, should never occur
            break;
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
  region->tile_arr[0][region->N_exit_j].ch = CHAR_PATH;
  if (E_exit_i == -1) {
    region->E_exit_i = (rand() % (MAX_ROW - 2)) + 1;
  } else {
    region->E_exit_i = E_exit_i;
  }
  region->tile_arr[region->E_exit_i][MAX_COL - 1].ter = ter_path;
  region->tile_arr[region->E_exit_i][MAX_COL - 1].ch = CHAR_PATH;
  if (S_exit_j == -1) {
    region->S_exit_j = (rand() % (MAX_COL - 2)) + 1;
  } else {
    region->S_exit_j = S_exit_j;
  }
  region->tile_arr[MAX_ROW - 1][region->S_exit_j].ter = ter_path;
  region->tile_arr[MAX_ROW - 1][region->S_exit_j].ch = CHAR_PATH;
  if (W_exit_i == -1) {
    region->W_exit_i = (rand() % (MAX_ROW - 2)) + 1;
  } else {
    region->W_exit_i = W_exit_i;
  }
  region->tile_arr[region->W_exit_i][0].ter = ter_path;
  region->tile_arr[region->W_exit_i][0].ch = CHAR_PATH;

  // W->E path
  // prefers to generate paths between ter_types
  int32_t path_i = region->W_exit_i;
  int32_t path_j = 1;
  region->tile_arr[path_i][path_j].ter = ter_path;
  region->tile_arr[path_i][path_j].ch = CHAR_PATH;
  while (path_j != MAX_COL - 2) {
    // find the closest seed
    seed_t *closest_seed = seed_arr[0];
    double closest_dist = distance(seed_arr[0]->j, seed_arr[0]->i, path_j, path_i);
    for (int32_t k = 1; k < num_seeds; k++) {
      double temp_dist = distance(seed_arr[k]->j, seed_arr[k]->i, path_j, path_i);
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
    double dist_to_seed = distance(closest_seed->j, closest_seed->i, path_j, path_i);
    double dist_to_exit = distance(MAX_COL - 1, region->E_exit_i, path_j, path_i);
    E_path_weight = 0.2*(distance(closest_seed->j, closest_seed->i, path_j + 1, path_i) - dist_to_seed) // prefer terrain boarders
                  + 0.5*(rand() % 10); // ensure random progress is made towards exit
    if (path_i - 1 != 0 && 
        region->tile_arr[path_i - 1][path_j].ter != ter_path) {
      N_path_weight = 0.8*(distance(closest_seed->j, closest_seed->i, path_j,  path_i - 1) - dist_to_seed) // prefer terrain boarders
                    + 0.05*path_j*(dist_to_exit - distance(MAX_COL - 1, region->E_exit_i, path_j, path_i - 1)) // head towards the exit especially near the end
                    + 0.05*path_i; // dont hug walls
    }
    if (path_i + 1 != MAX_ROW - 1 &&
        region->tile_arr[path_i + 1][path_j].ter != ter_path) {
      S_path_weight = 0.8*(distance(closest_seed->j, closest_seed->i, path_j, path_i + 1) - dist_to_seed) // prefer terrain boarders
                    + 0.05*path_j*(dist_to_exit - distance(MAX_COL - 1, region->E_exit_i, path_j, path_i + 1)) //  head towards the exit especially near the end
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
    region->tile_arr[path_i][path_j].ch = CHAR_PATH;
  }

  while (path_i > region->E_exit_i) {
    --path_i;
    region->tile_arr[path_i][path_j].ter = ter_path;
    region->tile_arr[path_i][path_j].ch = CHAR_PATH;
  }
  while (path_i < region->E_exit_i) {
    ++path_i;
    region->tile_arr[path_i][path_j].ter = ter_path;
    region->tile_arr[path_i][path_j].ch = CHAR_PATH; 
  }

  // N->S path
  path_i = 1;
  path_j = region->N_exit_j;
  region->tile_arr[path_i][path_j].ter = ter_path;
  region->tile_arr[path_i][path_j].ch = CHAR_PATH;
  while (path_i != MAX_ROW - 2) {
    // find the closest seed
    seed_t *closest_seed = seed_arr[0];
    double closest_dist = distance(seed_arr[0]->j, seed_arr[0]->i, path_j, path_i);
    for (int32_t k = 1; k < num_seeds; k++) {
      double temp_dist = distance(seed_arr[k]->j, seed_arr[k]->i, path_j, path_i);
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
    double dist_to_seed = distance(closest_seed->j, closest_seed->i, path_j, path_i);
    double dist_to_exit = distance(region->S_exit_j, MAX_ROW - 1, path_j, path_i);
    S_path_weight = 0.2*(distance(closest_seed->j, closest_seed->i, path_j, path_i + 1) - dist_to_seed)
                  + 0.008*(pow((path_i - MAX_ROW), 2))
                  + 0.05*(rand() % 10);
    if (path_j + 1 != MAX_COL - 1 && 
        region->tile_arr[path_i][path_j + 1].ter != ter_path) {
      E_path_weight = 3*(distance(closest_seed->j, closest_seed->i, path_j + 1,  path_i) - dist_to_seed)
                    + 0.5*path_i*(dist_to_exit - distance(region->S_exit_j, MAX_ROW - 1, path_j + 1, path_i));
    }
    if (path_j - 1 != 0 &&
        region->tile_arr[path_i][path_j - 1].ter != ter_path) {
      W_path_weight = 3*(distance(closest_seed->j, closest_seed->i, path_j - 1, path_i) - dist_to_seed)
                    + 0.5*path_i*(dist_to_exit - distance(region->S_exit_j, MAX_ROW - 1, path_j - 1, path_i));
    }

    if(S_path_weight >= E_path_weight && S_path_weight >= W_path_weight) {
      ++path_i;
    } else if (E_path_weight >= W_path_weight) {
      ++path_j;
    } else {
      --path_j;
    }
    region->tile_arr[path_i][path_j].ter = ter_path;
    region->tile_arr[path_i][path_j].ch = CHAR_PATH;
  }

  while (path_j > region->S_exit_j) {
    --path_j;
    region->tile_arr[path_i][path_j].ter = ter_path;
    region->tile_arr[path_i][path_j].ch = CHAR_PATH;
  }
  while (path_j < region->S_exit_j) {
    ++path_j;
    region->tile_arr[path_i][path_j].ter = ter_path;
    region->tile_arr[path_i][path_j].ch = CHAR_PATH; 
  }

free (seed_arr);

  return;
}

int main (int argc, char *argv[])
{
  // Allocate memory for a region
  region_t *region = malloc(sizeof(region_t));

  init_region(region, -1, -1, -1, -1);
  print_region(region);

  free(region);
  // note ... int num = (rand() % (upper - lower + 1)) + lower;

  return 0;
}