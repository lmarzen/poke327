#ifndef POKEMON_H
#define POKEMON_H

#include <cstdint>
#include "config.h"
#include "pokedex.h"

// Uses Gen 2-5 type chart, 
// We are using gen 3 pokemon so fiary type should never be used
static const float type_effectiveness[18][18] = {
/* AKv/DE> Nor Fig Fly Poi Gro Roc Bug Gho Ste Fir Wat Gra Ele Psy Ice Dra Dar Fai */
/* Nor */ {1  ,1  ,1  ,1  ,1  ,0.5,1  ,0  ,0.5,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  },
/* Fig */ {2  ,1  ,0.5,0.5,1  ,2  ,0.5,0  ,2  ,1  ,1  ,1  ,1  ,0.5,2  ,1  ,2  ,1  },
/* Fly */ {1  ,2  ,1  ,1  ,1  ,0.5,2  ,1  ,0.5,1  ,1  ,2  ,0.5,1  ,1  ,1  ,1  ,1  },
/* Poi */ {1  ,1  ,1  ,0.5,0.5,0.5,1  ,0.5,0  ,1  ,1  ,2  ,1  ,1  ,1  ,1  ,1  ,1  },
/* Gro */ {1  ,1  ,0  ,2  ,1  ,2  ,0.5,1  ,2  ,2  ,1  ,0.5,2  ,1  ,1  ,1  ,1  ,1  },
/* Roc */ {1  ,0.5,2  ,1  ,0.5,1  ,2  ,1  ,0.5,2  ,1  ,1  ,1  ,1  ,2  ,1  ,1  ,1  },
/* Bug */ {1  ,0.5,0.5,0.5,1  ,1  ,1  ,0.5,0.5,0.5,1  ,2  ,1  ,2  ,1  ,1  ,2  ,1  },
/* Gho */ {0  ,1  ,1  ,1  ,1  ,1  ,1  ,2  ,0.5,1  ,1  ,1  ,1  ,2  ,1  ,1  ,0.5,1  },
/* Ste */ {1  ,1  ,1  ,1  ,1  ,2  ,1  ,1  ,0.5,0.5,0.5,1  ,0.5,1  ,2  ,1  ,1  ,1  },
/* Fir */ {1  ,1  ,1  ,1  ,1  ,0.5,2  ,1  ,2  ,0.5,0.5,2  ,1  ,1  ,2  ,0.5,1  ,1  },
/* Wat */ {1  ,1  ,1  ,1  ,2  ,2  ,1  ,1  ,1  ,2  ,0.5,0.5,1  ,1  ,1  ,0.5,1  ,1  },
/* Gra */ {1  ,1  ,0.5,0.5,2  ,2  ,0.5,1  ,0.5,0.5,2  ,0.5,1  ,1  ,1  ,0.5,1  ,1  },
/* Ele */ {1  ,1  ,2  ,1  ,0  ,1  ,1  ,1  ,1  ,1  ,2  ,0.5,0.5,1  ,1  ,0.5,1  ,1  },
/* Psy */ {1  ,2  ,1  ,2  ,1  ,1  ,1  ,1  ,0.5,1  ,1  ,1  ,1  ,0.5,1  ,1  ,0  ,1  },
/* Ice */ {1  ,1  ,2  ,1  ,2  ,1  ,1  ,1  ,0.5,0.5,0.5,2  ,1  ,1  ,0.5,2  ,1  ,1  },
/* Dra */ {1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,0.5,1  ,1  ,1  ,1  ,1  ,1  ,2  ,1  ,1  },
/* Dar */ {1  ,0.5,1  ,1  ,1  ,1  ,1  ,2  ,0.5,1  ,1  ,1  ,1  ,2  ,1  ,1  ,0.5,1  },
/* Fai */ {1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  }
};

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
  pd_pokemon_species_t *pd_species_entry;
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
    pd_pokemon_species_t* get_pd_species_entry();
    const char* get_nickname();
    void rename(char new_name[13]);
    int32_t get_level();
    int32_t get_exp();
    int32_t get_exp_next_level();
    int32_t get_total_exp();
    int32_t get_total_exp_next_level();
    void give_exp(int32_t amount);
    bool process_level_up();
    pd_move_t* get_move(int32_t move_slot);
    int32_t ai_select_move_slot();
    int32_t get_num_moves();
    void learn_move(pd_move_t *m);
    void overwrite_move(int32_t move_slot, pd_move_t *m);
    int32_t get_base_stat(stat_id_t stat_id);
    int32_t get_stat(stat_id_t stat_id);
    int32_t get_iv(stat_id_t stat_id);
    int32_t get_current_hp();
    int32_t get_current_pp(int32_t move_slot);
    int32_t use_pp(int32_t move_slot);
    int32_t restore_pp(int32_t m, int32_t amount);
    bool has_pp();
    bool has_all_pp();
    void restore_all_pp(int32_t amount);
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
int32_t experience_gain(Pokemon *opp);

#endif