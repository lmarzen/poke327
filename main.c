#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_ROW 21
#define MAX_COL 80

#define MIN_SEEDS_PER_REGION 8 // at least 2 grass and 2 clearings seeds. (req)
#define MAX_SEEDS_PER_REGION 16

enum tile {
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

typedef struct seed {
  int32_t i;
  int32_t j;
  enum tile ter;
} seed_t;

double distance(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
   return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void print_region (enum tile (*region)[MAX_COL]) {
  int32_t i,j;
  int32_t randy;

  for (i = 0; i < MAX_ROW; i++) {
    for (j = 0; j < MAX_COL; j++) {
      switch (region[i][j]) {
        case ter_boulder:
          printf("%%");
          break;
        case ter_tree:
          printf("^");
          break;
        case ter_center:
          printf("C");
          break;
        case ter_mart:
          printf("M");
          break;
        case ter_path:
          printf("#");
          break;
        case ter_grass0:
        case ter_grass1:
        case ter_grass2:
        case ter_grass3:
          printf(":");
          break;
        case ter_clearing0:
        case ter_clearing1:
        case ter_clearing2:
        case ter_clearing3:
          printf(".");
          break;
        case ter_mixed0:
        case ter_mixed1:
        case ter_mixed2:
        case ter_mixed3:
          randy = rand() % 10;
          if (randy <= 3) {// 40% chance of grass
            printf(":");
          } else if (randy >= 4 && randy <= 6) { //30% chance of clearing
            printf(",");
          } else if (randy >= 7 && randy <= 8) { //20% chance of tree
            printf("^");
          }  else { //10% chance of boulder
            printf("%%");
          }
          break;
        default:
          printf("U"); // Undefined terrain, should never occur
          break;
      }
    }
    printf("\n");
  }
}

void init_region (enum tile (*region)[MAX_COL]) {
  struct timeval t;
  gettimeofday(&t, NULL);
  srand(t.tv_usec * t.tv_sec);
  
  // create a random number of random seeds
  int32_t num_seeds = (rand() % (MAX_SEEDS_PER_REGION - MIN_SEEDS_PER_REGION + 1)) 
                      + MIN_SEEDS_PER_REGION;

  // allocate memory for seeds, each seed has x and y
  seed_t (*seeds)[num_seeds] = malloc(sizeof(*seeds) * num_seeds);
  
  // initialize each seed with a random set of cordinates
  // at least 2 grass and 2 clearings seeds. (req)
  seeds[0]->i = rand() % MAX_ROW;
  seeds[0]->j = rand() % MAX_COL;
  seeds[0]->ter = ter_clearing0;
  seeds[1]->i = rand() % MAX_ROW;
  seeds[1]->j = rand() % MAX_COL;
  seeds[1]->ter = ter_clearing1;
  seeds[2]->i = rand() % MAX_ROW;
  seeds[2]->j = rand() % MAX_COL;
  seeds[2]->ter = ter_grass0; 
  seeds[3]->i = rand() % MAX_ROW;
  seeds[3]->j = rand() % MAX_COL;
  seeds[3]->ter = ter_grass1; 

  //  remaining seeds get random terrain type
  for (int32_t i = 4; i < num_seeds; i++) {
    seeds[i]->i = rand() % MAX_ROW;
    seeds[i]->j = rand() % MAX_COL;
    seeds[i]->ter = (rand() % (ter_mixed3 - ter_grass0 + 1)) + ter_grass0; 
  }

  // populate each tile by assigning it the terrain type of the closest seed
  // does not calculate for the outer most boarder because this space will 
  // be boulders or path.
  for (int32_t i = 0; i < MAX_ROW ; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      if (i == 0 || i == MAX_ROW - 1 || j == 0 || j == MAX_COL - 1) {
        region[i][j] = ter_boulder;
      } else {
        seed_t *closest_seed = seeds[0];
        double closest_dist = distance(seeds[0]->j, seeds[0]->i, j, i);

        for (int32_t k = 1; k < num_seeds; k++) {
          double temp_dist = distance(seeds[k]->j, seeds[k]->i, j, i);
          if (temp_dist < closest_dist) {
            closest_seed = seeds[k];
            closest_dist = temp_dist;
          }
        }

        region[i][j] = closest_seed->ter;
      }
    }
  }

  free (seeds);
  return;
}

int main (int argc, char *argv[])
{
  // Allocate memory for a region
  enum tile (*region)[MAX_COL] = malloc(sizeof (*region) * MAX_ROW);

  init_region(region);
  print_region(region);

  // free(region);
  // note ... int num = (rand() % (upper - lower + 1)) + lower;

  printf("%d\n", rand());


  return 0;
}