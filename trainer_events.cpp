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

int min(int a, int b) {
  return a <= b ? a : b;
}
int max(int a, int b) {
  return a >= b ? a : b;
}

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
  int32_t old_hp;

  switch (item) {
    case item_pokeball:
      if (opp_poke != NULL) {
        // we are in a battle
        user->remove_item_from_bag(item);
        if (opp_poke->get_has_owner()) {
          // We are in a battle with another trainer
          sprintf(m, "%s", catch_illegal_txt[rand() % NUM_CATCH_ILLEGAL_TXT]);
          render_battle_message_getch(m);
        } else {
          // We are in a battle with a wild encounter
          sprintf(m, "%s used %s", user->get_nickname(), item_name_txt[item]);
          render_battle_message_getch(m);
          // TODO: catch mechanics. (currently always success)
          // sprintf(m, "Oh, no! The POKEMON broke free!"); 
          sprintf(m, "Gotcha! %s was caught!", opp_poke->get_nickname());
          render_battle_message_getch(m);
          if (user->get_party_size() < 6) {
            user->add_pokemon(opp_poke);
          } else {
            // TODO: pokemon PC box mechanics. sent to box
            sprintf(m, "%s will be sent to BOX %d.", 
                    opp_poke->get_nickname(), 1);
            render_battle_message_getch(m);
          }
        }
        return 0;
      } else {
        // we are not in a battle
        render_bag_message("Can't use that here.");
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
      old_hp = user->get_pokemon(poke_index)->get_current_hp();
      user->get_pokemon(poke_index)->heal(20);
      sprintf(m, "%s's HP was restored by %d points.", 
              user->get_pokemon(poke_index)->get_nickname(), 
              user->get_pokemon(poke_index)->get_current_hp() - old_hp);
      render_party_getch(poke_index, -1, -1, m, 0, 0);
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
      render_party_getch(poke_index, -1, -1, m, 0, 0);
      user->set_defeated(false);
      return 0;
    default:
     return 1;
  }

  // we will never reach here
  return 1;
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
      battle_driver(pc, &(*it));
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
  } else {
    pc_poke = defender;
    opp_poke = attacker;
  }

  if (!attacker->has_pp()) {
    sprintf(m, "%s has no moves left!", attacker->get_nickname());
    render_battle_message_getch(m);
  }

  if (pc_is_attacker) {
    sprintf(m, "%s used %s!", attacker->get_nickname(), 
            attacker->get_move(attacking_move_slot)->identifier);
  } else {
    if (!attacker->get_has_owner()) {
      sprintf(m, "Wild %s used %s!", attacker->get_nickname(), 
              attacker->get_move(attacking_move_slot)->identifier);
    } else {
      sprintf(m, "Foe %s used %s!", attacker->get_nickname(), 
              attacker->get_move(attacking_move_slot)->identifier);
    }
  }

  attacker->use_pp(attacking_move_slot);

  if (is_miss(attacker->get_move(attacking_move_slot))) {
    // MISS
    render_battle_message_getch(m);
    sprintf(m, "%s's attack missed!", attacker->get_nickname());
    render_battle(pc_poke, opp_poke, m, false, 0, 0);
  } else if (attacker->get_move(attacking_move_slot)->damage_class_id == 1) {
    // STATUS MOVES ARE NOT IMPLEMENTED
    render_battle_message_getch(m);
    sprintf(m, "Status effects are not implemented yet. It had no effect!");
    render_battle_getch(pc_poke, opp_poke, m, false, 0, 0);
  } else {
    // HIT
    critical = is_critical(attacker, attacker->get_move(attacking_move_slot));
    damage = calculate_damage(attacker, attacker->get_move(attacking_move_slot),
                              defender, critical);
    defender->take_damage(damage);
    render_battle_message(m);
    usleep(BATTLE_ANIMATION_TIME);
    render_battle_getch(pc_poke, opp_poke, m, false, 0, 0);
    type = effectiveness(attacker->get_move(attacking_move_slot), defender);
    if (critical && type != 0) {
      sprintf(m, "A critical hit!");
      render_battle_getch(pc_poke, opp_poke, m, false, 0, 0);
    }
    if (type > 1) {
      sprintf(m, "It's super effective!");
      render_battle_getch(pc_poke, opp_poke, m, false, 0, 0);
    } else if (type == 0) {
      sprintf(m, "But it had no effect!");
      render_battle_getch(pc_poke, opp_poke, m, false, 0, 0);
    } else if (type < 1) {
      sprintf(m, "It's not very effective...");
      render_battle_getch(pc_poke, opp_poke, m, false, 0, 0);
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
      render_battle_getch(pc_poke, opp_poke, m, false, 0, 0);
      return true;
    }
  }
  return false;
}

