#include <cstdlib>

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
      battle_driver(this, pc);
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
      battle_driver(this, pc);
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
      battle_driver(this, pc);
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


/*******************************************************************************
* Player Character (Pc) Sub-Class
*******************************************************************************/
/*
 * Player character contructor
 */
Pc::Pc(int32_t r_x, int32_t r_y) {
  ch    = CHAR_PC;
  color = CHAR_COLOR_PC;
  tnr   = tnr_pc;
  reg_x = r_x;
  reg_y = r_y;

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


/*******************************************************************************
* Non-Player Character (Npc) Sub-Class
*******************************************************************************/
// (placeholder)
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
