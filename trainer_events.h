#ifndef TRAINER_EVENTS_H
#define TRAINER_EVENTS_H

#include <stdint.h>

#include "heap.h"
#include "region.h"

typedef struct battle {
  character_t *pc;
  character_t *opp;
  int32_t end_battle;
} battle_t;

void init_trainer_pq(heap_t *queue, character_t *pc, region_t *region);
void move_trainer(character_t *c, region_t *region, character_t *pc);
void step_all_movetimes(character_t *pc, region_t *region, int32_t amount);
void process_movement_turn(character_t *c, int32_t *region_x, int32_t *region_y, character_t *pc, int32_t *quit_game);
void process_pc_move_attempt(character_t *c, direction_t dir, region_t *region, int32_t *quit_game);

#endif