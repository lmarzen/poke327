#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "config.h"
#include "region.h"

/*
 * returns the distance between 2 points
 */
double dist(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
   return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

/*
 * returns the manhatten distance between 2 points
 */
int32_t m_dist(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
   return abs(x2 - x1) + abs(y2 - y1);
}

/*
 * Will decide if an event occurs given its probability
 * returns 1 if an event should happen, otherwise 0.
 */
int32_t rand_outcome(double probability) {
   return rand() < probability * ((double)RAND_MAX + 1.0);
}

/*******************************************************************************
* Region Class
*******************************************************************************/
/*
 * Region Constructor
 * Initializes a region...
 * 
 * Path exit points will be generated at the locations specified.
 * For random path exit point specify -1
 *
 * place_center and place_mart parameters specify whether or not to place a 
 * poke center and/or a poke mart in a region. 0 to not place a building, 
 * 1 to place a building
 */
Region::Region(int32_t N_exit_j, int32_t E_exit_i,
               int32_t S_exit_j, int32_t W_exit_i,
               int32_t place_center, int32_t place_mart)
{
  int32_t randy; // note... int num = (rand() % (upper - lower + 1)) + lower;

  // create a random number of random seeds
  int32_t num_seeds = (rand() % (MAX_SEEDS_PER_REGION - MIN_SEEDS_PER_REGION + 1)) 
                      + MIN_SEEDS_PER_REGION;

  // allocate memory for seeds, each seed has x and y
  std::vector<seed_t> seed_arr(num_seeds);
  
  // initialize each seed with a random set of cordinates
  // at least 2 grass and 2 clearings seeds. (req)
  seed_arr[0].i = rand() % MAX_ROW;
  seed_arr[0].j = rand() % MAX_COL;
  seed_arr[0].ter = ter_clearing;
  seed_arr[1].i = rand() % MAX_ROW;
  seed_arr[1].j = rand() % MAX_COL;
  seed_arr[1].ter = ter_clearing;
  seed_arr[2].i = rand() % MAX_ROW;
  seed_arr[2].j = rand() % MAX_COL;
  seed_arr[2].ter = ter_grass; 
  seed_arr[3].i = rand() % MAX_ROW;
  seed_arr[3].j = rand() % MAX_COL;
  seed_arr[3].ter = ter_grass;

  //  remaining seeds get random terrain type
  for (int32_t i = 4; i < num_seeds; i++) {
    randy = rand() % 100; 
    seed_arr[i].i = rand() % MAX_ROW;
    seed_arr[i].j = rand() % MAX_COL;
    if (randy >= 0 && randy < 25) {
      seed_arr[i].ter = ter_grass;
    } else if (randy >= 25 && randy < 50) {
      seed_arr[i].ter = ter_clearing;
    } else if (randy >= 50 && randy < 70) {
      seed_arr[i].ter = ter_mixed;
    } else if (randy >= 70 && randy < 85) {
      seed_arr[i].ter = ter_mountain;
    } else if (randy >= 85 && randy < 100) {
      seed_arr[i].ter = ter_forest;
    }
  }

  // populate each tile by assigning it the terrain type of the closest seed
  // does not calculate for the outer most boarder because this space will 
  // be boulders or path.
  for (int32_t i = 0; i < MAX_ROW ; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      if (i == 0 || i == MAX_ROW - 1 || j == 0 || j == MAX_COL - 1) {
        tile_arr[i][j].ter = ter_border;
      } else {
        int32_t closest_seed = 0;
        double closest_dist = dist(seed_arr[0].j, seed_arr[0].i, j, i);

        for (int32_t k = 1; k < num_seeds; k++) {
          double temp_dist = dist(seed_arr[k].j, seed_arr[k].i, j, i);
          if (temp_dist < closest_dist) {
            closest_seed = k;
            closest_dist = temp_dist;
          }
        }

        tile_arr[i][j].ter = seed_arr[closest_seed].ter;
        if (tile_arr[i][j].ter == ter_mixed) {
          randy = rand() % 10;
          if (randy <= 3) {// 40%  grass
            tile_arr[i][j].ter = ter_grass;
          } else if (randy >= 4 && randy <= 6) { //30% clearing
            tile_arr[i][j].ter = ter_clearing;
          } else if (randy >= 7 && randy <= 8) { // 20% tree
            tile_arr[i][j].ter = ter_tree;
          }  else { // 10% boulder
            tile_arr[i][j].ter = ter_boulder;
          }
        }

      }
    }
  }

  // set path exit points
  // generate random exits if specified exit is -1.
  // exit cannot be a corner
  if (N_exit_j == -1) {
    this->N_exit_j = (rand() % (MAX_COL - 2)) + 1;
  } else {
    this->N_exit_j = N_exit_j;
  }
  tile_arr[0][this->N_exit_j].ter = ter_path;
  if (E_exit_i == -1) {
    this->E_exit_i = (rand() % (MAX_ROW - 2)) + 1;
  } else {
    this->E_exit_i = E_exit_i;
  }
  tile_arr[this->E_exit_i][MAX_COL - 1].ter = ter_path;
  if (S_exit_j == -1) {
    this->S_exit_j = (rand() % (MAX_COL - 2)) + 1;
  } else {
    this->S_exit_j = S_exit_j;
  }
  tile_arr[MAX_ROW - 1][this->S_exit_j].ter = ter_path;
  if (W_exit_i == -1) {
    this->W_exit_i = (rand() % (MAX_ROW - 2)) + 1;
  } else {
    this->W_exit_i = W_exit_i;
  }
  tile_arr[this->W_exit_i][0].ter = ter_path;

  // W->E path
  // prefers to generate paths between ter_types
  int32_t path_i = this->W_exit_i;
  int32_t path_j = 1;
  tile_arr[path_i][path_j].ter = ter_path;
  while (path_j != MAX_COL - 2) {
    // find the closest seed
    int32_t closest_seed = 0;
    double closest_dist = dist(seed_arr[0].j, seed_arr[0].i, path_j, path_i);
    for (int32_t k = 1; k < num_seeds; k++) {
      double temp_dist = dist(seed_arr[k].j, seed_arr[k].i, path_j, path_i);
      if (temp_dist < closest_dist) {
        closest_seed = k;
        closest_dist = temp_dist;
      }
    }

    // step the path in a direction furthest from the closest seed
    // direction is weighted to head towards the exit especially near the end
    // do not go to other path tiles or backwards. 
    // (only sideways and forward progress is allowed)
    double E_path_weight;
    double N_path_weight = INT32_MIN;
    double S_path_weight = INT32_MIN;
    double dist_to_seed = dist(seed_arr[closest_seed].j, 
                               seed_arr[closest_seed].i, 
                               path_j, path_i);
    double dist_to_exit = dist(MAX_COL - 1, this->E_exit_i, path_j, path_i);
    E_path_weight = 0.2*(dist(seed_arr[closest_seed].j, seed_arr[closest_seed].i, path_j + 1, path_i) - dist_to_seed) // prefer terrain boarders
                  + 0.5*(rand() % 10); // ensure random progress is made towards exit
    if (path_i - 1 != 0 && tile_arr[path_i - 1][path_j].ter != ter_path) {
      N_path_weight = 0.2*(dist(seed_arr[closest_seed].j, seed_arr[closest_seed].i, path_j,  path_i - 1) - dist_to_seed) // prefer terrain boarders
                    + 0.05*path_j*(dist_to_exit - dist(MAX_COL - 1, this->E_exit_i, path_j, path_i - 1)) // head towards the exit especially near the end
                    + 0.05*path_i; // dont hug walls
    }
    if (path_i + 1 != MAX_ROW - 1 && tile_arr[path_i + 1][path_j].ter != ter_path) {
      S_path_weight = 0.2*(dist(seed_arr[closest_seed].j, seed_arr[closest_seed].i, path_j, path_i + 1) - dist_to_seed) // prefer terrain boarders
                    + 0.05*path_j*(dist_to_exit - dist(MAX_COL - 1, this->E_exit_i, path_j, path_i + 1)) //  head towards the exit especially near the end
                    + 0.05*(MAX_ROW - path_i); // dont hug walls
    }

    if(E_path_weight >= N_path_weight && E_path_weight >= S_path_weight) {
      ++path_j;
    } else if (N_path_weight >= S_path_weight) {
      --path_i;
    } else {
      ++path_i;
    }
    tile_arr[path_i][path_j].ter = ter_path;
  }

  while (path_i > this->E_exit_i) {
    --path_i;
    tile_arr[path_i][path_j].ter = ter_path;
  }
  while (path_i < this->E_exit_i) {
    ++path_i;
    tile_arr[path_i][path_j].ter = ter_path;
  }

  // N->S path
  path_i = 1;
  path_j = this->N_exit_j;
  tile_arr[path_i][path_j].ter = ter_path;
  while (path_i != MAX_ROW - 2) {
    // find the closest seed
    int32_t closest_seed = 0;
    double closest_dist = dist(seed_arr[0].j, seed_arr[0].i, path_j, path_i);
    for (int32_t k = 1; k < num_seeds; k++) {
      double temp_dist = dist(seed_arr[k].j, seed_arr[k].i, path_j, path_i);
      if (temp_dist < closest_dist) {
        closest_seed = k;
        closest_dist = temp_dist;
      }
    }

    // step the path in a direction furthest from the closest seed
    // direction is weighted to head towards the exit especially near the end
    // do not go to other path tiles or backwards. 
    // (only sideways and forward progress is allowed)
    double S_path_weight = INT32_MIN;
    double E_path_weight = INT32_MIN;
    double W_path_weight = INT32_MIN;
    double dist_to_seed = dist(seed_arr[closest_seed].j, 
                               seed_arr[closest_seed].i, 
                               path_j, path_i);
    double dist_to_exit = dist(this->S_exit_j, MAX_ROW - 1, path_j, path_i);
    S_path_weight = 0.2*(dist(seed_arr[closest_seed].j, seed_arr[closest_seed].i, path_j, path_i + 1) - dist_to_seed)
                  + 0.5*(rand() % 10);
    if (path_j + 1 != MAX_COL - 1 && tile_arr[path_i][path_j + 1].ter != ter_path) {
      E_path_weight = 0.2*(dist(seed_arr[closest_seed].j, seed_arr[closest_seed].i, path_j + 1,  path_i) - dist_to_seed)
                    + 0.1*path_i*(dist_to_exit - dist(this->S_exit_j, MAX_ROW - 1, path_j + 1, path_i))
                    + 0.05*(MAX_COL - path_j); // dont hug walls;;
    }
    if (path_j - 1 != 0 && tile_arr[path_i][path_j - 1].ter != ter_path) {
      W_path_weight = 0.2*(dist(seed_arr[closest_seed].j, seed_arr[closest_seed].i, path_j - 1, path_i) - dist_to_seed)
                    + 0.1*path_i*(dist_to_exit - dist(this->S_exit_j, MAX_ROW - 1, path_j - 1, path_i))
                    + 0.05*path_j; // dont hug walls;
    }

    if(S_path_weight >= E_path_weight && S_path_weight >= W_path_weight) {
      ++path_i;
    } else if (E_path_weight >= W_path_weight) {
      ++path_j;
    } else {
      --path_j;
    }

    // if we interest the W->E path, the follow it for a random amount of tiles
    if (tile_arr[path_i][path_j].ter == ter_path) {
      int32_t num_tiles_to_trace = rand() % (MAX_COL/2);
      // follow either E or W, whatever will lead us closer the the S exit
      int32_t heading = 1; // 1 is E, -1 is W
      if (path_j > S_exit_j) {
        heading = -1;
      }
      while (num_tiles_to_trace != 0 && path_j > 1 && path_j < MAX_COL - 2 && path_i != MAX_ROW - 3) {
        if (tile_arr[path_i][path_j + heading].ter  == ter_path) {
          path_j += heading;
          --num_tiles_to_trace;
        } else if (tile_arr[path_i + 1][path_j].ter  == ter_path) {
          ++path_i;
          --num_tiles_to_trace;
        } else if (tile_arr[path_i - 1][path_j].ter  == ter_path) {
            --path_i;
            --num_tiles_to_trace;
        }
      }
    }

    tile_arr[path_i][path_j].ter = ter_path;
  }

  while (path_j > this->S_exit_j) {
    --path_j;
    tile_arr[path_i][path_j].ter = ter_path;
  }
  while (path_j < this->S_exit_j) {
    ++path_j;
    tile_arr[path_i][path_j].ter = ter_path;
  }

  // randomly select tiles and place a poke center if location is valid
  // poke centers must be placed next to a path'
  while (place_center != 0) {
    pos_t c_seed;
    c_seed.i = (rand() % (MAX_ROW - 4)) + 1;
    c_seed.j = (rand() % (MAX_COL - 4)) + 1;

    if ( tile_arr[c_seed.i][c_seed.j].ter != ter_path
      && tile_arr[c_seed.i + 1][c_seed.j].ter != ter_path
      && tile_arr[c_seed.i][c_seed.j + 1].ter != ter_path
      && tile_arr[c_seed.i + 1][c_seed.j + 1].ter != ter_path
      && 
      (    tile_arr[c_seed.i - 1][c_seed.j].ter == ter_path
        || tile_arr[c_seed.i - 1][c_seed.j + 1].ter == ter_path
        || tile_arr[c_seed.i][c_seed.j - 1].ter == ter_path
        || tile_arr[c_seed.i + 1][c_seed.j - 1].ter == ter_path
        || tile_arr[c_seed.i][c_seed.j + 2].ter == ter_path
        || tile_arr[c_seed.i + 1][c_seed.j + 2].ter == ter_path
        || tile_arr[c_seed.i + 2][c_seed.j].ter == ter_path
        || tile_arr[c_seed.i + 2][c_seed.j + 1].ter == ter_path
      ) ) {
      tile_arr[c_seed.i][c_seed.j].ter = ter_center;
      tile_arr[c_seed.i + 1][c_seed.j].ter = ter_center;
      tile_arr[c_seed.i][c_seed.j + 1].ter = ter_center;
      tile_arr[c_seed.i + 1][c_seed.j + 1].ter = ter_center;
      place_center = 0;
    }
  }

  while (place_mart != 0) {
    pos_t c_seed;
    c_seed.i = (rand() % (MAX_ROW - 4)) + 1;
    c_seed.j = (rand() % (MAX_COL - 4)) + 1;

    if ( tile_arr[c_seed.i][c_seed.j].ter != ter_path
      && tile_arr[c_seed.i + 1][c_seed.j].ter != ter_path
      && tile_arr[c_seed.i][c_seed.j + 1].ter != ter_path
      && tile_arr[c_seed.i + 1][c_seed.j + 1].ter != ter_path
      && tile_arr[c_seed.i][c_seed.j].ter != ter_center
      && tile_arr[c_seed.i + 1][c_seed.j].ter != ter_center
      && tile_arr[c_seed.i][c_seed.j + 1].ter != ter_center
      && tile_arr[c_seed.i + 1][c_seed.j + 1].ter != ter_center
      && 
      (    tile_arr[c_seed.i - 1][c_seed.j].ter == ter_path
        || tile_arr[c_seed.i - 1][c_seed.j + 1].ter == ter_path
        || tile_arr[c_seed.i][c_seed.j - 1].ter == ter_path
        || tile_arr[c_seed.i + 1][c_seed.j - 1].ter == ter_path
        || tile_arr[c_seed.i][c_seed.j + 2].ter == ter_path
        || tile_arr[c_seed.i + 1][c_seed.j + 2].ter == ter_path
        || tile_arr[c_seed.i + 2][c_seed.j].ter == ter_path
        || tile_arr[c_seed.i + 2][c_seed.j + 1].ter == ter_path
      ) ) {
      tile_arr[c_seed.i][c_seed.j].ter = ter_mart;
      tile_arr[c_seed.i + 1][c_seed.j].ter = ter_mart;
      tile_arr[c_seed.i][c_seed.j + 1].ter = ter_mart;
      tile_arr[c_seed.i + 1][c_seed.j + 1].ter = ter_mart;
      place_mart = 0;
    }
  }

  // Assign tile character symbols and colors
  for (int32_t i = 0; i < MAX_ROW ; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {   
      switch (tile_arr[i][j].ter) {
      case ter_border:
        tile_arr[i][j].ch =  CHAR_BORDER;
        tile_arr[i][j].color = CHAR_COLOR_BORDER;
        break;
      case ter_clearing:
        tile_arr[i][j].ch =  CHAR_CLEARING;
        tile_arr[i][j].color = CHAR_COLOR_CLEARING;
        break;
      case ter_grass:
        tile_arr[i][j].ch =  CHAR_GRASS;
        tile_arr[i][j].color = CHAR_COLOR_GRASS;
        break;
      case ter_boulder:
        tile_arr[i][j].ch =  CHAR_BOULDER;
        tile_arr[i][j].color = CHAR_COLOR_BOULDER;
        break;
      case ter_tree:
        tile_arr[i][j].ch =  CHAR_TREE;
        tile_arr[i][j].color = CHAR_COLOR_TREE;
        break;
      case ter_mountain:
        tile_arr[i][j].ch =  CHAR_MOUNTAIN;
        tile_arr[i][j].color = CHAR_COLOR_MOUNTAIN;
        break;
      case ter_forest:
        tile_arr[i][j].ch =  CHAR_FOREST;
        tile_arr[i][j].color = CHAR_COLOR_FOREST;
        break;
      case ter_path:
        tile_arr[i][j].ch =  CHAR_PATH;
        tile_arr[i][j].color = CHAR_COLOR_PATH;
        break;
      case ter_center:
        tile_arr[i][j].ch =  CHAR_CENTER;
        tile_arr[i][j].color = CHAR_COLOR_CENTER;
        break;
      case ter_mart:
        tile_arr[i][j].ch =  CHAR_MART;
        tile_arr[i][j].color = CHAR_COLOR_MART;
        break;
      default:
        tile_arr[i][j].ch =  CHAR_UNDEFINED;
        tile_arr[i][j].color = CHAR_COLOR_UNDEFINED;
      }
    }
  }
}

