#ifndef TRAINER_EVENTS_H
#define TRAINER_EVENTS_H

#include <cstdint>

#include "heap.h"
#include "region.h"
#include "character.h"

typedef struct battle {
  Character *pc;
  Character *opp;
  int32_t end_battle;
} battle_t;

typedef struct encounter {
  Character *pc;
  Character *wp;
  int32_t end_encounter;
} encounter_t;

void init_trainer_pq(heap_t *queue, Region *r);
void battle_driver(Character *opp, Pc *pc);
bool check_battle(int32_t to_i, int32_t to_j);
void encounter_driver(Pc *pc);
void check_encounter (Pc *pc);
bool is_valid_location(int32_t to_i, int32_t to_j, trainer_t tnr);
bool is_valid_gradient(int32_t to_i, int32_t to_j, 
                       int32_t dist_map[MAX_ROW][MAX_COL]);
void move_along_gradient(Character *c, int32_t dist_map[MAX_ROW][MAX_COL]);
void step_all_movetimes(Region *r, int32_t amount);
int32_t process_pc_move_attempt(direction_t dir);


#endif