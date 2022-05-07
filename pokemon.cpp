#include <cstdlib>
#include <cstring>
#include <vector>

#include "pokemon.h"
#include "character.h"
#include "region.h"
#include "global_events.h"

extern Pc *pc;

/*
 * Helper method to look up and initialize a pokemon's type
 */
void Pokemon::lookup_type() {
  // initialize types to 0
  type[0] = -1; // primary type
  type[1] = -1; // secondary type
  
  // Lookup types
  for (int32_t i = 0; i < POKEDEX_POKEMON_TYPES_ENTRIES; ++i) {
    if (pd_pokemon_types[i].pokemon_id == pd_entry->id) {
      // we assume that the pokemon_types.csv lists type slots in order for one 
      // pokemon at a time.
      type[0] = pd_pokemon_types[i].type_id;
      if (pd_pokemon_types[i+1].pokemon_id == pd_entry->id) {
        type[1] = pd_pokemon_types[i+1].type_id;
      }
      break;
    }
  }
  return;
}
/*
 * Helper method to generate a pokemon's level
 * Called when initializing a pokemon
 */
void Pokemon::generate_level() {
  int32_t dist = m_dist(WORLD_SIZE/2, WORLD_SIZE/2, pc->get_x(), pc->get_y());
  int32_t min_level;
  int32_t max_level;
  if (dist <= 200) {
    min_level = POKEMON_MIN_LEVEL;
    max_level = dist / 2;
    if (max_level > POKEMON_MAX_LEVEL)
      max_level = POKEMON_MAX_LEVEL;
    if (max_level < 1)
      max_level = 1;
  } else {
    min_level = ((dist - 200) / 2);
    max_level = POKEMON_MAX_LEVEL;
    if (min_level > POKEMON_MAX_LEVEL)
      min_level = POKEMON_MAX_LEVEL;
    if (min_level < 1)
      min_level = 1;
  }
  level = min_level + (rand() % (max_level - min_level + 1));
  return;
}
/*
 * Helper method to populate a pokemon's moveset
 * Called when initializing a pokemon
 */
void Pokemon::populate_moveset() {
  // 1. Init to 0
  for (int32_t i = 0; i < 4; ++i) {
    moveset[i] = 0;
    current_pp[i] = 0;
  }
  num_moves = 0;

  // 2. Find levelup learnset
  std::vector<int32_t> levelup_learnset;
  for (int32_t i = 0; i < POKEDEX_POKEMON_MOVES_ENTRIES; ++i) {
    if ((pd_entry->species_id == pd_pokemon_moves[i].pokemon_id)
     && (level >= pd_pokemon_moves[i].level)) {
      // check if move is already in learnset
      bool is_dup = false;
      for (auto it  = levelup_learnset.begin(); 
                it != levelup_learnset.end(); ++it) {
        if (*it == pd_pokemon_moves[i].move_id) {
          is_dup = true;
          break;
        }
      }

      if (!is_dup) {
        levelup_learnset.push_back(pd_pokemon_moves[i].move_id);
      }
    }
  }

  // 3. Randomly select and assign up to 4 moves
  while (levelup_learnset.size() > 0 && num_moves < 4) {
    int32_t new_move_index = rand() % levelup_learnset.size();

    bool move_found = false;
    for (int32_t i = 0; i < POKEDEX_MOVES_ENTRIES; ++i) {
      if (pd_moves[i].id == levelup_learnset[new_move_index]) {
        learn_move(&pd_moves[i]);
        levelup_learnset.erase(levelup_learnset.begin() + new_move_index);
        move_found = true;
        break;
      }
    }
    if (!move_found) {
      exit_w_message("Error: Move exists in learnset, but not in moves!");
    }
  }
  return;
}
/*
 * Helper method to generate a pokemon's IVs
 * Called when initializing a pokemon
 */
void Pokemon::generate_ivs() {
  for (int32_t i = 0; i < 6; ++i)
    ivs[i] = rand() % 16;
  return;
}
/*
 * Helper method to lookup and update a pokemon's stats
 * Called when initializing a pokemon
 */