/*
 * Populates a region with the specified number of trainers
 * 
 * For random number of trainers, specify -1
 */
void Region::populate(int32_t num_tnrs) 
{
  if (num_tnrs < 0) {
    // generate a random number of npcs to attempt to spawn
    num_tnrs = (rand() % (MAX_TRAINERS - MIN_TRAINERS + 1)) + MIN_TRAINERS;
  }
  
  for (int32_t m = 0; m < num_tnrs; m++) {
    int32_t spawn_attempts = 5;
    while (spawn_attempts != 0) {
      int32_t ti = (rand() % (MAX_ROW - 2)) + 1;
      int32_t tj = (rand() % (MAX_COL - 2)) + 1;
      trainer_t tt;
      if (m == 0) {
        tt = tnr_rival;
      } else if (m == 1) {
        tt = tnr_hiker;
      } else {
        tt = static_cast<trainer_t>((rand() % (tnr_rand_walker - tnr_hiker + 1)) + tnr_hiker);
      }
      int32_t is_valid = 1;

      // verify this npc can move on the tile on to the tile it spawns on
      if (turn_times[tile_arr[ti][tj].ter][tt] == INT_MAX) {
        is_valid = 0;
      }

      // verify no other npcs occupy this space
      if (is_valid) {
        for (auto it = npc_arr.begin(); it != npc_arr.end(); ++it) {
          if (it->get_i() == ti && it->get_j() == tj) {
            is_valid = 0;
            break;
          }
        }
      }

      int32_t tmt = turn_times[tile_arr[ti][tj].ter][tt];

      if (is_valid) {
        npc_arr.push_back(Npc(tt, ti, tj, tmt));
        
        // give new trainer some pokemon 
        // at least 1, then 60% chance for n+1 pokemon, max of 6 pokemon
        npc_arr.back().add_pokemon(new Pokemon());
        while (rand() % 100 < 60 && npc_arr.back().get_party_size() < 6) {
          npc_arr.back().add_pokemon(new Pokemon());
        }

        spawn_attempts = 0;
      } else {
        --spawn_attempts;
      }

    }
  }
  return;
}

