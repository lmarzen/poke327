#ifndef REGION_H
#define REGION_H

#include <cstdint>
#include <vector>

#include "config.h"
#include "character.h"
#include "heap.h"

typedef enum terrain {
  ter_border,
  ter_boulder,
  ter_tree,
  ter_center,
  ter_mart,
  ter_path,
  ter_grass,
  ter_clearing,
  ter_mountain,
  ter_forest,
  ter_mixed
} terrain_t;

typedef struct tile {
  terrain_t ter;
  char ch;
  int32_t color;
} tile_t;

typedef struct seed {
  int32_t i, j;
  terrain_t ter;
} seed_t;

typedef struct pos {
  int32_t i, j;
} pos_t;

class Region {
  private:
    tile_t tile_arr[MAX_ROW][MAX_COL];
    int32_t N_exit_j, E_exit_i, S_exit_j, W_exit_i;
    std::vector<Character> npc_arr;

  public:
    Region(int32_t N_exit_j, int32_t E_exit_i,
           int32_t S_exit_j, int32_t W_exit_i,
           int32_t place_center, int32_t place_mart);

    void      populate(int32_t num_tnrs);
    terrain_t get_ter(int32_t i, int32_t j);
    char      get_ch(int32_t i, int32_t j);
    int32_t   get_color(int32_t i, int32_t j);
    int32_t   get_N_exit_j();
    int32_t   get_E_exit_i();
    int32_t   get_S_exit_j();
    int32_t   get_W_exit_i();
    void      close_N_exit();
    void      close_E_exit();
    void      close_S_exit();
    void      close_W_exit();
    std::vector<Character>* get_npcs();

    ~Region();
};

double dist(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
int32_t m_dist(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
int32_t rand_outcome(double probability) ;

#endif