void Pokemon::lookup_base_stats() {
  // Lookup Base Stats
  for (int32_t i = 0; i < POKEDEX_POKEMON_STATS_ENTRIES; ++i) {
    if (pd_pokemon_stats[i].pokemon_id == pd_entry->id) {
      // we assume that the pokemon_stats.csv lists stat ids in order for one 
      // pokemon at a time
      for (int32_t s = 0; s < 6; s++) {
        base_stats[s] = pd_pokemon_stats[i + s].base_stat;
      }
      break;
    }
  }
  return;
}
/*
 * Helper method to calculate and update a pokemon's stats
 * Called when initializing a pokemon and after level up
 */
void Pokemon::calculate_stats() {
  // Calculate HP
  stats[stat_hp] = ( ((base_stats[stat_hp] + ivs[stat_hp]) * 2 * level) / 100 ) 
                   + level + 10;

  // Calculate Other Stats
  for (int32_t s = 1; s < 6; s++) {
    stats[s] = ( ((base_stats[s] + ivs[s]) * 2 * level) / 100 ) + 5;
  }
  return;
}

/*
 * Pokemon constructor
 */
Pokemon::Pokemon() {
  // pointer arithmetic to select a random pokemon in the pd_pokemon array
  pd_entry = &pd_pokemon[rand() % POKEDEX_POKEMON_ENTRIES];
  strncpy(nickname, pd_entry->identifier, 12);
  pd_species_entry = &pd_pokemon_species[pd_entry->id - 1];

  generate_level();
  populate_moveset();
  generate_ivs();
  lookup_base_stats();
  calculate_stats();
  lookup_type();

  exp = pd_experience[(pd_species_entry->growth_rate_id - 1) * 100 + level - 1]
          .experience;
  current_hp = stats[stat_hp];
  gender = static_cast<gender_t>(rand() % 2);
  shiny = rand() % POKEMON_SHINY_RATE == 0 ? true : false;
  has_owner = false;
}

