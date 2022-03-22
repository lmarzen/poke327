#include <limits.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "region.h"
#include "trainer_events.h"
#include "global_events.h"

extern region_t *region_ptr[WORLD_SIZE][WORLD_SIZE];

/*
 * Process user input
 * Update which region is being displayed, generate new regions as needed.
 */
void process_input (character_t *pc, int32_t *region_x, int32_t *region_y, int32_t *quit) {
  uint32_t dead = 0;
  uint32_t no_op = 0;
  int32_t key = 0;


  key = getch();
  flushinp();
  if (CTRL_N) {
    process_pc_move_attempt(pc, dir_n, region_ptr[*region_x][*region_y]);
  } else if (CTRL_NE) {
    process_pc_move_attempt(pc, dir_ne, region_ptr[*region_x][*region_y]);
  } else if (CTRL_E) {
    process_pc_move_attempt(pc, dir_e, region_ptr[*region_x][*region_y]);
  } else if (CTRL_SE) {
    process_pc_move_attempt(pc, dir_se, region_ptr[*region_x][*region_y]);
  } else if (CTRL_S) {
    process_pc_move_attempt(pc, dir_s, region_ptr[*region_x][*region_y]);
  } else if (CTRL_SW) {
    process_pc_move_attempt(pc, dir_sw, region_ptr[*region_x][*region_y]);
  } else if (CTRL_W) {
    process_pc_move_attempt(pc, dir_w, region_ptr[*region_x][*region_y]);
  } else if (CTRL_NW) {
    process_pc_move_attempt(pc, dir_nw, region_ptr[*region_x][*region_y]);
  } else if (CTRL_PASS) {
    // do nothing
  } else if (CTRL_ENTER) {
    
  } else if (CTRL_EXIT) {
    
  } else if (CTRL_TNR_LIST_SHOW) {
    
  } else if (CTRL_TNR_LIST_HIDE) {
      
  } else if (CTRL_SCROLL_UP) {
    
  } else if (CTRL_SCROLL_DOWN) {

  } else if (CTRL_SCROLL_LEFT) {
    
  } else if (CTRL_SCROLL_RIGHT) {
    
  } else if (CTRL_QUIT_GAME) {
    *quit = 1;
  }
    

  
  return;
}


/*
    case 'n':
      if ((*region_y) < WORLD_SIZE - 1) {
        ++(*region_y);
      } else {
        printf("Illegal input, out of bounds.\n");
      }
      break;
    case 'e':
      if ((*region_x) < WORLD_SIZE - 1) {
        ++(*region_x);
      } else {
        printf("Illegal input, out of bounds.\n");
      }
      break;
    case 's':
      if ((*region_y) > 0) {
        --(*region_y);
      } else {
        printf("Illegal input, out of bounds.\n");
      }
      break;
    case 'w':
      if ((*region_x) > 0) {
        --(*region_x);
      } else {
        printf("Illegal input, out of bounds.\n");
      }
      break;
*/

void init_pc (character_t *pc, region_t *region) {
  pc->movetime = 0;
  pc->tnr = tnr_pc;
  pc->defeated = 0;

  int32_t found_location = 0;
  while (found_location != 1) {
    pc->pos_i = (rand() % (MAX_ROW - 2)) + 1;
    pc->pos_j = (rand() % (MAX_COL - 2)) + 1;
    if (region->tile_arr[pc->pos_i][pc->pos_j].ter == ter_path) {
      found_location = 1;
      for (int32_t i = 0; i < region->num_npc; i++) {
        if (region->npc_arr[i].pos_i == pc->pos_i && 
            region->npc_arr[i].pos_j == pc->pos_j) {
          found_location = 0;
          break;
        }
      }
    }
  }
}

