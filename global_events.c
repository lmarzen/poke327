#include <limits.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "region.h"
#include "trainer_events.h"
#include "global_events.h"

extern region_t *region_ptr[WORLD_SIZE][WORLD_SIZE];

/*
 * Initializes the player character on a path
 */
void init_pc (character_t *pc, region_t *region) {
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
  pc->movetime = travel_times[region->tile_arr[pc->pos_i][pc->pos_j].ter][pc->tnr];
}

/*
 * This procedure updates the players position and performs movement animation.
 * Used after moving to another region.
 */
void pc_next_region(character_t *pc, int32_t to_rx,    int32_t to_ry, 
                                     int32_t from_rx,  int32_t from_ry) {
  // player gets first move in a new region
  pc->movetime = 0;

  if (from_ry - to_ry > 0) {
    // coming from the north
    pc->pos_i = 0;
    render_region(region_ptr[to_rx][to_ry], pc);
    usleep(FRAMETIME);
    pc->pos_i = 1;
    render_region(region_ptr[to_rx][to_ry], pc);
    usleep(FRAMETIME);
    return;
  } else if (from_ry - to_ry < 0) {
    // coming from the south
    pc->pos_i = MAX_ROW - 1;
    render_region(region_ptr[to_rx][to_ry], pc);
    usleep(FRAMETIME);
    pc->pos_i = MAX_ROW - 2;
    render_region(region_ptr[to_rx][to_ry], pc);
    usleep(FRAMETIME);
    return;
  }

  if (from_rx - to_rx > 0) {
    // coming from the east
    pc->pos_j = MAX_COL - 1;
    render_region(region_ptr[to_rx][to_ry], pc);
    usleep(FRAMETIME);
    pc->pos_j = MAX_COL - 2;
    render_region(region_ptr[to_rx][to_ry], pc);
    usleep(FRAMETIME);
    return;
  } else if (from_rx - to_rx < 0) {
    // coming from the west
    pc->pos_j = 0;
    render_region(region_ptr[to_rx][to_ry], pc);
    usleep(FRAMETIME);
    pc->pos_j = 1;
    render_region(region_ptr[to_rx][to_ry], pc);
    usleep(FRAMETIME);
    return;
  }

  // If we get here we did not move regions!
  return;
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
  return;
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
  int32_t color;
  char ch;

  clear(); 
  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      terrain_t ter = region->tile_arr[i][j].ter;
      switch (ter) {
        case ter_border:
          ch =  CHAR_BORDER;
          color = CHAR_COLOR_BORDER;
          break;
        case ter_clearing:
          ch =  CHAR_CLEARING;
          color = CHAR_COLOR_CLEARING;
          break;
        case ter_grass:
          ch =  CHAR_GRASS;
          color = CHAR_COLOR_GRASS;
          break;
        case ter_boulder:
          ch =  CHAR_BOULDER;
          color = CHAR_COLOR_BOULDER;
          break;
        case ter_tree:
          ch =  CHAR_TREE;
          color = CHAR_COLOR_TREE;
          break;
        case ter_mountain:
          ch =  CHAR_MOUNTAIN;
          color = CHAR_COLOR_MOUNTAIN;
          break;
        case ter_forest:
          ch =  CHAR_FOREST;
          color = CHAR_COLOR_FOREST;
          break;
        case ter_path:
          ch =  CHAR_PATH;
          color = CHAR_COLOR_PATH;
          break;
        case ter_center:
          ch =  CHAR_CENTER;
          color = CHAR_COLOR_CENTER;
          break;
        case ter_mart:
          ch =  CHAR_MART;
          color = CHAR_COLOR_MART;
          break;
        default:
          ch =  CHAR_UNDEFINED;
          color = CHAR_COLOR_UNDEFINED;
      }
    attron(COLOR_PAIR(color));
    mvaddch(i + 1, j, ch);
    attroff(COLOR_PAIR(color));
    }
  }

  character_t *p = (region->npc_arr);
  for (int32_t k = 0; k < region->num_npc; k++, p++) {
    switch (p->tnr) {
      case tnr_hiker:
        ch = CHAR_HIKER;
        color = CHAR_COLOR_HIKER;
        break;
      case tnr_rival:
        ch = CHAR_RIVAL;
        color = CHAR_COLOR_RIVAL;
        break;
      case tnr_pacer:
        ch = CHAR_PACER;
        color = CHAR_COLOR_PACER;
        break;
      case tnr_wanderer:
        ch = CHAR_WANDERER;
        color = CHAR_COLOR_WANDERER;
        break;
      case tnr_stationary:
        ch = CHAR_STATIONARY;
        color = CHAR_COLOR_STATIONARY;
        break;
      case tnr_rand_walker:
        ch = CHAR_RAND_WALKER;
        color = CHAR_COLOR_RAND_WALKER;
        break;
      default:
        ch = CHAR_UNDEFINED;
        color = CHAR_COLOR_UNDEFINED;
    }
    attron(COLOR_PAIR(color));
    mvaddch(p->pos_i + 1, p->pos_j, ch);
    attroff(COLOR_PAIR(color));
  }
  
  attron(A_BOLD);
  attron(COLOR_PAIR(CHAR_COLOR_PC));
  mvaddch(pc->pos_i + 1, pc->pos_j, CHAR_PC);
  attroff(COLOR_PAIR(CHAR_COLOR_PC));
  attroff(A_BOLD);

  refresh();
}

