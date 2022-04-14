#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <climits>
#include <ncurses.h>
#include <vector>
#include <unistd.h>

#include "config.h"
#include "region.h"
#include "trainer_events.h"
#include "global_events.h"
#include "item.h"

extern Region *region_ptr[WORLD_SIZE][WORLD_SIZE];
extern Pc *pc;

/*
 * This procedure updates the players position and performs movement animation.
 * Used after moving to another region.
 * 
 * Friend of Pc (Player Character) class
 */
void pc_next_region(int32_t to_rx,   int32_t to_ry, 
                    int32_t from_rx, int32_t from_ry) {
  Region *r = region_ptr[to_rx][to_ry];

  // player gets first move in a new region
  pc->movetime = 0;

  if (from_ry - to_ry > 0) {
    // coming from the north
    pc->pos_i = 0;
    render_region(r);
    usleep(FRAMETIME);
    pc->pos_i = 1;
    render_region(r);
    usleep(FRAMETIME);
    return;
  } else if (from_ry - to_ry < 0) {
    // coming from the south
    pc->pos_i = MAX_ROW - 1;
    render_region(r);
    usleep(FRAMETIME);
    pc->pos_i = MAX_ROW - 2;
    render_region(r);
    usleep(FRAMETIME);
    return;
  }

  if (from_rx - to_rx > 0) {
    // coming from the east
    pc->pos_j = MAX_COL - 1;
    render_region(r);
    usleep(FRAMETIME);
    pc->pos_j = MAX_COL - 2;
    render_region(r);
    usleep(FRAMETIME);
    return;
  } else if (from_rx - to_rx < 0) {
    // coming from the west
    pc->pos_j = 0;
    render_region(r);
    usleep(FRAMETIME);
    pc->pos_j = 1;
    render_region(r);
    usleep(FRAMETIME);
  return;
  }

  // If we get here we did not move regions!
  return;
}

void load_region(int32_t region_x, int32_t region_y, int32_t num_tnr) {
  // If the region we are in is uninitialized, then generate the region.
  if (region_ptr[region_x][region_y] == NULL) {
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
        N_exit = region_ptr[region_x][region_y + 1]->get_S_exit_j(); /* North Region, South Exit */
      }
    }
    if (region_x + 1 < WORLD_SIZE) {
      if (region_ptr[region_x + 1][region_y] != NULL) {
        E_exit = region_ptr[region_x + 1][region_y]->get_W_exit_i(); /* East Region, West Exit */
      }
    }
    if (region_y - 1 >= 0) {
      if (region_ptr[region_x][region_y - 1] != NULL) {
        S_exit = region_ptr[region_x][region_y - 1]->get_N_exit_j(); /* South Region, North Exit */
      }
    }
    if (region_x - 1 >= 0) {
      if (region_ptr[region_x - 1][region_y] != NULL) {
        W_exit = region_ptr[region_x - 1][region_y]->get_E_exit_i(); /* West Region, East Exit */
      }
    }

    Region *new_region = new Region(N_exit, E_exit, S_exit, W_exit,
                                    place_center, place_mart);
    new_region->populate(num_tnr);
    region_ptr[region_x][region_y] = new_region;

    // If on the edge of the world, block exits with boulders so that player cannot
    // fall out of the world
    if (region_y == WORLD_SIZE - 1) {
      new_region->close_N_exit();
    }
    if (region_x == WORLD_SIZE - 1) {
      new_region->close_E_exit();
    }
    if (region_y == 0) {
      new_region->close_S_exit();
    }
    if (region_x == 0) {
      new_region->close_W_exit();
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
        delete region_ptr[i][j];
      }
    }
  }
  return;
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
void render_region(Region *r) { 
  clear(); 

  // add terrain to frame buffer
  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      attron(COLOR_PAIR(r->get_color(i, j)));
      mvaddch(i + 1, j, r->get_ch(i, j));
      attroff(COLOR_PAIR(r->get_color(i, j)));
    }
  }

  // add npcs to frame buffer
  for (auto it = r->get_npcs()->begin(); it != r->get_npcs()->end(); ++it) {
    attron(COLOR_PAIR(it->get_color()));
    mvaddch(it->get_i() + 1, it->get_j(), it->get_ch());
    attroff(COLOR_PAIR(it->get_color()));
  }
  
  // add player to frame buffer
  attron(A_BOLD);
  attron(COLOR_PAIR(pc->get_color()));
  mvaddch(pc->get_i() + 1, pc->get_j(), pc->get_ch());
  attroff(COLOR_PAIR(pc->get_color()));
  attroff(A_BOLD);

  refresh();
}