void load_region(int32_t region_x, int32_t region_y, int32_t num_tnr) {
  // If the region we are in is uninitialized, then generate the region.
  if (region_ptr[region_x][region_y] == NULL) {
    region_t *new_region = malloc(sizeof(*new_region));
    region_ptr[region_x][region_y] = new_region;
    int32_t N_exit, E_exit, S_exit, W_exit;
    N_exit = -1;
    E_exit = -1;
    S_exit = -1;
    W_exit = -1;

    // determine if new region should generate with poke center and/or mart
    // (-45d/200 + 50) / 100 => -0.45*d/200 + 0.50
    int32_t d = m_dist(region_x, region_y, WORLD_SIZE/2, WORLD_SIZE/2);
    double p = (-0.45*d)/200 + 0.50;
    int32_t place_center = rand_outcome(p);
    int32_t place_mart = rand_outcome(p);

    if (region_y + 1 < WORLD_SIZE) {
      if (region_ptr[region_x][region_y + 1] != NULL) {
        N_exit = region_ptr[region_x][region_y + 1]->S_exit_j; /* North Region, South Exit */
      }
    }
    if (region_x + 1 < WORLD_SIZE) {
      if (region_ptr[region_x + 1][region_y] != NULL) {
        E_exit = region_ptr[region_x + 1][region_y]->W_exit_i; /* East Region, West Exit */
      }
    }
    if (region_y - 1 >= 0) {
      if (region_ptr[region_x][region_y - 1] != NULL) {
        S_exit = region_ptr[region_x][region_y - 1]->N_exit_j; /* South Region, North Exit */
      }
    }
    if (region_x - 1 >= 0) {
      if (region_ptr[region_x - 1][region_y] != NULL) {
        W_exit = region_ptr[region_x - 1][region_y]->E_exit_i; /* West Region, East Exit */
      }
    }
    init_region(region_ptr[region_x][region_y], N_exit, E_exit, S_exit, W_exit, place_center, place_mart, num_tnr);
  
    // If on the edge of the world, block exits with boulders so that player cannot
    // fall out of the world
    if (region_y == WORLD_SIZE - 1) {
      N_exit = region_ptr[region_x][region_y]->N_exit_j;
      region_ptr[region_x][region_y]->tile_arr[0][N_exit].ter = ter_border;
    }
    if (region_x == WORLD_SIZE - 1) {
      E_exit = region_ptr[region_x][region_y]->E_exit_i;
      region_ptr[region_x][region_y]->tile_arr[E_exit][MAX_COL-1].ter = ter_border;
    }
    if (region_y == 0) {
      S_exit = region_ptr[region_x][region_y]->S_exit_j;
      region_ptr[region_x][region_y]->tile_arr[MAX_ROW-1][S_exit].ter = ter_border;
    }
    if (region_x == 0) {
      W_exit = region_ptr[region_x][region_y]->W_exit_i;
      region_ptr[region_x][region_y]->tile_arr[W_exit][0].ter = ter_border;
    }
  }
}

/*
 * Free all memomry allocated to regions
 */
void free_all_regions() {
  for (int32_t i = 0; i < WORLD_SIZE; i++) {
    for (int32_t j = 0; j < WORLD_SIZE; j++) {
      if (region_ptr[i][j] != NULL) {
        free(region_ptr[i][j]->npc_arr);
        free(region_ptr[i][j]);
      }
    }
  }
}

/*
 * integer bit pattern using a color byte
 */
int colornum(int fg, int bg)
{
    int B, bbb, ffff;

    B = 1 << 7;
    bbb = (7 & bg) << 4;
    ffff = 7 & fg;

    return (B | bbb | ffff);
}

/*
 * Initialize terminal with ncurses
 */
void init_terminal() {
  initscr();
  raw();
  noecho();

  curs_set(0);
  keypad(stdscr, TRUE);

  start_color();
  init_pair(COLOR_RED,     COLOR_RED,     CHAR_COLOR_BACKGROUND);
  init_pair(COLOR_GREEN,   COLOR_GREEN,   CHAR_COLOR_BACKGROUND);
  init_pair(COLOR_YELLOW,  COLOR_YELLOW,  CHAR_COLOR_BACKGROUND);
  init_pair(COLOR_BLUE,    COLOR_BLUE,    CHAR_COLOR_BACKGROUND);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, CHAR_COLOR_BACKGROUND);
  init_pair(COLOR_CYAN,    COLOR_CYAN,    CHAR_COLOR_BACKGROUND);
  init_pair(COLOR_WHITE,   COLOR_WHITE,   CHAR_COLOR_BACKGROUND);
}

/*
 * Renders a region to the screen
 */
