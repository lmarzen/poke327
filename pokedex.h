#ifndef POKEDEX_H
#define POKEDEX_H

#include <cstdint>
#include <climits>
#include <string>
#include <vector>

#include "config.h"

typedef struct pd_pokemon {
  int32_t id;
  char    identifier[13];
  int32_t species_id;
  int32_t height;
  int32_t weight;
  int32_t base_experience;
  int32_t order;
  int32_t is_default;
} pd_pokemon_t;

typedef struct pd_move {
  int16_t id;
  char    identifier[17];
  int32_t generation_id;
  int32_t type_id;
  int32_t power;
  int32_t pp;
  int32_t accuracy;
  int32_t priority;
  int32_t target_id;
  int32_t damage_class_id;
  int32_t effect_id;
  int32_t effect_chance;
  int32_t contest_type_id;
  int32_t contest_effect_id;
  int32_t super_contest_effect_id;
} pd_move_t;

typedef struct pd_pokemon_move {
  int32_t pokemon_id;
  // int32_t version_group_id;
  int16_t move_id;
  // int32_t pokemon_move_method_id;
  int32_t level;
  int32_t order;
} pd_pokemon_move_t;

typedef struct pd_pokemon_species {
  int32_t id;
  char    identifier[13];
  int32_t generation_id;
  int32_t evolves_from_species_id;
  int32_t evolution_chain_id;
  int32_t color_id;
  int32_t shape_id;
  int32_t habitat_id;
  int32_t gender_rate;
  int32_t capture_rate;
  int32_t base_happiness;
  int32_t is_baby;
  int32_t hatch_counter;
  int32_t has_gender_differences;
  int32_t growth_rate_id;
  int32_t forms_switchable;
  int32_t is_legendary;
  int32_t is_mythical;
  int32_t order;
  int32_t conquest_order;
} pd_pokemon_species_t;

typedef struct pd_pokemon_stat {
  int32_t pokemon_id;
  int32_t stat_id;
  int32_t base_stat;
  int32_t effort;
} pd_pokemon_stat_t;

typedef struct pd_experience {
  int32_t growth_rate_id;
  int32_t level;
  int32_t experience;
} pd_experience_t;

typedef struct pd_pokemon_type {
  int32_t pokemon_id;
  int32_t type_id;
  int32_t slot;
} pd_pokemon_type_t;

// Global pokedex data
extern pd_pokemon_t pd_pokemon[POKEDEX_POKEMON_ENTRIES];
extern pd_move_t pd_moves[POKEDEX_MOVES_ENTRIES];
extern pd_pokemon_move_t pd_pokemon_moves[POKEDEX_POKEMON_MOVES_ENTRIES];
extern pd_pokemon_species_t pd_pokemon_species[POKEDEX_POKEMON_SPECIES_ENTRIES];
extern pd_pokemon_stat_t pd_pokemon_stats[POKEDEX_POKEMON_STATS_ENTRIES];
extern pd_experience_t pd_experience[POKEDEX_EXPERIENCE_ENTRIES];
extern char pd_type_names[POKEDEX_TYPE_NAMES_ENTRIES][11];
extern pd_pokemon_type_t pd_pokemon_types[POKEDEX_POKEMON_TYPES_ENTRIES];

void init_pd_pokemon();
void init_pd_moves();
void init_pd_pokemon_moves();
void init_pd_pokemon_species();
void init_pd_pokemon_stats();
void init_pd_experience();
void init_pd_type_name();
void init_pd_pokemon_types();
void init_pd();

#endif