pd_pokemon_t* Pokemon::get_pd_entry() {
  return pd_entry;
}
pd_pokemon_species_t* Pokemon::get_pd_species_entry() {
  return pd_species_entry;
}
const char* Pokemon::get_nickname() {
  return nickname;
}
void Pokemon::rename(char new_name[13]) {
  strncpy(nickname, new_name, 12);
}
int32_t Pokemon::get_level() {
  return level;
}
int32_t Pokemon::get_total_exp() {
  return exp;
}
int32_t Pokemon::get_exp() {
if (level >= POKEMON_MAX_LEVEL) {
    return 0;
  }
  return 
    exp
    - pd_experience[(pd_species_entry->growth_rate_id - 1) * 100 + level - 1]
      .experience;
}
int32_t Pokemon::get_total_exp_next_level() {
  if (level >= POKEMON_MAX_LEVEL) {
    return 0;
  }
  return 
    pd_experience[(pd_species_entry->growth_rate_id - 1) * 100 + level]
      .experience;
}
int32_t Pokemon::get_exp_next_level() {
  if (level >= POKEMON_MAX_LEVEL) {
    return 0;
  }
  return 
    pd_experience[(pd_species_entry->growth_rate_id - 1) * 100 + level]
      .experience
    - pd_experience[(pd_species_entry->growth_rate_id - 1) * 100 + level - 1]
        .experience;
}
void Pokemon::give_exp(int32_t amount) {
  if (level < POKEMON_MAX_LEVEL) {
    exp += amount;
  }
}
bool Pokemon::process_level_up() { 
  if (exp >= get_total_exp_next_level() && level < POKEMON_MAX_LEVEL) {
    ++level;
    int32_t old_hp = stats[stat_hp];
    calculate_stats();
    current_hp += stats[stat_hp] - old_hp;
    
    // 1. Find levelup learnset
    std::vector<int32_t> levelup_learnset;
    for (int32_t i = 0; i < POKEDEX_POKEMON_MOVES_ENTRIES; ++i) {
      if ((pd_entry->species_id == pd_pokemon_moves[i].pokemon_id)
      && (level == pd_pokemon_moves[i].level)) {
        // check if move is already in learnset
        bool is_dup = false;
        for (auto it  = levelup_learnset.begin(); 
                  it != levelup_learnset.end(); ++it) {
          if (*it == pd_pokemon_moves[i].move_id) {
            is_dup = true;
            break;
          }
        }
        // check if move is already in moveset
        for (int32_t m = 0; m < num_moves; ++m) {
          if (get_move(m)->id == pd_pokemon_moves[i].move_id) {
            is_dup = true;
            break;
          }
        }

        if (!is_dup) {
          levelup_learnset.push_back(pd_pokemon_moves[i].move_id);
        }
      }
    }

    // 2. Teach moves in learnset
    while (levelup_learnset.size() > 0) {
      // 2a. Find move in database
      bool move_found = false;
      for (int32_t i = 0; i < POKEDEX_MOVES_ENTRIES; ++i) {
        if (pd_moves[i].id == levelup_learnset[0]) {
          // 2b. Move found now there are two cases
          //     Open teach pokemon view
          
          char m1[MAX_COL], m2[MAX_COL], m_cancel[MAX_COL];
          if (num_moves < 4) {
            // 2ba. No player choice, move is learned in first available slot
            learn_move(&pd_moves[i]);
            sprintf(m1, "%s learned %s!", nickname, pd_moves[i].identifier);
            render_select_move_getch(this, NULL, -1, m1, NULL, NULL);
          } else {
            // 2bb. Player must select move to forget
            sprintf(m1, "%s wants to learn the move %s.",
                    nickname, pd_moves[i].identifier);
            sprintf(m2, "Which move should be forgotten?");
            sprintf(m_cancel, "STOP LEARNING %s", pd_moves[i].identifier);
            int32_t scroller_pos = 
              select_move_driver(this, &pd_moves[i], m1, m2, m_cancel);

            if (scroller_pos < num_moves && scroller_pos >= 0) {
              // player choose to replace a move
              sprintf(m2, "%s forgot %s and... learned %s",
                    nickname, get_move(scroller_pos)->identifier, 
                    pd_moves[i].identifier);
              overwrite_move(scroller_pos, &pd_moves[i]);
              render_select_move_getch(this, &pd_moves[i], -1, m1, m2, NULL);
            } else if (scroller_pos == num_moves) {
              // cancel was selected
              sprintf(m2, "%s did not learn %s.",
                    nickname, pd_moves[i].identifier);
              render_select_move_getch(this, &pd_moves[i], -1, m1, m2, NULL);
            }
            
          }
          
          // 2c. Move has been handles now, Remove move from learnset
          levelup_learnset.erase(levelup_learnset.begin());
          move_found = true;
          break;
        }
      }
      if (!move_found) {
        exit_w_message("Error: Move exists in learnset, but not in moves!");
      }
    }

    return true;
  } else {
    return false;
  }
}
pd_move_t* Pokemon::get_move(int32_t move_slot) {
  if (move_slot < 0 || move_slot > num_moves) {
    return &pd_moves[164]; // struggle id 165
  }
  return moveset[move_slot];
}
int32_t Pokemon::ai_select_move_slot() {
  if (num_moves > 0 && has_pp()) {
    bool found_move = false;
    int32_t randy;
    while (!found_move) {
      randy = rand() % num_moves;
      if (current_pp[randy] > 0) {
        return randy;
      }
    }
  }
  // if no pp or no moves then pokemon uses struggle
  return -1;
}
int32_t Pokemon::get_num_moves() {
  return num_moves;
}
void Pokemon::learn_move(pd_move_t *m) {
  if (num_moves < 4) {
    moveset[num_moves] = m;
    current_pp[num_moves] = m->pp;
    ++num_moves;
  }
  return;
}
void Pokemon::overwrite_move(int32_t move_slot, pd_move_t *m) {
  if (move_slot < num_moves && move_slot >= 0) {
    moveset[move_slot] = m;
    current_pp[move_slot] = m->pp;
  }
  return;
}
int32_t Pokemon::get_base_stat(stat_id_t stat_id) {
  return base_stats[stat_id];
}
int32_t Pokemon::get_stat(stat_id_t stat_id) {
  return stats[stat_id];
}
int32_t Pokemon::get_iv(stat_id_t stat_id) {
  return ivs[stat_id];
}
int32_t Pokemon::get_current_hp() {
  return current_hp;
}
int32_t Pokemon::get_current_pp(int32_t move_slot) {
  if (move_slot < 0 || move_slot >= num_moves)
    return 0;
  return current_pp[move_slot];
}
int32_t Pokemon::use_pp(int32_t move_slot) {
  if (move_slot < 0 || move_slot >= num_moves)
    return 0;
  if (current_pp[move_slot] == 0)
    return 0;
  --current_pp[move_slot];
  return 1;
}
int32_t Pokemon::restore_pp(int32_t m, int32_t amount) {
  if (current_pp[m] == moveset[m]->pp) {
    return 0;
  }
  // protects against overflowing if amount + move pp > INT_MAX
  // we can fully heal a pokemon by passing amount = INT_MAX
  if (amount >= moveset[m]->pp) {
    current_pp[m] = moveset[m]->pp;
    return 1;
  }
  if (current_pp[m] + amount >= moveset[m]->pp) {
    current_pp[m] = moveset[m]->pp;
    return 1;
  }
  current_pp[m] += amount;
  return 1;
}
bool Pokemon::has_pp() {
  for (int32_t i = 0; i < num_moves; ++i)
    if (current_pp[i] > 0)
      return true;
  return false;
}
bool Pokemon::has_all_pp() {
  for (int32_t i = 0; i < num_moves; ++i)
    if (current_pp[i] != moveset[i]->pp)
      return false;
  return true;
}
void Pokemon::restore_all_pp(int32_t amount) {
  for (int32_t i = 0; i < num_moves; ++i)
    restore_pp(i, amount);
  return;
}