void render_region(region_t *region, character_t *pc) { 
  clear(); 
  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      terrain_t ter = region->tile_arr[i][j].ter;
      switch (ter) {
        case ter_border:
          attron(COLOR_PAIR(CHAR_COLOR_BORDER));
          mvaddch(i + 1, j, CHAR_BORDER);
          attroff(COLOR_PAIR(CHAR_COLOR_BORDER));
          break;
        case ter_clearing:
          attron(COLOR_PAIR(CHAR_COLOR_CLEARING));
          mvaddch(i + 1, j, CHAR_CLEARING);
          attroff(COLOR_PAIR(CHAR_COLOR_CLEARING));
          break;
        case ter_grass:
          attron(COLOR_PAIR(CHAR_COLOR_GRASS));
          mvaddch(i + 1, j, CHAR_GRASS);
          attroff(COLOR_PAIR(CHAR_COLOR_GRASS));
          break;
        case ter_boulder:
          attron(COLOR_PAIR(CHAR_COLOR_BOULDER));
          mvaddch(i + 1, j, CHAR_BOULDER);
          attroff(COLOR_PAIR(CHAR_COLOR_BOULDER));
          break;
        case ter_tree:
          attron(COLOR_PAIR(CHAR_COLOR_TREE));
          mvaddch(i + 1, j, CHAR_TREE);
          attroff(COLOR_PAIR(CHAR_COLOR_TREE));
          break;
        case ter_mountain:
          attron(COLOR_PAIR(CHAR_COLOR_MOUNTAIN));
          mvaddch(i + 1, j, CHAR_MOUNTAIN);
          attroff(COLOR_PAIR(CHAR_COLOR_MOUNTAIN));
          break;
        case ter_forest:
          attron(COLOR_PAIR(CHAR_COLOR_FOREST));
          mvaddch(i + 1, j, CHAR_FOREST);
          attroff(COLOR_PAIR(CHAR_COLOR_FOREST));
          break;
        case ter_path:
          attron(COLOR_PAIR(CHAR_COLOR_PATH));
          mvaddch(i + 1, j, CHAR_PATH);
          attroff(COLOR_PAIR(CHAR_COLOR_PATH));
          break;
        case ter_center:
          attron(A_BOLD);
          attron(COLOR_PAIR(CHAR_COLOR_CENTER));
          mvaddch(i + 1, j, CHAR_CENTER);
          attroff(A_BOLD);
          attroff(COLOR_PAIR(CHAR_COLOR_CENTER));
          break;
        case ter_mart:
          attron(A_BOLD);
          attron(COLOR_PAIR(CHAR_COLOR_MART));
          mvaddch(i + 1, j, CHAR_MART);
          attroff(A_BOLD);
          attroff(COLOR_PAIR(CHAR_COLOR_MART));
          break;
        default:
          attron(COLOR_PAIR(CHAR_COLOR_UNDEFINED));
          mvaddch(i + 1, j, CHAR_UNDEFINED);
          attroff(COLOR_PAIR(CHAR_COLOR_UNDEFINED));
      }
    }
  }

  character_t *p = (region->npc_arr);
  for (int32_t k = 0; k < region->num_npc; k++, p++) {
    switch (p->tnr) {
      case tnr_hiker:
        attron(COLOR_PAIR(CHAR_COLOR_HIKER));
        mvaddch(p->pos_i + 1, p->pos_j, CHAR_HIKER);
        attroff(COLOR_PAIR(CHAR_COLOR_HIKER));
        break;
      case tnr_rival:
        attron(COLOR_PAIR(CHAR_COLOR_RIVAL));
        mvaddch(p->pos_i + 1, p->pos_j, CHAR_RIVAL);
        attroff(COLOR_PAIR(CHAR_COLOR_RIVAL));
        break;
      case tnr_pacer:
        attron(COLOR_PAIR(CHAR_COLOR_PACER));
        mvaddch(p->pos_i + 1, p->pos_j, CHAR_PACER);
        attroff(COLOR_PAIR(CHAR_COLOR_PACER));
        break;
      case tnr_wanderer:
        attron(COLOR_PAIR(CHAR_COLOR_WANDERER));
        mvaddch(p->pos_i + 1, p->pos_j, CHAR_WANDERER);
        attroff(COLOR_PAIR(CHAR_COLOR_WANDERER));
        break;
      case tnr_stationary:
        attron(COLOR_PAIR(CHAR_COLOR_STATIONARY));
        mvaddch(p->pos_i + 1, p->pos_j, CHAR_STATIONARY);
        attroff(COLOR_PAIR(CHAR_COLOR_STATIONARY));
        break;
      case tnr_rand_walker:
        attron(COLOR_PAIR(CHAR_COLOR_RAND_WALKER));
        mvaddch(p->pos_i + 1, p->pos_j, CHAR_RAND_WALKER);
        attroff(COLOR_PAIR(CHAR_COLOR_RAND_WALKER));
        break;
      default:
        attron(COLOR_PAIR(CHAR_COLOR_UNDEFINED));
        mvaddch(p->pos_i + 1, p->pos_j, CHAR_UNDEFINED);
        attroff(COLOR_PAIR(CHAR_COLOR_UNDEFINED));
    }
  }
  
  attron(COLOR_PAIR(CHAR_COLOR_PC));
  mvaddch(pc->pos_i + 1, pc->pos_j, CHAR_PC);
  attroff(COLOR_PAIR(CHAR_COLOR_PC));

  refresh();
}