/*
 * Renders a battle to the screen
 */
void render_battle(battle_t *battle) { 
  clear(); 
  mvprintw(0,0,"***Battle placeholder***");
  mvprintw(2,0,"Press ESC to exit battle");
  refresh();
  return;
}

/*
 * Renders a poke center to the screen
 */
void render_center() { 
  clear(); 
  mvprintw(0,0,"***Poke Center placeholder***");
  mvprintw(2,0,"Press < to exit the Poke Center");
  refresh();
  return;
}

/*
 * Renders a poke mart to the screen
 */
void render_mart() { 
  clear(); 
  mvprintw(0,0,"***Poke Mart placeholder***");
  mvprintw(2,0,"Press < to exit the Poke Mart");
  refresh();
  return;
}

/*
 * Renders the trainer overlay to the screen
 */
void render_tnr_overlay(character_t *pc, region_t *region, int32_t scroller_pos) { 
  int32_t i;
  int32_t j = scroller_pos;
  int32_t color;
  char ch;
  int32_t rel_i, rel_j;
  char dir_ns, dir_ew;

  clear(); 
  attron(A_BOLD);
  mvprintw(0,0,"Nearby Trainers");
  attroff(A_BOLD);

  for (i = 0; (i < region->num_npc) && (i < MAX_ROW); i++) {
    mvaddch(i + 1,0, ACS_VLINE);

    character_t t = region->npc_arr[scroller_pos + i];
    switch (t.tnr) {
      case tnr_hiker:
        ch = CHAR_HIKER;
        color = CHAR_COLOR_HIKER;
        break;
      case tnr_rival:
        ch = CHAR_RIVAL;
        color = CHAR_COLOR_RIVAL;
        break;
      case tnr_pacer:
        ch = CHAR_PACER;
        color = CHAR_COLOR_PACER;
        break;
      case tnr_wanderer:
        ch = CHAR_WANDERER;
        color = CHAR_COLOR_WANDERER;
        break;
      case tnr_stationary:
        ch = CHAR_STATIONARY;
        color = CHAR_COLOR_STATIONARY;
        break;
      case tnr_rand_walker:
        ch = CHAR_RAND_WALKER;
        color = CHAR_COLOR_RAND_WALKER;
        break;
      default:
        ch = CHAR_UNDEFINED;
        color = CHAR_COLOR_UNDEFINED;
    }
    attron(COLOR_PAIR(color));
    mvaddch(i + 1, 2, ch);
    attroff(COLOR_PAIR(color));

    rel_i = t.pos_i - pc->pos_i;
    rel_j = t.pos_j - pc->pos_j;
    if (rel_i < 0) {
      dir_ns = 'n';
      rel_i = abs(rel_i);
    } else {
      dir_ns = 's';
    }
    if (rel_j < 0) {
      dir_ew = 'w';
      rel_j = abs(rel_j);
    } else {
      dir_ew = 'e';
    }
    mvprintw(i + 1,4,"(%2d%c, %2d%c)", rel_i, dir_ns, rel_j, dir_ew);
    
    if (t.defeated) {
      mvprintw(i + 1,15,"(defeated)");
    }
    
  }
  
  mvprintw(i + 2,0,"Press ESC to close overlay");
  refresh();
  return;
}

/*
 * Process user input while in a battle
 */
void process_input_battle(battle_t *battle, int32_t *quit_game) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_LEAVE_BATTLE) {
      battle->end_battle = 1;
      no_op = 0;
    } else if (CTRL_QUIT_GAME) {
      *quit_game = 1;
      no_op = 0;
    }
  }
  return;
}

/*
 * Process user input while in a center
 */
void process_input_center(character_t *pc, int32_t *exit_center, int32_t *quit_game) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_EXIT_BLDG) {
      *exit_center = 1;
      no_op = 0;
    } else if (CTRL_QUIT_GAME) {
      *quit_game = 1;
      no_op = 0;
    }
  }
  return;
}

