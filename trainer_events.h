#ifndef TRAINER_EVENTS_H
#define TRAINER_EVENTS_H

#include <cstdint>

#include "heap.h"
#include "region.h"
#include "character.h"
#include "pokemon.h"

typedef enum battle {
  trainer,
  encounter
} battle_t;

void init_trainer_pq(heap_t *queue, Region *r);
void battle_trainer_driver(Character *opp, Pc *pc);
bool check_battle(int32_t to_i, int32_t to_j);
void encounter_driver(Pc *pc);
void check_encounter (Pc *pc);
bool is_valid_location(int32_t to_i, int32_t to_j, trainer_t tnr);
bool is_valid_gradient(int32_t to_i, int32_t to_j, 
                       int32_t dist_map[MAX_ROW][MAX_COL]);
void move_along_gradient(Character *c, int32_t dist_map[MAX_ROW][MAX_COL]);
void step_all_movetimes(Region *r, int32_t amount);
int32_t process_pc_move_attempt(direction_t dir);
int32_t bag_driver();
void party_view_driver();



#endif