/*
 * Process a battle turn for attempting to run from a pokemon
 * Returns True only if escape is successful.
 */
bool attempt_escape (Pokemon *pc_active, Pokemon *opp, int32_t *attempts) {
  if (opp->get_has_owner()) {
    // escape failed
    render_battle_message_getch(
      "No! There's no running from a TRAINER battle!");
    return false;
  }

  int32_t escape_odds = (pc_active->get_stat(stat_speed) * 32)
                        / ((opp->get_stat(stat_speed) / 4) % 256) 
                        + 30 * (*attempts);
  if (rand() % 256 < escape_odds) {
    // Escape was successful
    render_battle_message_getch("Got away safely!");
    return true;
  }
  // Escape failed
  render_battle_message_getch("Can't escape!");
  return false;
}

/*
 * Initiates and drives a battle
 * if c is NULL, then battle will be a wild encounter
 */
void battle_driver(Pc *pc, Character *opp) {
  bool end_battle = false;
  int32_t pokemon_fainted = 0;
  int32_t scroller_pos = 0;
  bool selected_fight = false;
  int32_t pc_move_slot, ai_move_slot;
  int32_t priority;
  bool pc_turn;
  char m[MAX_COL];
  Pokemon *pc_active = pc->get_active_pokemon();
  Pokemon *opp_active;
  item_t selected_item;
  int32_t esc_atempts = 0;

  if (opp == NULL) {
    // wild encounter
    opp_active = new Pokemon();
    sprintf(m, "Wild %s appeared! Go! %s!", opp_active->get_nickname(), 
                                          pc_active->get_nickname());
    render_battle_getch(pc_active, opp_active, 
                        m, false, scroller_pos, selected_fight);
  } else {
    // trainer battle
    opp_active = opp->get_active_pokemon();
    sprintf(m, "%s would like to battle! %s sent out %s! Go! %s!", 
            opp->get_nickname(), opp->get_nickname(), 
            opp_active->get_nickname(), pc_active->get_nickname());
    render_battle_getch(pc_active, opp_active, 
                        m, false, scroller_pos, selected_fight);
  }

  while (!end_battle) {
    sprintf(m, "What will %s do?", pc_active->get_nickname());
    render_battle(pc_active, opp_active, m, true, scroller_pos, selected_fight);

    if (!pc_turn) {
      ai_move_slot = opp_active->ai_select_move_slot();
      pc_turn = true;
    }

    while (pc_turn) {
      process_input_battle(pc_active, &scroller_pos, &selected_fight, &pc_turn);
      render_battle(pc_active, opp_active, 
                    m, true, scroller_pos, selected_fight);
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
                    opp_active->get_move(ai_move_slot)->priority, 
                    opp_active->get_stat(stat_speed));
          
        if (priority > 0) {
          pokemon_fainted = do_fight_turn(pc_active, pc_move_slot, 
                                          opp_active, true);
          if (!pokemon_fainted) {
            pokemon_fainted = do_fight_turn(opp_active, ai_move_slot, 
                                            pc_active, false);
          }
        } else {
          pokemon_fainted = do_fight_turn(opp_active, ai_move_slot, 
                                          pc_active, false);
          if (!pokemon_fainted) {
            pokemon_fainted = do_fight_turn(pc_active, pc_move_slot, 
                                            opp_active, true);
          }
        }
        pc_turn = false;
      } else {
        // selected move with no pp but is not struggling
        render_battle_message_getch("There's no PP left for this move!");
        pc_turn = true;
      }
        if (!pc_active->has_pp()) {
          selected_fight = false;
        }
    // BAG
    } else if (scroller_pos == 1) {
      selected_item = bag_driver();
      render_battle(pc_active, opp_active, m, false, 0, 0);
      pc_turn = use_item(pc, pc_active, opp_active, selected_item);
      render_battle(pc_active, opp_active, m, false, 0, 0);
      if (opp_active->get_has_owner() && opp == NULL) {
        // wild pokemon was caught... end battle
        end_battle = 1;
      } else if (!pc_turn) {
        pokemon_fainted = do_fight_turn(opp_active, ai_move_slot, 
                                        pc_active, false);
      }
    // POKEMON
    } else if (scroller_pos == 2) {
      pc_turn = party_view_driver(1);

      // check for swapped pokemon
      Pokemon *new_active = pc->get_active_pokemon();
      if (pc_active != new_active) {
        sprintf(m, "%s, that's enough! Come back!", pc_active->get_nickname());
        render_battle_getch(pc_active, opp_active, m, false, 0, 0);
        pc_active = new_active;
        sprintf(m, "Go! %s!", pc_active->get_nickname());
        render_battle_getch(pc_active, opp_active, m, false, 0, 0);
        pc_turn = false;
      }

      pc_active = pc->get_active_pokemon();
      render_battle(pc_active, opp_active, m, false, 0, 0);

    if (!pc_turn && !opp_active->get_has_owner()) {
      pokemon_fainted = do_fight_turn(opp_active, ai_move_slot, 
                                      pc_active, false);
    }
    // RUN
    } else if (scroller_pos == 3) {
      end_battle = pc_turn = attempt_escape(pc_active, opp_active, 
                                            &esc_atempts);
      if (!pc_turn) {
        pokemon_fainted = do_fight_turn(opp_active, ai_move_slot, 
                                        pc_active, false);
      }
    }

    if (pokemon_fainted) {
      // give exp and level up
      if (opp_active->is_fainted()) {
        int32_t exp_gain = experience_gain(opp_active);
        int32_t exp_for_this_level;
        sprintf(m, "%s gained %d EXP. Points!", pc_active->get_nickname()
                                              , exp_gain);
        render_battle_message_getch(m);
        while (exp_gain > 0) {
          exp_for_this_level = min(pc_active->get_exp_next_level(), exp_gain);
          exp_gain -= exp_for_this_level;
          pc_active->give_exp(exp_for_this_level);
          usleep(BATTLE_ANIMATION_TIME);
          render_battle(pc_active, opp_active, m, false, 0, 0);
          if (pc_active->process_level_up()) {
            usleep(BATTLE_ANIMATION_TIME);
            // pokemon leveled up, 
            // screen must be redrawn completely incase a new move was learned
            sprintf(m, "%s grew to LV. %d!", pc_active->get_nickname()
                                           , pc_active->get_level());
            render_battle_getch(pc_active, opp_active, m, false, 0, 0);
          }
        }
      }

      if (pc->is_defeated()) {
        // player defeated... end battle
        sprintf(m, "%s is out of usable POKEMON!", pc->get_nickname());
        render_battle_message_getch(m);
        int32_t payout = pc->get_payout();
        pc->take_poke_dollars(payout);
        sprintf(m, "%s panicked and lost $%d!", pc->get_nickname(), payout);
        render_battle_message_getch(m);
        sprintf(m, "%s whited out!", pc->get_nickname());
        render_battle_message_getch(m);
        end_battle = 1;
      } else if (pc_active->is_fainted()) {
        // player pokemon fainted... use next pokemon
        party_view_driver(2);
        pc_active = pc->get_pokemon(0);
        sprintf(m, "Go! %s!", pc_active->get_nickname());
        render_battle_getch(pc_active, opp_active, m, false, 0, 0);
      } else if (opp == NULL) {
        // WILD POKEMON ENCOUNTER
        if (opp_active->is_fainted()) {
          // wild pokemon fainted... clean up, end battle
          delete opp_active;
          end_battle = 1;
        }
      } else {
        // TRAINER BATTLE
        if (opp->get_active_pokemon() == NULL) {
          // opponent defeated
          opp->set_defeated(true);
          sprintf(m, "%s defeated %s", pc->get_nickname(), opp->get_nickname());
          render_battle_getch(pc_active, opp_active, m, false, 0, 0);
          int32_t payout = opp->get_payout();
          pc->give_poke_dollars(payout);
          sprintf(m, "%s got $%d for winning!", pc->get_nickname(), payout);
          render_battle_message_getch(m);
          end_battle = 1;
        } else {
          // opponent pokemon fainted... use next pokemon
          opp_active = opp->get_active_pokemon();
          sprintf(m, "%s sent out %s!", 
                  opp->get_nickname(), opp_active->get_nickname());
          render_battle_getch(pc_active, opp_active, 
                              m, false, scroller_pos, selected_fight);
        }
      }
    }
    
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
    battle_driver(pc, NULL);
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
    battle_driver(pc, c);
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
 * 0 = wandering          (allows unlimited switches)
 *   o1 = Shift, 
 *   o2 = Summary
 * 1 = battle             (allows 1 switch)
 *   o1 = Shift, 
 *   o2 = Summary
 * 2 = battle select next (select 1 pokemon)
 *   o1 = Send Out, 
 *   o2 = Summary
 * 3 = use item           (select 1 pokemon)
 * 
 * Returns pokemon selected pokemon index, -1 if pokemon was not selected
 */