/*
 * Renders an encounter to the screen
 */
void render_battle(Pokemon *p_pc, Pokemon *p_opp,
                   const char* message, bool show_menu,
                   int32_t scroller_pos, bool selected_fight) { 
  int32_t color;
  clear();

  // OPPONENT POKEMON
  attron(A_BOLD);
  mvprintw(2, 2,"%s ", p_opp->get_nickname());
  attroff(A_BOLD);
  if (p_opp->get_gender() == gender_male) {
    attron(COLOR_PAIR(COLOR_BLUE));
    addch('m');
    attroff(COLOR_PAIR(COLOR_BLUE));
  } else {
    attron(COLOR_PAIR(COLOR_MAGENTA));
    addch('f');
    attroff(COLOR_PAIR(COLOR_MAGENTA));
  }
  if (p_opp->is_shiny()) {
    attron(A_BOLD);
    addch('*');
    attroff(A_BOLD);
  }                
  mvprintw(2, 20 - (p_opp->get_level()/10), "Lv.%d", p_opp->get_level());
  mvprintw(3, 2, "HP");
  if (p_opp->get_current_hp() < p_opp->get_stat(stat_hp) / 5) {
    color = CHAR_COLOR_HEALTH_LOW;
  } else if (p_opp->get_current_hp() < p_opp->get_stat(stat_hp) / 2) {
    color = CHAR_COLOR_HEALTH_MED;
  } else {
    color = CHAR_COLOR_HEALTH_HIGH;
  }
  attron(COLOR_PAIR(color));
  for (int32_t h = 20 * p_opp->get_current_hp(); h > 0; 
       h -= p_opp->get_stat(stat_hp)) {
    addch(CHAR_HEALTH);
  }
  attroff(COLOR_PAIR(color));

  // PLAYER POKEMON
  attron(A_BOLD);
  mvprintw(7, 2,"%s ", p_pc->get_nickname());
  attroff(A_BOLD);
  if (p_opp->get_gender() == gender_male) {
    attron(COLOR_PAIR(COLOR_BLUE));
    addch('m');
    attroff(COLOR_PAIR(COLOR_BLUE));
  } else {
    attron(COLOR_PAIR(COLOR_MAGENTA));
    addch('f');
    attroff(COLOR_PAIR(COLOR_MAGENTA));
  }
  if (p_opp->is_shiny()) {
    attron(A_BOLD);
    addch('*');
    attroff(A_BOLD);
  }          
  mvprintw(7, 20 - (p_pc->get_level()/10), "Lv.%d", p_pc->get_level());
  mvprintw(8, 2, "HP");
  if (p_pc->get_current_hp() < p_pc->get_stat(stat_hp) / 5) {
    color = CHAR_COLOR_HEALTH_LOW;
  } else if (p_pc->get_current_hp() < p_pc->get_stat(stat_hp) / 2) {
    color = CHAR_COLOR_HEALTH_MED;
  } else {
    color = CHAR_COLOR_HEALTH_HIGH;
  }
  attron(COLOR_PAIR(color));
  for (int32_t h = 20 * p_pc->get_current_hp(); h > 0; 
       h -= p_pc->get_stat(stat_hp)) {
    addch(CHAR_HEALTH);
  }
  attroff(COLOR_PAIR(color));
  mvprintw(9, 19,"%d/%d", p_pc->get_current_hp(), p_pc->get_stat(stat_hp));
  mvprintw(10, 2, "EXP");
  attron(COLOR_PAIR(CHAR_COLOR_EXP));
  for (int32_t e = 19 * 0 /*TODO: p_pc->get_level_up_exp()*/; e > 0; 
       e -= p_pc->get_exp()) {
    addch(CHAR_EXP);
  }
  attroff(COLOR_PAIR(CHAR_COLOR_EXP));

  mvprintw(12, 0, message);
  
  if (show_menu) {
    mvaddch(13, 0, (0 == scroller_pos ? '>' : ACS_VLINE));
    if (!selected_fight) {
      printw(" Fight");
    } else if (p_pc->get_num_moves() > 0) {
      printw(" %s", p_pc->get_move(0)->identifier);
    }
    mvaddch(14, 0, (1 == scroller_pos ? '>' : ACS_VLINE));
    if (!selected_fight) {
      printw(" Bag");
    } else if (p_pc->get_num_moves() > 1) {
      printw(" %s", p_pc->get_move(1)->identifier);
    }
    mvaddch(15, 0, (2 == scroller_pos ? '>' : ACS_VLINE));
      if (!selected_fight) {
      printw(" Pokemon");
    } else if (p_pc->get_num_moves() > 2) {
      printw(" %s", p_pc->get_move(2)->identifier);
    }
    mvaddch(16, 0, (3 == scroller_pos ? '>' : ACS_VLINE));
    if (!selected_fight) {
      printw(" Run");
    } else if (p_pc->get_num_moves() > 3) {
      printw(" %s", p_pc->get_move(3)->identifier);
    }

    if (selected_fight && p_pc->get_num_moves() > 0) {
      mvprintw(17, 0, "PP  %d/%d", p_pc->get_current_pp(scroller_pos), 
                                   p_pc->get_move(scroller_pos)->pp);
      mvprintw(18, 0, "type/%s", 
               pd_type_names[p_pc->get_move(scroller_pos)->type_id]);
    }

  }

  // mvprintw(6, 2, "Stats");
  // mvprintw(7, 2, "HP:      %d", p_opp->get_stat(stat_hp));
  // mvprintw(8, 2, "ATTACK:  %d", p_opp->get_stat(stat_attack));
  // mvprintw(9, 2, "DEFENSE: %d", p_opp->get_stat(stat_defense));
  // mvprintw(10,2, "SP. ATK: %d", p_opp->get_stat(stat_sp_atk));
  // mvprintw(11,2, "SP. DEF: %d", p_opp->get_stat(stat_sp_def));
  // mvprintw(12,2, "SPEED:   %d", p_opp->get_stat(stat_speed));
  // mvprintw(6, 17, "IVs");
  // mvprintw(7, 17, "%d", p_opp->get_iv(stat_hp));
  // mvprintw(8, 17, "%d", p_opp->get_iv(stat_attack));
  // mvprintw(9, 17, "%d", p_opp->get_iv(stat_defense));
  // mvprintw(10,17, "%d", p_opp->get_iv(stat_sp_atk));
  // mvprintw(11,17, "%d", p_opp->get_iv(stat_sp_def));
  // mvprintw(12,17, "%d", p_opp->get_iv(stat_speed));

  // mvprintw(14,2, "Moves");
  // pd_move_t *moveslot_0 = p_opp->get_move(0);
  // pd_move_t *moveslot_1 = p_opp->get_move(1);
  // pd_move_t *moveslot_2 = p_opp->get_move(2);
  // pd_move_t *moveslot_3 = p_opp->get_move(3);
  // if (moveslot_0 != 0)
  //   mvprintw(15,2, "%s", moveslot_0->identifier);
  // if (moveslot_1 != 0)
  //   mvprintw(16,2, "%s", moveslot_1->identifier);
  // if (moveslot_2 != 0)
  //   mvprintw(17,2, "%s", moveslot_2->identifier);
  // if (moveslot_3 != 0)
  //   mvprintw(18,2, "%s", moveslot_3->identifier);
  // mvprintw(20,0,"Press ESC to exit encounter");
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
void render_tnr_overlay(int32_t scroller_pos) { 
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];
  
  int32_t i;
  int32_t rel_i, rel_j;
  char dir_ns, dir_ew;

  clear(); 
  attron(A_BOLD);
  mvprintw(0,0,"Nearby Trainers");
  attroff(A_BOLD);

  for (i = 0; 
       (i < static_cast<int32_t>(r->get_npcs()->size())) && (i < MAX_ROW); 
       i++) {
    Character *c = &r->get_npcs()->at(scroller_pos + i);

    mvaddch(i + 1,0, ACS_VLINE);
    attron(COLOR_PAIR(c->get_color()));
    mvaddch(i + 1, 2, c->get_ch());
    attroff(COLOR_PAIR(c->get_color()));

    rel_i = c->get_i() - pc->get_i();
    rel_j = c->get_j() - pc->get_j();
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
    
    if (c->is_defeated()) {
      mvprintw(i + 1,15,"(defeated)");
    }
    
  }
  
  mvprintw(i + 2,0,"Press ESC to close trainer overlay");
  refresh();
  return;
}

