#include <climits>
#include <cstdint>
#include <cstdlib>
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

  while (!close_bag && !(pc->is_quit_game()) && item_selected == -1) {
    render_bag(page_index, scroller_pos);
    process_input_bag(&page_index, &scroller_pos, &close_bag, &item_selected);
  }
  if (item_selected != -1) {
    return pc->peek_bag_slot(item_selected).item;
  }
  return item_empty;
}

/*
 * Returns true if item was successfully used, otherwise returns false.
 */
bool use_item(Character *user, Pokemon *user_poke, Pokemon *opp_poke,
              item_t item) {
  char m[MAX_COL];

  switch (item) {
    case item_pokeball:
      if (opp_poke != NULL) {
        user->remove_item_from_bag(item);
        if (!opp_poke->get_has_owner()) {
          // We are in a battle with a wild encounter
          sprintf(m, "%s used %s", user->get_nickname(), item_name_txt[item]);
          render_battle_message(m);
          usleep(FRAMETIME);
          flushinp();
          getch();
          user->add_pokemon(opp_poke);
          sprintf(m, "Gotcha! %s was caught!", opp_poke->get_nickname());
          // TODO: catch mechanics. (currently always success)
          // sprintf(m, "Oh, no! The POKEMON broke free!"); 
        } else {
          // We are in a battle with another trainer
          sprintf(m, "%s", catch_illegal_txt[rand() % NUM_CATCH_ILLEGAL_TXT]);
        }
        
        render_battle_message(m);
        usleep(FRAMETIME);
        flushinp();
        getch();
      }
      break;
    case item_potion:
      break;
    case item_revive:
      break;
    default:
     return false;
  }

  return false;
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

  // while (!battle.end_battle && !(pc->is_quit_game())) {
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
bool do_fight_turn(Pokemon *attacker, pd_move_t *attacking_move, 
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
                            attacking_move->identifier);
  } else {
    pc_poke = defender;
    opp_poke = attacker;
    if (!defender->get_has_owner()) {
      sprintf(m, "Wild %s used %s!", attacker->get_nickname(), 
                                     attacking_move->identifier);
    } else {
      sprintf(m, "Foe %s used %s!", attacker->get_nickname(), 
                                    attacking_move->identifier);
    }
  }
  render_battle(pc_poke, opp_poke, m, false, 0, 0);
  usleep(FRAMETIME);
  flushinp();
  getch();

  if (is_miss(attacking_move)) {
    sprintf(m, "%s's attack missed!", attacker->get_nickname());
    render_battle(pc_poke, opp_poke, m, false, 0, 0);
    usleep(FRAMETIME);
    flushinp();
    getch();
  } else {
    critical = is_critical(attacker);
    damage = calculate_damage(attacker, attacking_move, defender, critical);
    defender->take_damage(damage);
    if (critical) {
      sprintf(m, "A critical hit!");
      render_battle(pc_poke, opp_poke, m, false, 0, 0);
      usleep(FRAMETIME);
      flushinp();
      getch();
    }
    type = effectiveness(attacking_move, defender);
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
    if (defender->get_current_hp() == 0) {
      if (pc_is_attacker) {
        if (!defender->get_has_owner()) {
          sprintf(m, "Wild %s fainted!", defender->get_nickname());
        } else {
          sprintf(m, "Foe %s fainted!", defender->get_nickname());
        }
      } else {
        sprintf(m, "%s fainted!", defender->get_nickname());
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
bool attempt_escape (Pokemon *pc_active_p, Pokemon *wp, int32_t *attempts) {
  int32_t escape_odds = (pc_active_p->get_stat(stat_speed) * 32)
                        / ((wp->get_stat(stat_speed) / 4) % 256) 
                        + 30 * (*attempts);
  if (rand() % 256 < escape_odds) {
    // Escape was successful
    render_battle_message("Got away safely!");
    usleep(FRAMETIME);
    flushinp();
    getch();
    return true;
  }
  // Escape failed
  render_battle_message("Can't escape!");
  usleep(FRAMETIME);
  flushinp();
  getch();
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
  pd_move_t *ai_move;
  int32_t priority;
  bool pc_turn;
  char m[MAX_COL];
  Pokemon *pc_active_p = pc->get_pokemon(0);
  item_t selected_item;
  int32_t esc_atempts = 0;

  sprintf(m, "Wild %s appeared! Go! %s!", wp->get_nickname(), 
                                          pc_active_p->get_nickname());
  render_battle(pc_active_p, wp, m, true, scroller_pos, selected_fight);
  usleep(500000);
  flushinp();
  getch();

  while (!end_encounter && !(pc->is_quit_game()) 
      && !wp->get_has_owner() && wp->get_current_hp() > 0) {
    sprintf(m, "What will %s do?", pc_active_p->get_nickname());
    render_battle(pc_active_p, wp, m, true, scroller_pos, selected_fight);

    if (!pc_turn) {
      ai_move = wp->get_rand_move();
      pc_turn = true;
    }

    while (pc_turn && !(pc->is_quit_game())) {
      process_input_battle(pc_active_p, &scroller_pos, &selected_fight, 
                           &pc_turn);
      render_battle(pc_active_p, wp, m, true, scroller_pos, selected_fight);
    }

   /* 
    * Pokemon battle menu scroller layout
    * 0 Fight
    * 1 Bag
    * 2 Pokemon  
    * 3 Run
    */
    if (!(pc->is_quit_game())) {
      // FIGHT
      if (selected_fight) {
          priority = move_priority(
                      pc_active_p->get_move(scroller_pos)->priority, 
                      pc_active_p->get_stat(stat_speed),
                      ai_move->priority, 
                      wp->get_stat(stat_speed));
          
        if (priority > 0) {
          end_encounter = do_fight_turn(pc_active_p, 
                            pc_active_p->get_move(scroller_pos), wp, true);
          pc_active_p->use_pp(scroller_pos);
          if (!end_encounter) {
            end_encounter = do_fight_turn(wp, ai_move, pc_active_p, false);
            wp->use_rand_move_pp();
          }
        } else {
          end_encounter = do_fight_turn(wp, ai_move, pc_active_p, false);
          wp->use_rand_move_pp();
          if (!end_encounter) {
            end_encounter = do_fight_turn(pc_active_p, 
                              pc_active_p->get_move(scroller_pos), wp, true);
            pc_active_p->use_pp(scroller_pos);
          }
        }
        pc_turn = false;
      // BAG
      } else if (scroller_pos == 1) {
        selected_item = bag_driver();
        render_battle(pc_active_p, wp, m, false, 0, 0);
        pc_turn = use_item(pc, pc_active_p, wp, selected_item);
        if (!pc_turn && !wp->get_has_owner()) {
          end_encounter = do_fight_turn(wp, ai_move, pc_active_p, false);
          wp->use_rand_move_pp();
        }
      // POKEMON
      } else if (scroller_pos == 2) {
        // TODO: POKEMON SWITCHING
      if (!pc_turn && !wp->get_has_owner()) {
        end_encounter = do_fight_turn(wp, ai_move, pc_active_p, false);
        wp->use_rand_move_pp();
      }
      // RUN
      } else if (scroller_pos == 3) {
        end_encounter = pc_turn = attempt_escape(pc_active_p, wp, &esc_atempts);
        if (!pc_turn && !wp->get_has_owner()) {
          end_encounter = do_fight_turn(wp, ai_move, pc_active_p, false);
          wp->use_rand_move_pp();
        }
      }
    }
    
  }

  if (wp->get_current_hp() == 0 || pc->is_quit_game()) {
    delete wp;
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
 * Switch pokemon or view pokemon summary
 */
void party_view_driver() {
  int32_t close_party = 0;
  int32_t scroller_pos = 0;
  int32_t option_pos = 0;
  int32_t pokemon_selected = -1;
  int32_t option_selected = -1;

  while (!close_party && !(pc->is_quit_game()) && option_selected == -1) {
    //render_party(page_index, scroller_pos);
    //process_input_party_view(&page_index, &scroller_pos, &close_party, &item_selected);
  }

  return;
}