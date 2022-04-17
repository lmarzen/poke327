#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include "config.h"
#include "heap.h"
#include "region.h"
#include "global_events.h"
#include "trainer_events.h"
#include "items.h"

extern Region *region_ptr[WORLD_SIZE][WORLD_SIZE];
extern Pc *pc;
extern int32_t dist_map_hiker[MAX_ROW][MAX_COL];
extern int32_t dist_map_rival[MAX_ROW][MAX_COL];

#define NUM_CATCH_FAIL_TXT 4
const char catch_fail_txt[NUM_CATCH_FAIL_TXT][MAX_COL] {
  "Oh, no! The POKEMON broke free!",
  "Aww! It appeared to be caught!",
  "Aargh! Almost had it!",
  "Shoot! It was so close, too!"
};

#define NUM_CATCH_ILLEGAL_TXT 4
const char catch_illegal_txt[NUM_CATCH_ILLEGAL_TXT][MAX_COL] {
  "The TRAINER blocked the BALL!",
  "Don't be a thief!",
  "It dodged the thrown BALL! This POKEMON can't be caught!",
  "You missed the POKEMON!"
};

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
 * Drives player bag interactions when the bag is opened
 * Returns 0 if an item was used. 1 if no item was used.
 */
item_t bag_driver() {
  int32_t close_bag = 0;
  int32_t scroller_pos = 0;
  int32_t page_index = 0;
  int32_t item_selected = -1;

  while (!close_bag && item_selected == -1) {
    render_bag(page_index, scroller_pos);
    process_input_bag(&page_index, &scroller_pos, &close_bag, &item_selected);
  }
  if (item_selected != -1) {
    return pc->peek_bag_slot(item_selected).item;
  }
  return item_empty;
}

/*
 * Return value indicates if the action counts as the user's turn.
 * 0 indicates that the user has used its turn, 1 if the action does not count
 * as the user's turn.
 */
int32_t use_item(Character *user, Pokemon *user_poke, Pokemon *opp_poke,
                 item_t item) {
  char m[MAX_COL];
  int32_t poke_index;

  switch (item) {
    case item_pokeball:
      if (opp_poke != NULL) {
        // we are in a battle
        user->remove_item_from_bag(item);
        if (opp_poke->get_has_owner()) {
          // We are in a battle with another trainer
          sprintf(m, "%s", catch_illegal_txt[rand() % NUM_CATCH_ILLEGAL_TXT]);
          render_battle_message(m);
        } else {
          // We are in a battle with a wild encounter
          sprintf(m, "%s used %s", user->get_nickname(), item_name_txt[item]);
          render_battle_message(m);
          // TODO: catch mechanics. (currently always success)
          // sprintf(m, "Oh, no! The POKEMON broke free!"); 
          sprintf(m, "Gotcha! %s was caught!", opp_poke->get_nickname());
          render_battle_message(m);
          if (user->get_party_size() < 6) {
            user->add_pokemon(opp_poke);
          } else {
            // TODO: pokemon PC box mechanics. sent to box
            sprintf(m, "%s will be sent to BOX %d.", 
                    opp_poke->get_nickname(), 1);
            render_battle_message(m);
          }
        }
        return 0;
      } else {
        // we are not in a battle
        
      }
      return 1;
    case item_potion:
      poke_index = party_view_driver(3 /*Select pokemon for item*/);
      if (poke_index == -1) {
        // user cancel
        return 1;
      }
      if (user->get_pokemon(poke_index)->get_current_hp()
       == user->get_pokemon(poke_index)->get_stat(stat_hp)) {
        // pokemon is at full health
        render_party_message("It won't have any effect.");
        return 1;
      }
      if (user->get_pokemon(poke_index)->is_fainted()) {
        // pokemon is fainted
        render_party_message("It won't have any effect.");
        return 1;
      }
      user->remove_item_from_bag(item);
      user->get_pokemon(poke_index)->heal(20);
      sprintf(m, "%s's HP was restored by 20 points.", 
              user->get_pokemon(poke_index)->get_nickname());
      render_party(poke_index, -1, -1, m, 0, 0);
      usleep(FRAMETIME);
      flushinp();
      getch();
      return 0;
    case item_revive:
      poke_index = party_view_driver(3 /*Select pokemon for item*/);
      if (poke_index == -1) {
        // user cancel
        return 1;
      }
      if (!user->get_pokemon(poke_index)->is_fainted()) {
        // pokemon is not fainted
        render_party_message("It won't have any effect.");
        return 1;
      }
      user->remove_item_from_bag(item);
      user->get_pokemon(poke_index)->heal(
        user->get_pokemon(poke_index)->get_stat(stat_hp) / 2);
      sprintf(m, "%s's HP was restored by %d points.", 
              user->get_pokemon(poke_index)->get_nickname(),
              user->get_pokemon(poke_index)->get_stat(stat_hp) / 2);
      render_party(poke_index, -1, -1, m, 0, 0);
      usleep(FRAMETIME);
      flushinp();
      getch();
      return 0;
    default:
     return 1;
  }

  // we will never reach here
  return 1;
}