/*
 * Process user input while in a mart
 */
void process_input_mart(character_t *pc, int32_t *exit_mart, int32_t *quit_game) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_EXIT_BLDG) {
      *exit_mart = 1;
      no_op = 0;
    } else if (CTRL_QUIT_GAME) {
      *quit_game = 1;
      no_op = 0;
    }
  }
  return;
}

/*
 * Process user input while in the trainer overlay
 */
void process_input_tnr_overlay(region_t *region, int32_t *scroller_pos, int32_t *close_overlay, int32_t *quit_game) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_TNR_LIST_HIDE) {
      *close_overlay = 1;
      no_op = 0;
    } else if (CTRL_SCROLL_DOWN) {
      if (*scroller_pos < (region->num_npc - MAX_ROW)) {
        ++(*scroller_pos);
        no_op = 0;
      }
    } else if (CTRL_SCROLL_UP) {
      if (*scroller_pos > 0) {
        --(*scroller_pos);
        no_op = 0;
      }
    } else if (CTRL_QUIT_GAME) {
      *quit_game = 1;
      no_op = 0;
    }
  }
  return;
}

/*
 * Drives interactions in a poke center
 */
void center_driver(character_t *pc, int32_t *quit_game) {
  int32_t exit_center = 0;

  while (!exit_center && !(*quit_game)) {
    render_center();
    process_input_center(pc, &exit_center, quit_game);
  }
}

/*
 * Drives interactions in a poke mart
 */
void mart_driver(character_t *pc, int32_t *quit_game) {
  int32_t exit_mart = 0;

  while (!exit_mart && !(*quit_game)) {
    render_mart();
    process_input_mart(pc, &exit_mart, quit_game);
  }

  return;
}

/*
 * Drives trainer list overlay
 */
void tnr_overlay_driver(character_t *pc, region_t *region, int32_t *quit_game) {
  int32_t close_overlay = 0;
  int32_t scroller_pos = 0;

  while (!close_overlay && !(*quit_game)) {
    render_tnr_overlay(pc, region, scroller_pos);
    process_input_tnr_overlay(region, &scroller_pos, &close_overlay, quit_game);
  }
  return;
}

/*
 * Process user input while navigating a region.
 * Update which region is being displayed, generate new regions as needed.
 */
void process_input_nav (character_t *pc, int32_t *region_x, int32_t *region_y, int32_t *quit_game) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_N) {
      no_op = process_pc_move_attempt(pc, dir_n, region_x, region_y, quit_game);
    } else if (CTRL_NE) {
      no_op = process_pc_move_attempt(pc, dir_ne, region_x, region_y, quit_game);
    } else if (CTRL_E) {
      no_op = process_pc_move_attempt(pc, dir_e, region_x, region_y, quit_game);
    } else if (CTRL_SE) {
      no_op = process_pc_move_attempt(pc, dir_se, region_x, region_y, quit_game);
    } else if (CTRL_S) {
      no_op = process_pc_move_attempt(pc, dir_s, region_x, region_y, quit_game);
    } else if (CTRL_SW) {
      no_op = process_pc_move_attempt(pc, dir_sw, region_x, region_y, quit_game);
    } else if (CTRL_W) {
      no_op = process_pc_move_attempt(pc, dir_w, region_x, region_y, quit_game);
    } else if (CTRL_NW) {
      no_op = process_pc_move_attempt(pc, dir_nw, region_x, region_y, quit_game);
    } else if (CTRL_PASS) {
      // do nothing
      no_op = 0;
    } else if (CTRL_ENTER_BLDG) {
      if (region_ptr[*region_x][*region_y]->tile_arr[pc->pos_i][pc->pos_j].ter == ter_center) {
        center_driver(pc, quit_game);
        render_region(region_ptr[*region_x][*region_y], pc);
        no_op = 0;
      } else if (region_ptr[*region_x][*region_y]->tile_arr[pc->pos_i][pc->pos_j].ter == ter_mart) {
        mart_driver(pc, quit_game);
        render_region(region_ptr[*region_x][*region_y], pc);
        no_op = 0;
      }
    } else if (CTRL_TNR_LIST_SHOW) {
      tnr_overlay_driver(pc, region_ptr[*region_x][*region_y], quit_game);
      render_region(region_ptr[*region_x][*region_y], pc);
      if (*quit_game == 1) {
        no_op = 0;
      }
    } else if (CTRL_QUIT_GAME) {
      *quit_game = 1;
      no_op = 0;
    }
  }
    
  return;
}