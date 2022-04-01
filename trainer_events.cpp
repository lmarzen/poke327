#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "config.h"
#include "heap.h"
#include "region.h"
#include "global_events.h"
#include "trainer_events.h"

extern Region *region_ptr[WORLD_SIZE][WORLD_SIZE];
extern Pc *pc;
extern int32_t dist_map_hiker[MAX_ROW][MAX_COL];
extern int32_t dist_map_rival[MAX_ROW][MAX_COL];

static int32_t movetime_cmp(const void *key, const void *with) {
  return ((Character *) key)->get_movetime() 
         - ((Character *) with)->get_movetime();
}

/*
 * Initializes the priority queue with the player and trainers from a region
 */
void init_trainer_pq(heap_t *queue, Region *r) {
  heap_init(queue, movetime_cmp, NULL);
  heap_insert(queue, pc);
  for (auto it = r->get_npcs()->begin(); it != r->get_npcs()->end(); ++it) {
    heap_insert(queue, (Character*) &(*it));
  }
}

/*
 * Initiates and drives a battle
 */
void battle_driver(Character *opp, Pc *pc) {
  battle_t battle;
  battle.opp = opp;
  battle.pc = pc;
  battle.end_battle = 0;

  while (!battle.end_battle && !(pc->is_quit_game())) {
    render_battle(&battle);
    process_input_battle(&battle);
  }

  opp->set_defeated(true);
  return;
}

/*
 * Checks if a player is initiating a battle
 * Returns 1 if battle occured
 */
bool check_battle(int32_t to_i, int32_t to_j) {
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];

  for (auto it = r->get_npcs()->begin(); it != r->get_npcs()->end(); ++it) {
    if (it->get_i() == to_i
     && it->get_j() == to_j
     && !(it->is_defeated())) {
      battle_driver(&(*it), pc);
      return true;
    }
  }
  return false;
}

/*
 * Performs the all the nessesary checks to ensure a location is a valid spot to move to.
 * Returns 1 if the location is valid, 0 otherwise.
 */
bool is_valid_location(int32_t to_i, int32_t to_j, trainer_t tnr) {
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];

  if (turn_times[r->get_ter(to_i, to_j)][tnr] == INT_MAX) {
    return false;
  }
  if (pc->get_i() == to_i
   && pc->get_j() == to_j) {
    return false;
  }
  for (auto it = r->get_npcs()->begin(); it != r->get_npcs()->end(); ++it) {
    if (it->get_i() == to_i
     && it->get_j() == to_j) {
        return false;
      }
  }
  if (tnr != tnr_pc && ( 
     to_i <= 0 || to_i >= MAX_ROW - 1 ||
     to_j <= 0 || to_j >= MAX_COL - 1) ) {
    return false;
  }
  return true;
}

/*
 * Performs the all the nessesary checks to ensure a location is a valid spot to move to
 * except if the player is in that location.
 * Returns true if the location is valid or player exists there, false otherwise.
 */
bool is_valid_gradient(int32_t to_i, int32_t to_j, int32_t dist_map[MAX_ROW][MAX_COL]) {
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];
  
  if (dist_map[to_i][to_j] == INT_MAX) {
    return false;
  }
  for (auto it = r->get_npcs()->begin(); it != r->get_npcs()->end(); ++it) {
    if (it->get_i() == to_i
     && it->get_j() == to_j) {
        return false;
    }
  }
  return true;
}

/*
 * Move a trainer along the maximum gradient
 *
 * Friend of Character so that trainer locations can be updated 
 */