/*
 * Initiates and drives a battle
 */
void battle_trainer_driver(Character *opp, Pc *pc) {
  // battle_t battle;
  // battle.opp = opp;
  // battle.pc = pc;
  // battle.end_battle = 0;
  // pc->set_state(s_battle);

  // while (!battle.end_battle) {
  //   render_battle(&battle);
  //   process_input_battle(&battle);
  // }

  // opp->set_defeated(true);
  return;
}

/*
 * Checks if a player is initiating a battle
 * Returns 1 if a trainer battle occured
 */
bool check_trainer_battle(int32_t to_i, int32_t to_j) {
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];
  
  if (pc->is_defeated()) {
    return false;
  }

  for (auto it = r->get_npcs()->begin(); it != r->get_npcs()->end(); ++it) {
    if (it->get_i() == to_i
     && it->get_j() == to_j
     && !(it->is_defeated())) {
      battle_trainer_driver(&(*it), pc);
      return true;
    }
  }
  return false;
}

/*
 * Process a single fighting turn.
 * Returns true if the defending pokemon fainted
 */
bool do_fight_turn(Pokemon *attacker, int32_t attacking_move_slot, 
                   Pokemon *defender, bool pc_is_attacker) {
  int32_t damage;
  float type;
  bool critical;
  char m[MAX_COL];
  Pokemon *pc_poke, *opp_poke;

  if (pc_is_attacker) {
    pc_poke = attacker;
    opp_poke = defender;
    sprintf(m, "%s used %s!", attacker->get_nickname(), 
            attacker->get_move(attacking_move_slot)->identifier);
  } else {
    pc_poke = defender;
    opp_poke = attacker;
    if (!defender->get_has_owner()) {
      sprintf(m, "Wild %s used %s!", attacker->get_nickname(), 
              attacker->get_move(attacking_move_slot)->identifier);
    } else {
      sprintf(m, "Foe %s used %s!", attacker->get_nickname(), 
              attacker->get_move(attacking_move_slot)->identifier);
    }
  }
  if (!attacker->has_pp()) {
    sprintf(m, "%s has no moves left!", attacker->get_nickname());
  }
  render_battle_message(m);

  attacker->use_pp(attacking_move_slot);

  if (is_miss(attacker->get_move(attacking_move_slot))) {
    sprintf(m, "%s's attack missed!", attacker->get_nickname());
    render_battle(pc_poke, opp_poke, m, false, 0, 0);
  } else {
    critical = is_critical(attacker, attacker->get_move(attacking_move_slot));
    damage = calculate_damage(attacker, attacker->get_move(attacking_move_slot),
                              defender, critical);
    defender->take_damage(damage);
    if (critical) {
      sprintf(m, "A critical hit!");
      render_battle(pc_poke, opp_poke, m, false, 0, 0);
      usleep(FRAMETIME);
      flushinp();
      getch();
    }
    type = effectiveness(attacker->get_move(attacking_move_slot), defender);
    if (type > 1) {
      sprintf(m, "It's super effective!");
      render_battle(pc_poke, opp_poke, m, false, 0, 0);
      usleep(FRAMETIME);
      flushinp();
      getch();
    } else if (type == 0) {
      sprintf(m, "But it had no effect!");
      render_battle(pc_poke, opp_poke, m, false, 0, 0);
      usleep(FRAMETIME);
      flushinp();
      getch();
    } else if (type < 1) {
      sprintf(m, "It's not very effective...");
      render_battle(pc_poke, opp_poke, m, false, 0, 0);
      usleep(FRAMETIME);
      flushinp();
      getch();
    }
    if (defender->is_fainted()) {
      if (pc_is_attacker) {
        if (!defender->get_has_owner()) {
          sprintf(m, "Wild %s fainted!", defender->get_nickname());
        } else {
          sprintf(m, "Foe %s fainted!", defender->get_nickname());
        }
      } else {
        sprintf(m, "%s fainted!", defender->get_nickname());
        pc->set_defeated(pc->get_active_pokemon() == NULL);
      }
      render_battle(pc_poke, opp_poke, m, false, 0, 0);
      usleep(FRAMETIME);
      flushinp();
      getch();
      return true;
    }
  }
  return false;
}

