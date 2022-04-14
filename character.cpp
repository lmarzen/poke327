#include <cstdlib>
#include <ncurses.h>

#include "character.h"
#include "region.h"
#include "trainer_events.h"
#include "global_events.h"

extern Region *region_ptr[WORLD_SIZE][WORLD_SIZE];
extern Pc *pc;
extern int32_t dist_map_hiker[MAX_ROW][MAX_COL];
extern int32_t dist_map_rival[MAX_ROW][MAX_COL];

/*******************************************************************************
* Abstract Character Base-Class
*******************************************************************************/
int32_t Character::get_movetime() {
  return movetime;
}
void Character::step_movetime(int32_t amount) {
    movetime -= amount;
}
int32_t Character::get_i() {
    return pos_i;
}
int32_t Character::get_j() {
    return pos_j;
}
char Character::get_ch() {
    return ch;
}
int32_t Character::get_color() {
    return color;
}
trainer_t Character::get_tnr() {
    return tnr;
}
bool Character::is_defeated() {
    return defeated;
}
void Character::set_defeated(bool d) {
    defeated = d;
    return;
}

/*
 * Process a trainer's turn
 */
void Character::process_movement_turn() {
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];

  // update before moving to avoid issues with player changing regions
  movetime = turn_times[ r->get_ter(pos_i, pos_j) ][ tnr ];

  switch (tnr)
  {
  case tnr_pc:
    render_region(r);
    process_input_nav();
    break;
  case tnr_hiker:
    if (!defeated) {
      move_along_gradient(this, dist_map_hiker);
    }
    break;
  case tnr_rival:
    if (!defeated) {
      move_along_gradient(this, dist_map_rival);
    }
    break;
  case tnr_pacer:
    if (defeated) {
      // do nothing
    } else if (pc->get_i() == (pos_i + dir_offsets[dir][0])
            && pc->get_j() == (pos_j + dir_offsets[dir][1])) {
      battle_trainer_driver(this, pc);
    } else if (is_valid_location(pos_i + dir_offsets[dir][0], 
                                 pos_j + dir_offsets[dir][1], 
                                 tnr)) {
      pos_i += dir_offsets[dir][0];
      pos_j += dir_offsets[dir][1];
    } else {
      dir = static_cast<direction_t>((dir + 4) % 8);
    }
    break;
  case tnr_wanderer:
    if (defeated) {
      // do nothing
      } else if (pc->get_i() == (pos_i + dir_offsets[dir][0])
              && pc->get_j() == (pos_j + dir_offsets[dir][1])) {
      battle_trainer_driver(this, pc);
    } else if ((r->get_ter(pos_i + dir_offsets[dir][0], 
                           pos_j + dir_offsets[dir][1])
             == r->get_ter(pos_i                      , 
                           pos_j                      ))
      && is_valid_location(pos_i + dir_offsets[dir][0],
                           pos_j + dir_offsets[dir][1], tnr)) {
      pos_i += dir_offsets[dir][0];
      pos_j += dir_offsets[dir][1];
    } else {
      dir = static_cast<direction_t>(rand() % 8);
    }
    break;
  case tnr_stationary:
    // Do nothing
    break;
  case tnr_rand_walker:
    if (defeated) {
      // do nothing
    } else if (pc->get_i() == (pos_i + dir_offsets[dir][0])
            && pc->get_j() == (pos_j + dir_offsets[dir][1])) {
      battle_trainer_driver(this, pc);
    } else if (is_valid_location(pos_i + dir_offsets[dir][0],
                                 pos_j + dir_offsets[dir][1], tnr)) {
      pos_i += dir_offsets[dir][0];
      pos_j += dir_offsets[dir][1];
    } else {
      dir = static_cast<direction_t>(rand() % 8);
    }
    break;
  default:
    char m[MAX_COL];
    sprintf(m,"Error: Movement for trainer type %d has not been implemented!", 
            tnr);
    exit_w_message(m);
    break;
  }
  return;
}
int32_t Character::num_in_bag(item_t i) {
  for (auto it = bag.begin(); it != bag.end(); ++it) {
    if (it->item == i) {
      return it->cnt;
    }
  }
  return 0;
}
void Character::remove_item_from_bag(item_t i) {
  for (auto it = bag.begin(); it != bag.end(); ++it) {
    if (it->item == i) {
      --it->cnt;
      if (it->cnt < 1) {
        bag.erase(it);
      }
      return;
    }
  }

  return;
}
void Character::add_item_to_bag(item_t i, int32_t cnt) {
  // check if count is a positive number that is less than MAX_ITEMS
  // check if bag_slot already exists
  //   if (bag_slot exists)
  //     add cnt to bag_slot
  //     return
  // create new bag_slot
  
  if (cnt < 1 || cnt > MAX_ITEMS) {
    return;
  }

  for (auto it = bag.begin(); it != bag.end(); ++it) {
    if (it->item == i) {
      it->cnt += cnt;
      if (cnt > MAX_ITEMS) {
        it->cnt = MAX_ITEMS;
      }
      return;
    }
  }

  bag_slot_t new_bag_slot;
  new_bag_slot.item = i;
  new_bag_slot.cnt = cnt;
  bag.push_back(new_bag_slot);
}
int32_t Character::num_bag_slots() {
  return bag.size();
}
bag_slot_t Character::peek_bag_slot(int32_t index) {
  return bag[index];
}
/*
 * Return 1 if pokemon was successfully added to the party, 0 otherwise
 */
