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
  char nickname[13];
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
  bool has_owner;

  void lookup_type();
  void generate_level();
  void populate_moveset();
  void generate_ivs();
  void lookup_base_stats();
  void calculate_stats();

  public:
    Pokemon();
    pd_pokemon_t* get_pd_entry();
    const char* get_nickname();
    void rename(char new_name[13]);
    int32_t get_level();
    int32_t get_exp();
    int32_t get_exp_next_level();
    pd_move_t* get_move(int32_t move_slot);
    int32_t ai_select_move_slot();
    int32_t get_num_moves();
    void learn_move(pd_move_t *m);
    int32_t get_base_stat(stat_id_t stat_id);
    int32_t get_stat(stat_id_t stat_id);
    int32_t get_iv(stat_id_t stat_id);
    int32_t get_current_hp();
    int32_t get_current_pp(int32_t move_slot);
    int32_t use_pp(int32_t move_slot);
    int32_t restore_pp(int32_t m, int32_t amount);
    bool has_pp();
    gender_t get_gender();
    bool is_shiny();
    int32_t get_type(int32_t slot);
    int32_t heal(int32_t amount);
    void take_damage(int32_t amount);
    bool get_has_owner();
    void set_has_owner(bool new_value);
    bool is_fainted();
};

const char* type_name(int32_t type_id);
int32_t move_priority(int32_t move_priority_1, int32_t poke_speed_1,
                      int32_t move_priority_2, int32_t poke_speed_2);
float effectiveness(pd_move_t *attacking_move, Pokemon *defender);
int32_t calculate_damage(Pokemon *attacker, pd_move_t *attacking_move, 
                         Pokemon *defender, bool is_critical);
bool is_critical(Pokemon *attacker, pd_move_t *attacking_move);
bool is_miss(pd_move_t *attacking_move);

#endif