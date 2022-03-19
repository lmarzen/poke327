#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "heap.h"
#include "region.h"
#include "trainer_events.h"

extern int32_t dist_map_hiker[MAX_ROW][MAX_COL];
extern int32_t dist_map_rival[MAX_ROW][MAX_COL];

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
    // for (int32_t k = 0; k < region->num_npc; ++k) {
    //   if (region->npc_arr[k].pos_i <= pc->pos_i + 1
    //   && region->npc_arr[k].pos_i >= pc->pos_i - 1
    //   && region->npc_arr[k].pos_j <= pc->pos_j + 1
    //   && region->npc_arr[k].pos_j >= pc->pos_j - 1) {
    //       init_pc(c, region);
    //   }
    // }
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