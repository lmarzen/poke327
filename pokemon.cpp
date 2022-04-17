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
      // check if move is already in moveset
      bool is_dup = false;
      for (auto it  = levelup_learnset.begin(); 
                it != levelup_learnset.end(); ++it) {
        if (*it == pd_pokemon_moves[i].move_id) {
          is_dup = true;
        }
      }

      if (!is_dup) {
        levelup_learnset.push_back(pd_pokemon_moves[i].move_id);
      }
    }
  }

  // 3. Randomly select and assign up to 2 moves
  while (levelup_learnset.size() > 0 && num_moves < 2) {
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
  pd_entry = pd_pokemon + (rand() % POKEDEX_POKEMON_ENTRIES);
  strncpy(nickname, pd_entry->identifier, 30);

  generate_level();
  populate_moveset();
  generate_ivs();
  lookup_base_stats();
  calculate_stats();
  lookup_type();

  exp = 0;
  current_hp = stats[stat_hp];
  gender = static_cast<gender_t>(rand() % 2);
  shiny = rand() % POKEMON_SHINY_RATE == 0 ? true : false;
  has_owner = false;
}

pd_pokemon_t* Pokemon::get_pd_entry() {
  return pd_entry;
}
const char* Pokemon::get_nickname() {
  return nickname;
}
void Pokemon::rename(char new_name[12]) {
  strncpy(nickname, new_name, 12);
}
int32_t Pokemon::get_level() {
  return level;
}
int32_t Pokemon::get_exp() {
  return exp;
}
int32_t Pokemon::get_exp_next_level() {
  // TODO
  return 0;
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
  // TODO
  return 1.0;
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