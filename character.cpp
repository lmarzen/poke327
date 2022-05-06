#include <cmath>
#include <cstdlib>
#include <cstring>
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
    if (!defeated && !pc->is_defeated()) {
      move_along_gradient(this, dist_map_hiker);
    }
    break;
  case tnr_rival:
    if (!defeated && !pc->is_defeated()) {
      move_along_gradient(this, dist_map_rival);
    }
    break;
  case tnr_pacer:
    if (defeated || pc->is_defeated()) {
      // do nothing
    } else if (pc->get_i() == (pos_i + dir_offsets[dir][0])
            && pc->get_j() == (pos_j + dir_offsets[dir][1])) {
      battle_driver(pc, this);
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
    if (defeated || pc->is_defeated()) {
      // do nothing
      } else if (pc->get_i() == (pos_i + dir_offsets[dir][0])
              && pc->get_j() == (pos_j + dir_offsets[dir][1])) {
      battle_driver(pc, this);
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
    if (defeated || pc->is_defeated()) {
      // do nothing
    } else if (pc->get_i() == (pos_i + dir_offsets[dir][0])
            && pc->get_j() == (pos_j + dir_offsets[dir][1])) {
      battle_driver(pc, this);
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
  if (party_size < 6) {
    p->set_has_owner(true);
    party[party_size] = p;
    ++party_size;
    return 1;
  }
  return 0;
}
Pokemon* Character::get_pokemon(int32_t i) {
  if (i < 0 || i > party_size) {
    return NULL;
  }
  return party[i];
}
int32_t Character::get_party_size() {
  return party_size;
}
/*
 * Returns the index of the first non-fainted pokemon in party,
 * -1 if all are fainted
 */
int32_t Character::get_active_pokemon_index() {
  for (int32_t i = 0; i < party_size; ++i)
    if (!party[i]->is_fainted())
      return i;
  return -1;
}
/*
 * Returns a pointer to the first non-fainted pokemon in party, 
 * NULL if all are fainted
 */
Pokemon* Character::get_active_pokemon() {
  for (int32_t i = 0; i < party_size; ++i)
    if (!party[i]->is_fainted())
      return party[i];
  return NULL;
}
const char* Character::get_nickname() {
  return nickname;
}
void Character::rename(char new_name[13]) {
  strncpy(nickname, new_name, 12);
  return;
}
/*
 * Switches the order of pokemon in a trainers party
 */
void Character::switch_pokemon(int32_t a, int32_t b) {
  if (a < 0 || a >= party_size || b < 0 || b >= party_size || a == b)
    return;

  Pokemon *tmp = party[a];
  party[a] = party[b];
  party[b] = tmp;
  return;
}
/*
 * Calculate a character's payout for losing in battle
 * Using gen II mechanics
 * https://bulbapedia.bulbagarden.net/wiki/Prize_money
 */
int32_t Character::get_payout() {
  int32_t p;
  if (tnr != tnr_pc) {
    p = base_payout[tnr] * party[party_size - 1]->get_level();
  } else {
    // player character
    int32_t dist = m_dist(WORLD_SIZE/2, WORLD_SIZE/2, pc->get_x(), pc->get_y());
    // custom formula to increase payout with distance
    int32_t base = 4 * pow(dist, 0.7835) + 16;
    int32_t level = 1;
    for (int32_t i = 0; i < party_size; ++i) {
      if (party[i]->get_level() > level) {
        level = party[i]->get_level();
      }
    }
    p = base * level;
    if (p > pc->get_poke_dollars()) {
      p = pc->get_poke_dollars();
    }
  }
  return p;
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
  party_size = 0;
  strncpy(nickname, "PLAYER", 12);
  
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
  add_item_to_bag(item_poke_ball,    START_POKE_BALL);
  add_item_to_bag(item_great_ball,   START_GREAT_BALL);
  add_item_to_bag(item_ultra_ball,   START_ULTRA_BALL);
  add_item_to_bag(item_master_ball,  START_MASTER_BALL);
  add_item_to_bag(item_potion,       START_POTION);
  add_item_to_bag(item_super_potion, START_SUPER_POTION);
  add_item_to_bag(item_hyper_potion, START_HYPER_POTION);
  add_item_to_bag(item_max_potion,   START_MAX_POTION);
  add_item_to_bag(item_ether,        START_ETHER);
  add_item_to_bag(item_max_ether,    START_MAX_ETHER);
  add_item_to_bag(item_elixir,       START_ELIXIR);
  add_item_to_bag(item_max_elixir,   START_MAX_ELIXIR);
  add_item_to_bag(item_revive,       START_REVIVE);
  add_item_to_bag(item_max_revive,   START_MAX_REVIVE);
  add_item_to_bag(item_rare_candy,   START_RARE_CANDY);
}

Pc::~Pc() {
  for (int32_t i = 0; i < party_size; ++i)
    free(party[i]);
}

int32_t Pc::get_x() {
  return reg_x;
}
int32_t Pc::get_y() {
  return reg_y;
}

void Pc::pick_starter_driver() {
  int32_t scroller_pos = 0;
  int32_t selected_pokemon = 0;
  
  Pokemon *p1 = new Pokemon();
  Pokemon *p2 = new Pokemon();
  // ensure no duplicate starter options
  while (p2->get_pd_entry()->id == p1->get_pd_entry()->id) {
    delete p2;
    p2 = new Pokemon();
  }
  Pokemon *p3 = new Pokemon();
  // ensure no duplicate starter options
  while (p3->get_pd_entry()->id == p1->get_pd_entry()->id
      || p3->get_pd_entry()->id == p2->get_pd_entry()->id) {
    delete p3;
    p3 = new Pokemon();
  }

  while (!selected_pokemon) {
    render_pick_starter(scroller_pos, p1, p2, p3);
    process_input_pick_starter(&scroller_pos, &selected_pokemon);
  }

  // clean up starters that were not picked
  if (selected_pokemon != 1)
    delete p1;
  if (selected_pokemon != 2)
    delete p2;
  if (selected_pokemon != 3)
    delete p3;

  if (selected_pokemon == 1) {
    add_pokemon(p1);
  } else if (selected_pokemon == 2) {
    add_pokemon(p2);
  } else if (selected_pokemon == 3) {
    add_pokemon(p3);
  }

  return;
}

int32_t Pc::get_poke_dollars() {
  return poke_dollars;
}
void Pc::give_poke_dollars(int32_t amount) {
  poke_dollars += amount;
  return;
}
void Pc::take_poke_dollars(int32_t amount) {
  poke_dollars -= amount;
  if (poke_dollars < 0) {
    poke_dollars = 0;
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
  party_size = 0;

  switch (tnr) {
    case tnr_hiker:
    strncpy(nickname, "HIKER", 12);
      ch = CHAR_HIKER;
      color = CHAR_COLOR_HIKER;
      // dir is unused for this trainer type
      break;
    case tnr_rival:
      strncpy(nickname, "RIVAL", 12);
      ch = CHAR_RIVAL;
      color = CHAR_COLOR_RIVAL;
      // dir is unused for this trainer type
      break;
    case tnr_pacer:
      strncpy(nickname, "PACER", 12);
      ch = CHAR_PACER;
      color = CHAR_COLOR_PACER;
      dir = static_cast<direction_t>(rand() % 8);
      break;
    case tnr_wanderer:
      strncpy(nickname, "WANDERER", 12);
      ch = CHAR_WANDERER;
      color = CHAR_COLOR_WANDERER;
      dir = static_cast<direction_t>(rand() % 8);
      break;
    case tnr_stationary:
      strncpy(nickname, "STATIONARY", 12);
      ch = CHAR_STATIONARY;
      color = CHAR_COLOR_STATIONARY;
      // dir is unused for this trainer type
      break;
    case tnr_rand_walker:
      strncpy(nickname, "WALKER", 12);
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
  for (int32_t i = 0; i < party_size; ++i)
    free(party[i]);
}
