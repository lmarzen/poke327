#ifndef TRAINER_EVENTS_H
#define TRAINER_EVENTS_H

#include <cstdint>

#include "heap.h"
#include "region.h"
#include "character.h"
#include "pokemon.h"
#include "items.h"

typedef enum battle {
  trainer,
  encounter
} battle_t;

void init_trainer_pq(heap_t *queue, Region *r);
item_t bag_driver();
void battle_driver(Pc *pc, Character *opp);
bool check_battle(int32_t to_i, int32_t to_j);
void check_encounter (Pc *pc);
bool is_valid_location(int32_t to_i, int32_t to_j, trainer_t tnr);
bool is_valid_gradient(int32_t to_i, int32_t to_j, 
                       int32_t dist_map[MAX_ROW][MAX_COL]);
void move_along_gradient(Character *c, int32_t dist_map[MAX_ROW][MAX_COL]);
void step_all_movetimes(Region *r, int32_t amount);
int32_t process_pc_move_attempt(direction_t dir);
int32_t party_view_driver(int32_t scenario);
int32_t use_item(Character *user, Pokemon *user_poke, Pokemon *opp_poke,
                 item_t item);



#endif