#ifndef REGION_H
#define REGION_H

#include <stdint.h>

#include "config.h"
#include "heap.h"

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

double dist(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
int32_t m_dist(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
int32_t rand_outcome(double probability) ;
void print_region (region_t *region, character_t *pc);
void init_region (region_t *region, 
                  int32_t N_exit_j, int32_t E_exit_i,
                  int32_t S_exit_j, int32_t W_exit_i,
                  int32_t place_center, int32_t place_mart,
                  int32_t numtrainers_opt);
#endif