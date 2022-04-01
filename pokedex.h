#ifndef POKEDEX_H
#define POKEDEX_H

#include <cstdint>
#include <climits>
#include <string>
#include <vector>

#include "config.h"

typedef struct pokedex_pokemon {
  int32_t     id;
  std::string identifier;
  int32_t     species_id;
  int32_t     height;
  int32_t     weight;
  int32_t     base_experience;
  int32_t     order;
  bool        is_default;
} pokemon_t;

typedef struct pokedex_move {
  int32_t     id;
  std::string identifier;
  int32_t     generation_id;
  int32_t     type_id;
  int32_t     power;
  int32_t     pp;
  int32_t     accuracy;
  int32_t     priority;
  int32_t     target_id;
  int32_t     damage_class_id;
  int32_t     effect_id;
  int32_t     effect_chance;
  int32_t     contest_type_id;
  int32_t     contest_effect_id;
  int32_t     super_contest_effect_id;
} move_t;

typedef struct pokedex_pokemon_move {
  int32_t     pokemon_id;
  int32_t     version_group_id;
  int32_t     move_id;
  int32_t     pokemon_move_method_id;
  int32_t     level;
  int32_t     order;
} pokemon_move_t;

typedef struct pokedex_pokemon_species {
  int32_t     id;
  std::string identifier;
  int32_t     generation_id;
  int32_t     evolves_from_species_id;
  int32_t     evolution_chain_id;
  int32_t     color_id;
  int32_t     shape_id;
  int32_t     habitat_id;
  int32_t     gender_rate;
  int32_t     capture_rate;
  int32_t     base_happiness;
  int32_t     is_baby;
  int32_t     hatch_counter;
  int32_t     has_gender_differences;
  int32_t     growth_rate_id;
  int32_t     forms_switchable;
  int32_t     is_legendary;
  int32_t     is_mythical;
  int32_t     order;
  int32_t     conquest_order;
} pokemon_species_t;

typedef struct pokedex_experience {
  int32_t     growth_rate_id;
  int32_t     level;
  int32_t     experience;
} experience_t;

typedef struct pokedex_type_name {
  int32_t     type_id;
  std::string name;
} type_name_t;

// Global pokedex data
extern std::vector<pokemon_t> pokedex_pokemon_data;
extern std::vector<move_t> pokedex_moves;
extern std::vector<pokemon_move_t> pokedex_pokemon_moves;
extern std::vector<pokemon_species_t> pokedex_pokemon_species;
extern std::vector<experience_t> pokedex_experience;
extern std::vector<type_name_t> pokedex_type_name;

void init_pokedex_pokemon();
void init_pokedex_moves();
void init_pokedex_pokemon_moves();
void init_pokedex_pokemon_species();
void init_pokedex_experience();
void init_pokedex_type_name();
void init_pokedex();


#endif