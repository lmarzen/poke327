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
  char nickname[12];
  int32_t level;
  int32_t exp;
  pd_move_t *moveset[4];
  int32_t current_pp[4];
  int32_t num_moves;
  int32_t base_stats[6];
  int32_t stats[6];
  int32_t ivs[6];
  int32_t current_hp;
  gender_t gender;
  bool shiny;
  int32_t type[2];

  void lookup_type();
  void generate_level();
  void populate_moveset();
  void generate_ivs();
  void lookup_base_stats();
  void calculate_stats();

  public:
    Pokemon();
    pd_pokemon_t* get_pd_entry();
    char* get_nickname();
    void rename(char new_name[12]);
    int32_t get_level();
    int32_t get_exp();
    pd_move_t* get_move(int32_t move_slot);
    pd_move_t* get_rand_move();
    int32_t get_num_moves();
    void learn_move(pd_move_t *m);
    int32_t get_base_stat(stat_id_t stat_id);
    int32_t get_stat(stat_id_t stat_id);
    int32_t get_iv(stat_id_t stat_id);
    int32_t get_current_hp();
    int32_t get_current_pp(int32_t m);
    gender_t get_gender();
    bool is_shiny();
    int32_t get_type(int32_t slot);
};

int32_t move_priority(int32_t move_priority_1, int32_t poke_speed_1,
                      int32_t move_priority_2, int32_t poke_speed_2);
int32_t calculate_damage(Pokemon *attacker, pd_move_t *attacking_move, 
                         Pokemon *defender, bool is_critical);
bool is_critical (Pokemon *attacker);
bool is_miss (pd_move_t *attacking_move);

#endif