gender_t Pokemon::get_gender() {
  return gender;
}
bool Pokemon::is_shiny() {
  return shiny;
}
int32_t Pokemon::get_type(int32_t slot) {
  return type[slot];
}
int32_t Pokemon::heal(int32_t amount) {
  if (current_hp == stats[stat_hp]) {
    return 0;
  }
  // protects against overflowing if amount + current_hp > INT_MAX
  // we can fully heal a pokemon by passing amount = INT_MAX
  if (amount >= stats[stat_hp]) {
    current_hp = stats[stat_hp];
    return 1;
  }
  if (current_hp + amount >= stats[stat_hp]) {
    current_hp = stats[stat_hp];
    return 1;
  }
  current_hp += amount;
  return 1;
}
void Pokemon::take_damage(int32_t amount) {
  // protects against overflowing if current_hp - amount < INT_MIN
  if (amount > stats[stat_hp]) {
    current_hp = 0;
    return;
  }
  if (current_hp - amount <= 0) {
    current_hp = 0;
    return;
  }
  current_hp -= amount;
  return;
}
bool Pokemon::get_has_owner() {
  return has_owner;
}
void Pokemon::set_has_owner(bool new_value) {
  has_owner = new_value;
  return;
}
bool Pokemon::is_fainted() {
  return current_hp <= 0;
}

const char* type_name(int32_t type_id) {
  if (type_id < 1) {
    return "";
  }
  // handle type_id 1-18
  if (type_id <= 18) {
    return pd_type_names[type_id - 1];
  }
  // handle type_id 10001-10002
  return pd_type_names[type_id - 9983];
}

/*
 * Determines the move priority of two pokemon attempt to use a move for thier
 * respective turns.
 * 
 * Priority is will determined by
 * 1. Move priority
 * 2. Pokemon speed
 * 3. Random chance
 * Only relying on 2 or 3 if the previous metric results in a tie.
 * 
 * Returns a positive integer if first pokemon has priority.
 * Returns a negative integer if second pokemon has priority.
 */
