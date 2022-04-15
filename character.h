#ifndef CHARACTER_H
#define CHARACTER_H

#include <cstdint>
#include <climits>
#include <vector>
#include "config.h"
#include "items.h"
#include "pokemon.h"

typedef enum trainer {
  tnr_pc,
  tnr_hiker,
  tnr_rival,
  tnr_pacer,
  tnr_wanderer,
  tnr_stationary,
  tnr_rand_walker
} trainer_t;

static const int32_t turn_times[11][7] = {
                  /*       PC,   Hiker,   Rival,   Pacer, Wandere, Station,  Walker*/
  /* ter_border   */ {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_boulder  */ {INT_MAX,      10, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_tree     */ {INT_MAX,      10, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_center   */ {     10, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_mart     */ {     10, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_path     */ {     10,      10,      10,      10,      10,      10,      10},
  /* ter_grass    */ {     20,      15,      20,      20,      20,      20,      20},
  /* ter_clearing */ {     10,      10,      10,      10,      10,      10,      10},
  /* ter_mountain */ {INT_MAX,      15, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_forest   */ {INT_MAX,      15, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX},
  /* ter_mixed    */ {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX}
};

typedef enum direction {
  dir_n,
  dir_ne,
  dir_e,
  dir_se,
  dir_s,
  dir_sw,
  dir_w,
  dir_nw
} direction_t;

static const int32_t dir_offsets[8][2] = {
            /* { i, j} */
  /* dir_n  */ {-1, 0},
  /* dir_ne */ {-1, 1},
  /* dir_e  */ { 0, 1},
  /* dir_se */ { 1, 1},
  /* dir_s  */ { 1, 0},
  /* dir_sw */ { 1,-1},
  /* dir_w  */ { 0,-1},
  /* dir_nw */ {-1,-1}
};

// Abstract base class
class Character {
  protected:
    int32_t pos_i, pos_j;
    trainer_t tnr;
    int32_t movetime;
    char ch = CHAR_UNDEFINED;
    int32_t color = CHAR_COLOR_UNDEFINED;
    bool defeated = false;
    direction_t dir;
    std::vector<bag_slot_t> bag;
    std::vector<Pokemon> party;
    char nickname[12];
    
  public:
    int32_t get_movetime();
    void step_movetime(int32_t amount);
    int32_t get_i();
    int32_t get_j();
    trainer_t get_tnr();
    char get_ch();
    int32_t get_color();
    bool is_defeated();
    void set_defeated(bool d);
    void process_movement_turn();
    int32_t num_in_bag(item_t i);
    void remove_item_from_bag(item_t i);
    void add_item_to_bag(item_t i, int32_t cnt);
    int32_t num_bag_slots();
    bag_slot_t peek_bag_slot(int32_t index);
    int32_t add_pokemon(Pokemon *p);
    Pokemon* get_pokemon(int32_t i);
    int32_t party_size();
    const char* get_nickname();
    void rename(char new_name[12]);

  friend void move_along_gradient(Character *c, 
                                  int32_t dist_map[MAX_ROW][MAX_COL]);
};

// Derived class
class Pc : public Character {
  int32_t reg_x;
  int32_t reg_y;
  int32_t quit_game;

  public:
    Pc(int32_t r_x, int32_t r_y);
    ~Pc();

    int32_t get_x();
    int32_t get_y();
    bool is_quit_game();
    void set_quit_game(bool q);
    void pick_starter_driver();

  friend int32_t process_pc_move_attempt(direction_t dir);
  friend void pc_next_region(int32_t to_rx,   int32_t to_ry, 
                             int32_t from_rx, int32_t from_ry);
};

// Derived abstract class
class Npc : public Character {

  public:
    Npc(trainer_t tnr, int32_t i, int32_t j, int32_t init_movetime);
    ~Npc();
};

  
#endif