/*
 * Process a battle turn for attempting to run from a wild pokemon
 * Returns True only if escape is successful.
 */
bool attempt_escape (Pokemon *pc_active, Pokemon *wp, int32_t *attempts) {
  int32_t escape_odds = (pc_active->get_stat(stat_speed) * 32)
                        / ((wp->get_stat(stat_speed) / 4) % 256) 
                        + 30 * (*attempts);
  if (rand() % 256 < escape_odds) {
    // Escape was successful
    render_battle_message("Got away safely!");
    return true;
  }
  // Escape failed
  render_battle_message("Can't escape!");
  return false;
}

/*
 * Initiates and drives an encounter
 */
void battle_encounter_driver(Pc *pc) {
  Pokemon *wp = new Pokemon();
  bool end_encounter = false;
  int32_t scroller_pos = 0;
  bool selected_fight = false;
  int32_t pc_move_slot, ai_move_slot;
  int32_t priority;
  bool pc_turn;
  char m[MAX_COL];
  Pokemon *pc_active = pc->get_active_pokemon();
  item_t selected_item;
  int32_t esc_atempts = 0;

  sprintf(m, "Wild %s appeared! Go! %s!", wp->get_nickname(), 
                                          pc_active->get_nickname());
  render_battle(pc_active, wp, m, false, scroller_pos, selected_fight);
  usleep(500000);
  flushinp();
  getch();

  while (!end_encounter && !wp->get_has_owner() 
        && !wp->is_fainted() && !pc->is_defeated()) {
    sprintf(m, "What will %s do?", pc_active->get_nickname());
    render_battle(pc_active, wp, m, true, scroller_pos, selected_fight);

    if (!pc_turn) {
      ai_move_slot = wp->ai_select_move_slot();
      pc_turn = true;
    }

    while (pc_turn) {
      process_input_battle(pc_active, &scroller_pos, &selected_fight, &pc_turn);
      render_battle(pc_active, wp, m, true, scroller_pos, selected_fight);
    }

   /* 
    * Pokemon battle menu scroller layout
    * 0 Fight
    * 1 Bag
    * 2 Pokemon  
    * 3 Run
    */
    // FIGHT
    if (selected_fight) {
      if ((pc_active->has_pp() 
           && pc_active->get_current_pp(scroller_pos) > 0) 
          || !pc_active->has_pp()) {
        // selected move with pp or is struggling
        pc_move_slot = pc_active->has_pp() ? scroller_pos : -1;

        priority = move_priority(
                    pc_active->get_move(pc_move_slot)->priority, 
                    pc_active->get_stat(stat_speed),
                    wp->get_move(ai_move_slot)->priority, 
                    wp->get_stat(stat_speed));
          
        if (priority > 0) {
          end_encounter = do_fight_turn(pc_active, pc_move_slot, wp, true);
          if (!end_encounter) {
            end_encounter = do_fight_turn(wp, ai_move_slot, pc_active, false);
          }
        } else {
          end_encounter = do_fight_turn(wp, ai_move_slot, pc_active, false);
          if (!end_encounter) {
            end_encounter = do_fight_turn(pc_active, pc_move_slot, wp, true);
          }
        }
        pc_turn = false;
      } else {
        // selected move with no pp but is not struggling
        render_battle_message("There's no PP left for this move!");
        pc_turn = true;
      }
        if (!pc_active->has_pp()) {
          selected_fight = false;
        }
    // BAG
    } else if (scroller_pos == 1) {
      selected_item = bag_driver();
      render_battle(pc_active, wp, m, false, 0, 0);
      pc_turn = use_item(pc, pc_active, wp, selected_item);
      render_battle(pc_active, wp, m, false, 0, 0);
      if (!pc_turn && !wp->get_has_owner()) {
        end_encounter = do_fight_turn(wp, ai_move_slot, pc_active, false);
      }
    // POKEMON
    } else if (scroller_pos == 2) {
      pc_turn = party_view_driver(1);

      // TODO: POKEMON SWITCHING
      pc_active = pc->get_active_pokemon();
      render_battle(pc_active, wp, m, false, 0, 0);

    if (!pc_turn && !wp->get_has_owner()) {
      end_encounter = do_fight_turn(wp, ai_move_slot, pc_active, false);
    }
    // RUN
    } else if (scroller_pos == 3) {
      end_encounter = pc_turn = attempt_escape(pc_active, wp, &esc_atempts);
      if (!pc_turn && !wp->get_has_owner()) {
        end_encounter = do_fight_turn(wp, ai_move_slot, pc_active, false);
      }
    }
    
  }

  if (wp->is_fainted()) {

    delete wp;
  }
  if (pc->is_defeated()) {
    sprintf(m, "%s whited out!", pc->get_nickname());
    render_battle_message(m);
  }
  
  return;
}