int32_t move_priority(int32_t move_priority_1, int32_t poke_speed_1,
                      int32_t move_priority_2, int32_t poke_speed_2) {
  // 1. Move priority
  int32_t priority = move_priority_1 - move_priority_2;
  if (priority != 0) {
    return priority;
  }

  // 2. Pokemon speed
  priority = poke_speed_1 - poke_speed_2;
  if (priority != 0) {
    return priority;
  }

  // 3. Random chance
  return ((rand() % 2) ? 1 : -1);
}

/*
 * Returns the type effectiveness multiplier of a move against another pokemon
 */
float effectiveness(pd_move_t *attacking_move, Pokemon *defender) {
  int32_t atk_tid = attacking_move->type_id - 1;
  int32_t def_tid_0 = defender->get_type(0) - 1;
  int32_t def_tid_1 = defender->get_type(1) - 1;
  
  float eff = type_effectiveness[atk_tid][def_tid_0];
  // if pokemon has two types
  if (def_tid_1 >= 0) {
    eff *= type_effectiveness[atk_tid][def_tid_1];
  }
  return eff;
}

/*
 * Returns the damage an attack will have on a defending pokemon
 */
int32_t calculate_damage(Pokemon *attacker, pd_move_t *attacking_move, 
                         Pokemon *defender, bool is_critical) {
  float level = attacker->get_level();
  float power = attacking_move->power;
  if (power == -1)
    power = 0;
  float attack, defense;
  if (attacking_move->damage_class_id == 2) {
    // physical attack
    attack = attacker->get_stat(stat_attack);
    defense = defender->get_stat(stat_defense);
  } else if (attacking_move->damage_class_id == 3) {
    // special attack
    attack = attacker->get_stat(stat_sp_atk);
    defense = defender->get_stat(stat_sp_def);
  } else {
    // status (not implemented)
    return 0;
  }
  float critical = is_critical ? 1.5 : 1.0;
  float random = ((rand() % (100 - 85 + 1)) + 85) / 100.0;
  float stab = attacking_move->type_id == attacker->get_type(0) 
            || attacking_move->type_id == attacker->get_type(1)
             ? 1.5 : 1;
  float type = effectiveness(attacking_move, defender);
                           
  return static_cast<int32_t>(
        ( ( (2.0 * level) / 5.0 * power * (attack / defense) ) / 50.0 + 2.0) 
         * critical * random * stab * type
        );
}

/*
 * Returns a boolean, calculated with the appropriate random odds, to indicate
 * if a critical hit occurs.
 */
bool is_critical (Pokemon *attacker, pd_move_t *attacking_move) {
  // status moves can not crit
  if (attacking_move->damage_class_id == 1) 
    return false;
  return (rand() % 256) < attacker->get_base_stat(stat_speed)/2;
}

/*
 * Returns a boolean, calculated with the appropriate random odds, to indicate
 * if a miss occurs.
 */
bool is_miss (pd_move_t *attacking_move) {
  // accuracy not specified means this attack cannot miss
  if (attacking_move->accuracy == -1)
    return false;
  return !( (rand() % 100) < attacking_move->accuracy );
}

/*
 * Calculates and returns the amount of experience gained from defeating a
 * pokemon.
 * 
 * Uses the flat formula that applies up to Gen IV and in Gen VI
 * We will follow the rules for Gen III.
 * https://bulbapedia.bulbagarden.net/wiki/Experience
 */
int32_t experience_gain(Pokemon *opp) {
  float a = opp->get_has_owner() ? 1.5 : 1;
  float b = opp->get_pd_entry()->base_experience;
  float e = 1; // Lucky egg is not implemented.
  float t = 1; // Trading is not implemented.
  float s = 1; // We will use 1 because we are giving all the experience to the
               //   pokemon that knocked out the opponent.
  return (a * t * b * e) / (7 * s);


}