#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "region.h"
#include "global_events.h"

extern region_t *region_ptr[WORLD_SIZE][WORLD_SIZE];

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