/*
 * Checks if a wild encounter should occur
 * Returns 1 if a wild encounter occured
 */
bool check_wild_encounter() {
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];

  if (pc->is_defeated())
    return false;

  // 10% chance of wild encounter if player moved to tall grass
  int32_t randy = rand() % POKEMON_ENCOUNTER_RATE;

  terrain_t standing_on = r->get_ter(pc->get_i(), pc->get_j());
  if ((standing_on == ter_grass) && (randy == 0)) {
    battle_encounter_driver(pc);
    return true;
  }
  return false;
}

/*
 * Performs the all the nessesary checks to ensure a location is a valid spot to
 * move to.
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
 * Performs the all the nessesary checks to ensure a location is a valid spot to
 * move to except if the player is in that location.
 * 
 * Returns true if the location is valid or player exists there, false 
 * otherwise.
 */
bool is_valid_gradient(int32_t to_i, int32_t to_j, 
                       int32_t dist_map[MAX_ROW][MAX_COL]) {
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
    battle_trainer_driver(c, pc);
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
  if (check_trainer_battle(pc->pos_i + dir_offsets[dir][0], 
                   pc->pos_j + dir_offsets[dir][1])) {
    return 0;
  }

  if (is_valid_location(pc->pos_i + dir_offsets[dir][0], 
                        pc->pos_j + dir_offsets[dir][1], tnr_pc)) {
    pc->pos_i += dir_offsets[dir][0];
    pc->pos_j += dir_offsets[dir][1];

    if (check_wild_encounter()) {
      return 0;
    }

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

/*
 * Drives player party interactions
 *
 * Implements pokemon menu mechanics for the following scenarios
 * Select, Switch, and view pokemon summary
 * 
 * scenario:
 * 0 = wandering          (allows unlimited switches)  Returns 0
 *   o1 = Shift, 
 *   o2 = Summary
 * 1 = battle             (allows 1 switch)           *Returns pokemon index
 *   o1 = Shift, 
 *   o2 = Summary
 * 2 = battle select next (select 1 pokemon)           Returns pokemon index
 *   o1 = Send Out, 
 *   o2 = Summary
 * 3 = use item           (select 1 pokemon)          *Returns pokemon index
 * 
 * *Returns -1 if pokemon was not selected
 */
int32_t party_view_driver(int32_t scenario) {
  int32_t close_party = 0;
  int32_t selected_p1 = 0;
  int32_t selected_p2 = -1;
  int32_t selected_opt = -1;
  char m[MAX_COL] = "Choose a POKEMON.";
  char o1[MAX_COL - 5] = {'\0'};
  char o2[MAX_COL - 5] = "SUMMARY";

  switch (scenario) {
    case 0:
    case 1:
      strncpy(o1, "SHIFT", MAX_COL - 5);
      strncpy(m, "Choose a POKEMON.", MAX_COL);
      break;
    case 2:
      strncpy(o1, "SEND OUT", MAX_COL - 5);
      strncpy(m, "Choose a POKEMON.", MAX_COL);
      break;
    case 3:
      strncpy(o1, "THIS TEXT WILL NEVER APPEAR", MAX_COL - 5);
      strncpy(m, "Use on which POKEMON?", MAX_COL);
      break;
    default:
      return -1;
  }

  while (!close_party) {

    render_party(selected_p1, selected_p2, selected_opt, m, o1, o2);
    process_input_party(scenario, &selected_p1, &selected_p2, 
                        &selected_opt, &close_party);
  }

  return selected_p1;
}