void move_along_gradient(Character *c, int32_t dist_map[MAX_ROW][MAX_COL]) {
  int32_t next_i = 0;
  int32_t next_j = 0;
  int32_t max_gradient = INT_MAX;

  // North
  if (is_valid_gradient(c->pos_i - 1, c->pos_j    , dist_map)
            && dist_map[c->pos_i - 1][c->pos_j    ] < max_gradient) {
    max_gradient = dist_map[c->pos_i - 1][c->pos_j    ];
    next_i = -1;
    next_j = 0;
  }
  // East
  if (is_valid_gradient(c->pos_i    , c->pos_j + 1, dist_map)
            && dist_map[c->pos_i    ][c->pos_j + 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i    ][c->pos_j + 1];
    next_i = 0;
    next_j = 1;
  }
  // South
  if (is_valid_gradient(c->pos_i + 1, c->pos_j    , dist_map)
            && dist_map[c->pos_i + 1][c->pos_j    ] < max_gradient) {
    max_gradient = dist_map[c->pos_i + 1][c->pos_j    ];
    next_i = 1;
    next_j = 0;
  }
  // West
  if (is_valid_gradient(c->pos_i    , c->pos_j - 1, dist_map)
            && dist_map[c->pos_i    ][c->pos_j - 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i    ][c->pos_j - 1];
    next_i = 0;
    next_j = -1;
  }
  // North East
  if (is_valid_gradient(c->pos_i - 1, c->pos_j + 1, dist_map)
            && dist_map[c->pos_i - 1][c->pos_j + 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i - 1][c->pos_j + 1];
    next_i = -1;
    next_j = 1;
  }
  // South East
  if (is_valid_gradient(c->pos_i + 1, c->pos_j + 1, dist_map)
            && dist_map[c->pos_i + 1][c->pos_j + 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i + 1][c->pos_j + 1];
    next_i = 1;
    next_j = 1;
  }
  // South West
  if (is_valid_gradient(c->pos_i + 1, c->pos_j - 1, dist_map)
            && dist_map[c->pos_i + 1][c->pos_j - 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i + 1][c->pos_j - 1];
    next_i = 1;
    next_j = -1;
  }
  // North West
  if (is_valid_gradient(c->pos_i - 1, c->pos_j - 1, dist_map)
            && dist_map[c->pos_i - 1][c->pos_j - 1] < max_gradient) {
    max_gradient = dist_map[c->pos_i - 1][c->pos_j - 1];
    next_i = -1;
    next_j = -1;
  }

  // Check if initiating a battle
  if (pc->get_i() == (c->pos_i + next_i)
   && pc->get_j() == (c->pos_j + next_j)) {
    battle_driver(c, pc);
    return;
  }

  c->pos_i += next_i;
  c->pos_j += next_j;
  return;
}

/*
 * Step player character and npc movetimes by amount
 */
void step_all_movetimes(Region *r, int32_t amount) {
  if (amount == 0) {
    return;
  }
  pc->step_movetime(amount);
  for (auto it = r->get_npcs()->begin(); it != r->get_npcs()->end(); ++it) {
    it->step_movetime(amount);
  }
  return;
}

/*
 * Process the player's movement attempt
 *
 * Returns int32_t for no_op.
 * A return value of 0 indicates players turn is over.
 *  
 * Friend of Pc (Player Character) class so that we can move the player
 */
int32_t process_pc_move_attempt(direction_t dir) {
  if (check_battle(pc->pos_i + dir_offsets[dir][0], 
                   pc->pos_j + dir_offsets[dir][1])) {
    return 0;
  }

  if (is_valid_location(pc->pos_i + dir_offsets[dir][0], 
                        pc->pos_j + dir_offsets[dir][1], tnr_pc)) {
    pc->pos_i += dir_offsets[dir][0];
    pc->pos_j += dir_offsets[dir][1];
    
    if (pc->pos_i == 0) {
      ++(pc->reg_y);
    } else if (pc->pos_i == MAX_ROW - 1) {
      --(pc->reg_y);
    } else if (pc->pos_j == 0) {
      --(pc->reg_x);
    } else if (pc->pos_j == MAX_COL - 1) {
      ++(pc->reg_x);
    }
    return 0;
  }
  
  return 1;
}