/*
 * Renders the players bag to the screen
 */
void render_bag(int32_t page_index, int32_t scroller_pos) { 
  int32_t i;
  bag_slot_t s;

  clear(); 
  attron(A_BOLD);
  mvprintw(0,0,"Bag");
  attroff(A_BOLD);

  for (i = 0; (i < pc->num_bag_slots() && i < MAX_ROW); ++i) {
    s = pc->peek_bag_slot(i + page_index);
    if (i == scroller_pos - page_index) {
      mvaddch(i + 1,0, '>');
    } else {
      mvaddch(i + 1,0, ACS_VLINE);
    }
    
    mvprintw(i + 1, 2, "%3dx %s", s.cnt, item_name_txt[s.item]);
  }

  mvprintw(i + 2,0,"Press ESC to close bag");
  refresh();
  return;
}

/*
 * Renders the pick starter menu to screen
 */
void render_pick_starter(int32_t scroller_pos, 
                         Pokemon *p1, Pokemon *p2, Pokemon *p3) { 

  clear(); 
  attron(A_BOLD);
  mvprintw(0,0,"Choose a Starter Pokemon");
  attroff(A_BOLD);
  

  mvaddch(2,0, ACS_VLINE);
  mvprintw(2, 2, "%s  Lv.%d", p1->get_nickname(), p1->get_level());
  mvaddch(3,0, ACS_VLINE);
  mvprintw(3, 2, "%s  Lv.%d", p2->get_nickname(), p2->get_level());
  mvaddch(4,0, ACS_VLINE);
  mvprintw(4, 2, "%s  Lv.%d", p3->get_nickname(), p3->get_level());

  mvaddch(2 + scroller_pos,0, '>');

  mvprintw(6,0,"Press Enter to select a pokemon");
  refresh();
  return;
}