int32_t Character::add_pokemon(Pokemon *p) {
  if (party.size() < 6) {
    party.push_back(*p);
    return 1;
  }
  return 0;
}
Pokemon* Character::get_pokemon(int32_t i) {
  return &party[i];
}
int32_t Character::party_size() {
  return party.size();
}


/*******************************************************************************
* Player Character (Pc) Sub-Class
*******************************************************************************/
/*
 * Player Character contructor
 */
Pc::Pc(int32_t r_x, int32_t r_y) {
  ch    = CHAR_PC;
  color = CHAR_COLOR_PC;
  tnr   = tnr_pc;
  reg_x = r_x;
  reg_y = r_y;
  movetime = 0;
  
  // get pointer to present region from global variables
  Region *r = region_ptr[reg_x][reg_y];

  // Find a valid spawn location
  int32_t found_location = 0;
  while (found_location != 1) {
    pos_i = (rand() % (MAX_ROW - 2)) + 1;
    pos_j = (rand() % (MAX_COL - 2)) + 1;
    if (r->get_ter(pos_i, pos_j) == ter_path) {
      found_location = 1;
      for (auto it = r->get_npcs()->begin(); it != r->get_npcs()->end(); ++it) {
        if (it->get_i() == pos_i && it->get_j() == pos_j) {
          found_location = 0;
          break;
        }
      }
    }
  }
  movetime = turn_times[r->get_ter(pos_i, pos_j)][tnr];


  // give the player starting items
  add_item_to_bag(item_potion, 10);
  add_item_to_bag(item_revive, 10);
  add_item_to_bag(item_pokeball, 10);
}

Pc::~Pc() {
  party.clear();
}

int32_t Pc::get_x() {
  return reg_x;
}
int32_t Pc::get_y() {
  return reg_y;
}
bool Pc::is_quit_game() {
  return quit_game;
}
void Pc::set_quit_game(bool q) {
  quit_game = q;
  return;
}

void Pc::pick_starter_driver() {
  int32_t scroller_pos = 0;
  int32_t selected_pokemon = 0;
  Pokemon *p1 = new Pokemon();
  Pokemon *p2 = new Pokemon();
  Pokemon *p3 = new Pokemon();

  while (!selected_pokemon) {
    render_pick_starter(scroller_pos, p1, p2, p3);
    process_input_pick_starter(&scroller_pos, &selected_pokemon);
  }

  if (selected_pokemon == 1) {
    add_pokemon(p1);
  } else if (selected_pokemon == 2) {
    add_pokemon(p2);
  } else if (selected_pokemon == 3) {
    add_pokemon(p3);
  }

  return;
}

/*******************************************************************************
* Non-Player Character (Npc) Sub-Class
*******************************************************************************/
/*
 * Non-Player Character contructor
 */
Npc::Npc(trainer_t tnr, int32_t i, int32_t j, int32_t init_movetime) {
  pos_i = i;
  pos_j = j;
  movetime = init_movetime;
  this->tnr = tnr;

  switch (tnr) {
    case tnr_hiker:
      ch = CHAR_HIKER;
      color = CHAR_COLOR_HIKER;
      // dir is unused for this trainer type
      break;
    case tnr_rival:
      ch = CHAR_RIVAL;
      color = CHAR_COLOR_RIVAL;
      // dir is unused for this trainer type
      break;
    case tnr_pacer:
      ch = CHAR_PACER;
      color = CHAR_COLOR_PACER;
      dir = static_cast<direction_t>(rand() % 8);
      break;
    case tnr_wanderer:
      ch = CHAR_WANDERER;
      color = CHAR_COLOR_WANDERER;
      dir = static_cast<direction_t>(rand() % 8);
      break;
    case tnr_stationary:
      ch = CHAR_STATIONARY;
      color = CHAR_COLOR_STATIONARY;
      // dir is unused for this trainer type
      break;
    case tnr_rand_walker:
      ch = CHAR_RAND_WALKER;
      color = CHAR_COLOR_RAND_WALKER;
      dir = static_cast<direction_t>(rand() % 8);
      break;
    default:
      char m[MAX_COL];
      sprintf(m,"Error: Unhandled trainer type %d", tnr);
      exit_w_message(m);
  }
}

Npc::~Npc() {
  party.clear();
}
