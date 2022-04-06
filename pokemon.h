#ifndef POKEMON_H
#define POKEMON_H

#include <cstdint>
#include "config.h"
#include "pokedex.h"

typedef enum gender {
  gender_male,
  gender_female
} gender_t;

typedef enum stat_id {
  stat_hp,
  stat_attack,
  stat_defense,
  stat_sp_atk,
  stat_sp_def,
  stat_speed
} stat_id_t;

class Pokemon {
  pd_pokemon_t *pd_entry;
  char nickname[31];
  int32_t level;
  int32_t exp;
  pd_move_t *moveset[4];
  int32_t stats[6];
  int32_t ivs[6];
  int32_t current_hp;
  gender_t gender;
  bool shiny;

  void generate_level();
  void populate_moveset();
  void generate_ivs();
  void calculate_stats();

  public:
    Pokemon();
    pd_pokemon_t* get_pd_entry();
    char* get_nickname();
    void rename(char new_name[30]);
    int32_t get_level();
    int32_t get_exp();
    pd_move_t* get_move(int32_t move_slot);
    int32_t get_stat(stat_id_t stat_id);
    int32_t get_iv(stat_id_t stat_id);
    gender_t get_gender();
    bool is_shiny();
};

#endif