/*
 * Process user input while in an encounter
 */
void process_input_battle(Pokemon *p_pc, int32_t *scroller_pos, 
                          bool *selected_fight, bool *pc_turn) {
  uint32_t no_op = 1;
  int32_t key = 0;

  /* 
   * Pokemon battle menu scroller layout
   * 0 Fight    1 Bag
   * 2 Pokemon  3 Run
   * 
   * Moveslot layout:
   * move 0   move 1
   * move 2   move 3
   */

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_UP) {
      if (*scroller_pos > 0) {
        --(*scroller_pos); 
        no_op = 0;
      }
    } else if (CTRL_DOWN) {
      if ((!*selected_fight && *scroller_pos < 3)
        || (*selected_fight && *scroller_pos < p_pc->get_num_moves() - 1))
       {
        ++(*scroller_pos); 
        no_op = 0;
      }
    } else if (CTRL_SELECT || CTRL_RIGHT) {
      if (*scroller_pos != 0 || *selected_fight) {
        *pc_turn = false; 
        no_op = 0;
      } else {
        *selected_fight = true;
        *scroller_pos = 0;
        no_op = 0;
      }
    } else if (CTRL_BACK || CTRL_LEFT) {
      if (*selected_fight) {
        *selected_fight = false;
        *scroller_pos = 0;
        no_op = 0;
      }
    } else if (CTRL_QUIT_GAME) {
      pc->set_quit_game(true);
      no_op = 0;
    }
  }
  return;
}

/*
 * Process user input while in a center
 */
void process_input_center(int32_t *exit_center) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_EXIT_BLDG) {
      *exit_center = 1;
      no_op = 0;
    } else if (CTRL_QUIT_GAME) {
      pc->set_quit_game(true);
      no_op = 0;
    }
  }
  return;
}

/*
 * Process user input while in a mart
 */
void process_input_mart(int32_t *exit_mart) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_EXIT_BLDG) {
      *exit_mart = 1;
      no_op = 0;
    } else if (CTRL_QUIT_GAME) {
      pc->set_quit_game(true);
      no_op = 0;
    }
  }
  return;
}

/*
 * Process user input while in the trainer overlay
 */
void process_input_tnr_overlay(int32_t *scroller_pos, int32_t *close_overlay) {
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];

  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_TNR_LIST_HIDE) {
      *close_overlay = 1;
      no_op = 0;
    } else if (CTRL_DOWN) {
      if (*scroller_pos < (static_cast<int32_t>(r->get_npcs()->size()) - MAX_ROW)) {
        ++(*scroller_pos);
        no_op = 0;
      }
    } else if (CTRL_UP) {
      if (*scroller_pos > 0) {
        --(*scroller_pos);
        no_op = 0;
      }
    } else if (CTRL_QUIT_GAME) {
      pc->set_quit_game(true);
      no_op = 0;
    }
  }
  return;
}

/*
 * Process user input while in the player bag menu
 */