terrain_t Region::get_ter(int32_t i, int32_t j) {
  return tile_arr[i][j].ter;
}  
char Region::get_ch(int32_t i, int32_t j) {
  return tile_arr[i][j].ch;
}
int32_t Region::get_color(int32_t i, int32_t j) {
  return tile_arr[i][j].color;
}
int32_t Region::get_N_exit_j() {
  return N_exit_j;
}
int32_t Region::get_E_exit_i() {
  return E_exit_i;
}
int32_t Region::get_S_exit_j() {
  return S_exit_j;
}
int32_t Region::get_W_exit_i() {
  return W_exit_i;
}
void Region::close_N_exit() {
  tile_arr[0][N_exit_j].ter = ter_border;
  tile_arr[0][N_exit_j].ch = CHAR_BORDER;
  tile_arr[0][N_exit_j].color = CHAR_COLOR_BORDER;
}
void Region::close_E_exit() {
  tile_arr[E_exit_i][MAX_COL - 1].ter = ter_border;
  tile_arr[E_exit_i][MAX_COL - 1].ch = CHAR_BORDER;
  tile_arr[E_exit_i][MAX_COL - 1].color = CHAR_COLOR_BORDER;
}
void Region::close_S_exit() {
  tile_arr[MAX_ROW - 1][S_exit_j].ter = ter_border;
  tile_arr[MAX_ROW - 1][S_exit_j].ch = CHAR_BORDER;
  tile_arr[MAX_ROW - 1][S_exit_j].color = CHAR_COLOR_BORDER;
}
void Region::close_W_exit(){
  tile_arr[W_exit_i][0].ter = ter_border;
  tile_arr[W_exit_i][0].ch = CHAR_BORDER;
  tile_arr[W_exit_i][0].color = CHAR_COLOR_BORDER;
}
std::vector<Character>* Region::get_npcs() {
  return &npc_arr;
}
Region::~Region() {
  npc_arr.clear();
  return;
}