int32_t party_view_driver(int32_t scenario) {
  int32_t close_party = 0;
  int32_t selected_p1 = 0;
  int32_t selected_p2 = -1;
  int32_t selected_opt = -1;
  char m1[MAX_COL] = {'\0'};
  char m2[MAX_COL] = "Do what with this POKEMON?";
  char m3[MAX_COL] = {'\0'};
  char o1[MAX_COL - 5] = {'\0'};
  char o2[MAX_COL - 5] = "SUMMARY";

  switch (scenario) {
    case 0:
    case 1:
      strncpy(o1, "SHIFT", MAX_COL - 5);
      strncpy(m1, "Choose a POKEMON.", MAX_COL);
      break;
    case 2:
      strncpy(o1, "SEND OUT", MAX_COL - 5);
      strncpy(m1, "Choose next POKEMON.", MAX_COL);
      break;
    case 3:
      // strncpy(o1, "THIS TEXT WILL NEVER APPEAR", MAX_COL - 5);
      strncpy(m1, "Use on which POKEMON?", MAX_COL);
      break;
    default:
      return -1;
  }

  if (pc->get_party_size() < 2) {
    strncpy(m3, "That's your last POKEMON!", MAX_COL);
  } else {
    strncpy(m3, "Move to where?", MAX_COL);
  }

  while (!close_party) {
    if (selected_opt == -1) {
      render_party(selected_p1, selected_p2, selected_opt, m1, o1, o2);
    } else if (selected_p2 == -1) {
      render_party(selected_p1, selected_p2, selected_opt, m2, o1, o2);
    } else {
      render_party(selected_p1, selected_p2, selected_opt, m3, o1, o2);
    }
    
    process_input_party(scenario, &selected_p1, &selected_p2, 
                        &selected_opt, &close_party);
  }

  return selected_p1;
}