void process_input_bag(int32_t *page_index, int32_t *scroller_pos, 
                       int32_t *close_bag, int32_t *item_selected) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_CLOSE_BAG) {
      *close_bag = 1;
      no_op = 0;
    } else if (CTRL_DOWN) {
      if (*scroller_pos < pc->num_bag_slots() - 1) {
        ++(*scroller_pos);
        if (*scroller_pos > *page_index + MAX_ROW - 1) {
          ++(*page_index);
        }
        no_op = 0;
      }
    } else if (CTRL_UP) {
      if (*scroller_pos > 0) {
        --(*scroller_pos);
        if (*scroller_pos < *page_index) {
          --(*page_index);
        }
        no_op = 0;
      }
    } else if (CTRL_SELECT) {
        *item_selected = *scroller_pos;
        no_op = 0;
    } else if (CTRL_QUIT_GAME) {
      pc->set_quit_game(true);
      no_op = 0;
    }
  }
  return;
}

/*
 * Process user input while in the pick starter menu
 */
void process_input_pick_starter(int32_t *scroller_pos, 
                                int32_t *selected_pokemon) {
  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_DOWN) {
      if (*scroller_pos < 2) {
        ++(*scroller_pos);
        no_op = 0;
      }
    } else if (CTRL_UP) {
      if (*scroller_pos > 0) {
        --(*scroller_pos);
        no_op = 0;
      }
    } else if (CTRL_SELECT || CTRL_RIGHT) {
      *selected_pokemon = *scroller_pos + 1;
      no_op = 0;
    }
  }
  return;
}

/*
 * Drives interactions in a poke center
 */
void center_driver() {
  int32_t exit_center = 0;

  while (!exit_center && !(pc->is_quit_game())) {
    render_center();
    process_input_center(&exit_center);
  }
}

/*
 * Drives interactions in a poke mart
 */
void mart_driver() {
  int32_t exit_mart = 0;

  while (!exit_mart && !(pc->is_quit_game())) {
    render_mart();
    process_input_mart(&exit_mart);
  }

  return;
}

/*
 * Drives trainer list overlay
 */
void tnr_overlay_driver() {
  int32_t close_overlay = 0;
  int32_t scroller_pos = 0;

  while (!close_overlay && !(pc->is_quit_game())) {
    render_tnr_overlay(scroller_pos);
    process_input_tnr_overlay(&scroller_pos, &close_overlay);
  }
  return;
}

/*
 * Process user input while navigating a region.
 * Update which region is being displayed, generate new regions as needed.
 */
void process_input_nav() {
  // get pointer to present region from global variables
  Region *r = region_ptr[pc->get_x()][pc->get_y()];

  uint32_t no_op = 1;
  int32_t key = 0;

  flushinp();
  while (no_op)  {
    key = getch();
    if (CTRL_UP) {
      no_op = process_pc_move_attempt(dir_n);
    } else if (CTRL_UP_RIGHT) {
      no_op = process_pc_move_attempt(dir_ne);
    } else if (CTRL_RIGHT) {
      no_op = process_pc_move_attempt(dir_e);
    } else if (CTRL_DOWN_RIGHT) {
      no_op = process_pc_move_attempt(dir_se);
    } else if (CTRL_DOWN) {
      no_op = process_pc_move_attempt(dir_s);
    } else if (CTRL_DOWN_LEFT) {
      no_op = process_pc_move_attempt(dir_sw);
    } else if (CTRL_LEFT) {
      no_op = process_pc_move_attempt(dir_w);
    } else if (CTRL_UP_LEFT) {
      no_op = process_pc_move_attempt(dir_nw);
    } else if (CTRL_PASS) {
      // do nothing
      no_op = 0;
    } else if (CTRL_ENTER_BLDG) {
      terrain_t standing_on = r->get_ter(pc->get_i(), pc->get_j());
      if (standing_on == ter_center) {
        center_driver();
        render_region(r);
        no_op = 0;
      } else if (standing_on == ter_mart) {
        mart_driver();
        render_region(r);
        no_op = 0;
      }
    } else if (CTRL_TNR_LIST_SHOW) {
      tnr_overlay_driver();
      render_region(r);
      if (pc->is_quit_game())
        no_op = 0;
    } else if (CTRL_OPEN_BAG) {
      bag_driver();
      render_region(r);
      if (pc->is_quit_game())
        no_op = 0;
    } else if (CTRL_VIEW_PARTY) {
      party_view_driver();
      render_region(r);
      if (pc->is_quit_game())
        no_op = 0;
    } else if (CTRL_QUIT_GAME) {
      pc->set_quit_game(true);
      no_op = 0;
    }
  }
    
  return;
}

void exit_w_message(const char* message) {
  clear();
  mvprintw(0,0, message);
  mvprintw(2,0,"Press any key to exit.");
  refresh();
  int32_t key = 0;
  while (!key)  {
    key = getch();
  }
  endwin();
  exit(-1);
}