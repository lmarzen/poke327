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
#include "items.h"

extern Region *region_ptr[WORLD_SIZE][WORLD_SIZE];
extern Pc *pc;
extern heap_t move_queue;;


int32_t digits(int32_t n)  
{  
    int32_t counter = 0;
    while(n != 0)  
    {  
        n = n / 10;  
        ++counter;  
    }  
    return counter;  
}

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
  set_escdelay(100);

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
 * Updates the battle message
 */
void render_battle_message(const char* m) {
  for (int32_t i = 12; i < MAX_ROW + 3; ++i) {
    move(i, 0);
    clrtoeol();
  }
  
  mvprintw(12, 0, m);
  refresh();
}

/*
 * Updates the battle message and waits for next key
 */
void render_battle_message_getch(const char* m) {
  render_battle_message(m);
  
  // also wait for keypress
  usleep(FRAMETIME);
  flushinp();
  getch_next();
}

/*
 * Renders a battle to the screen
 */
void render_battle(Pokemon *p_pc, Pokemon *p_opp,
                   const char* message, bool show_menu,
                   int32_t scroller_pos, bool selected_fight) { 
  int32_t color;
  clear();

  // OPPONENT POKEMON
  if (p_opp->is_shiny()) {
    attron(A_BOLD);
    mvaddch(2, 1,'*');
    attroff(A_BOLD);
  }    
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
  mvprintw(2, 21 - digits(p_opp->get_level()), "Lv.%d", p_opp->get_level());
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
  if (p_pc->is_shiny()) {
    attron(A_BOLD);
    mvaddch(7, 1,'*');
    attroff(A_BOLD);
  }     
  attron(A_BOLD);
  mvprintw(7, 2,"%s ", p_pc->get_nickname());
  attroff(A_BOLD);
  if (p_pc->get_gender() == gender_male) {
    attron(COLOR_PAIR(COLOR_BLUE));
    addch('m');
    attroff(COLOR_PAIR(COLOR_BLUE));
  } else {
    attron(COLOR_PAIR(COLOR_MAGENTA));
    addch('f');
    attroff(COLOR_PAIR(COLOR_MAGENTA));
  }     
  mvprintw(7, 21 - digits(p_pc->get_level()), "Lv.%d", p_pc->get_level());
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
  mvprintw(9, 17,"%3d/%3d", p_pc->get_current_hp(), p_pc->get_stat(stat_hp));
  mvprintw(10, 2, "EXP");
  attron(COLOR_PAIR(CHAR_COLOR_EXP));
  for (int32_t e = 19 * p_pc->get_exp(); e > 0; 
       e -= p_pc->get_exp_next_level()) {
    addch(CHAR_EXP);
  }
  attroff(COLOR_PAIR(CHAR_COLOR_EXP));

  mvprintw(12, 0, message);
  
  if (show_menu) {
    mvaddch(13, 0, (0 == scroller_pos ? CHAR_CURSOR : CHAR_SCROLL_BAR));
    if (!selected_fight) {
      printw(" FIGHT");
    } else if (p_pc->get_num_moves() > 0) {
      printw(" %s", p_pc->get_move(0)->identifier);
    }
    mvaddch(14, 0, (1 == scroller_pos ? CHAR_CURSOR : CHAR_SCROLL_BAR));
    if (!selected_fight) {
      printw(" BAG");
    } else if (p_pc->get_num_moves() > 1) {
      printw(" %s", p_pc->get_move(1)->identifier);
    }
    mvaddch(15, 0, (2 == scroller_pos ? CHAR_CURSOR : CHAR_SCROLL_BAR));
      if (!selected_fight) {
      printw(" POKEMON");
    } else if (p_pc->get_num_moves() > 2) {
      printw(" %s", p_pc->get_move(2)->identifier);
    }
    mvaddch(16, 0, (3 == scroller_pos ? CHAR_CURSOR : CHAR_SCROLL_BAR));
    if (!selected_fight) {
      printw(" RUN");
    } else if (p_pc->get_num_moves() > 3) {
      printw(" %s", p_pc->get_move(3)->identifier);
    }

    if (selected_fight && p_pc->get_num_moves() > 0) {
      mvprintw(17, 0, "PP  %d/%d", p_pc->get_current_pp(scroller_pos), 
                                   p_pc->get_move(scroller_pos)->pp);
      mvprintw(18, 0, "type/%s", type_name(p_pc->get_move(scroller_pos)->type_id));
    }

  }

  refresh();
  return;
}

void render_battle_getch(Pokemon *p_pc, Pokemon *p_opp,
                         const char* message, bool show_menu,
                         int32_t scroller_pos, bool selected_fight) {
  render_battle(p_pc, p_opp, message, show_menu, scroller_pos, selected_fight);

  usleep(FRAMETIME);
  flushinp();
  getch_next();
}

void render_party_message(const char *m) {
  // if (display_options) {
  //   move(10, 0);
  //   clrtoeol();
  //   mvprintw(10, 0, m);
  // } else {
  //   move(6, 0);
  //   clrtoeol();
  //   mvprintw(6, 0, m);
  // }

  move(10, 0);
  clrtoeol();
  mvprintw(10, 0, m);
  
  refresh();
  // also wait for keypress
  usleep(FRAMETIME);
  flushinp();
  getch_next();
}

void render_party(int32_t selected_p1, int32_t selected_p2, 
                  int32_t selected_opt, 
                  const char *m, const char *o1, const char *o2) {
  int32_t i, j;
  Pokemon *p;
  int32_t color;

  clear(); 
  attron(A_BOLD);
  mvprintw(0,0,"Pokemon");
  attroff(A_BOLD);

  for (i = 0; i < pc->get_party_size(); ++i) {
    j = (i > selected_p1 && selected_opt != -1) ? 4 : 1;
    p = pc->get_pokemon(i);
    if (i == selected_p1) {
      mvaddch(i + j, 0, selected_opt == -1 ? CHAR_CURSOR 
                                           : CHAR_CURSOR_SELECTED);
    } else if (i == selected_p2) {
      mvaddch(i + j, 0, CHAR_CURSOR);
    } else if (selected_opt == -1 || selected_p2 != -1) {
      mvaddch(i + j, 0, CHAR_SCROLL_BAR);
    }

    if (p->is_shiny()) {
      attron(A_BOLD);
      mvaddch(i + j, 1,'*');
      attroff(A_BOLD);
    } 
    attron(A_BOLD);
    if (i == selected_p1) {
      mvprintw(i + j, 2, "%s ", p->get_nickname());
    } else {
      mvprintw(i + j, 2, "%s ", p->get_nickname());
    }
    attroff(A_BOLD);
    
    if (p->get_gender() == gender_male) {
      attron(COLOR_PAIR(COLOR_BLUE));
      addch('m');
      attroff(COLOR_PAIR(COLOR_BLUE));
    } else {
      attron(COLOR_PAIR(COLOR_MAGENTA));
      addch('f');
      attroff(COLOR_PAIR(COLOR_MAGENTA));
    }     
    mvprintw(i + j, 21 - digits(p->get_level()), "Lv.%d", p->get_level());
    mvprintw(i + j, 26, "HP");
    if (p->get_current_hp() < p->get_stat(stat_hp) / 5) {
      color = CHAR_COLOR_HEALTH_LOW;
    } else if (p->get_current_hp() < p->get_stat(stat_hp) / 2) {
      color = CHAR_COLOR_HEALTH_MED;
    } else {
      color = CHAR_COLOR_HEALTH_HIGH;
    }
    attron(COLOR_PAIR(color));
    for (int32_t h = 12 * p->get_current_hp(); h > 0; 
        h -= p->get_stat(stat_hp)) {
      addch(CHAR_HEALTH);
    }
    attroff(COLOR_PAIR(color));
    mvprintw(i + j, 40,"%3d/%3d", p->get_current_hp(), p->get_stat(stat_hp));
    mvprintw(i + j, 49, "EXP");
    attron(COLOR_PAIR(CHAR_COLOR_EXP));
    for (int32_t e = 12 * p->get_exp(); e > 0; 
        e -= p->get_exp_next_level()) {
      addch(CHAR_EXP);
    }
    attroff(COLOR_PAIR(CHAR_COLOR_EXP));

    // draw submenu
    if (selected_opt != -1 && i == selected_p1) {
      if (selected_opt == 0) {
        if (selected_p2 == -1) {
          mvaddch(i + j + 1, 2, CHAR_CURSOR);
          mvaddch(i + j + 2, 2, CHAR_SCROLL_BAR);
          mvaddch(i + j + 3, 2, CHAR_SCROLL_BAR);
        } else {
          mvaddch(i + j + 1, 2, CHAR_CURSOR_SELECTED);
        }
      } else if (selected_opt == 1) {
        mvaddch(i + j + 1, 2, CHAR_SCROLL_BAR);
        mvaddch(i + j + 2, 2, CHAR_CURSOR);
        mvaddch(i + j + 3, 2, CHAR_SCROLL_BAR);
      }  else if (selected_opt == 2) {
        mvaddch(i + j + 1, 2, CHAR_SCROLL_BAR);
        mvaddch(i + j + 2, 2, CHAR_SCROLL_BAR);
        mvaddch(i + j + 3, 2, CHAR_CURSOR);
      } 
      mvprintw(i + j + 1, 4, "%s", o1);
      mvprintw(i + j + 2, 4, "%s", o2);
      mvprintw(i + j + 3, 4, "CANCEL");
    }

    // if (selected_p2 != -1 && i == selected_p1) {
    //   mvaddch(i + j + 1, 0, CHAR_SCROLL_BAR);
    //   mvaddch(i + j + 2, 0, CHAR_SCROLL_BAR);
    //   mvaddch(i + j + 3, 0, CHAR_SCROLL_BAR);
    // }
  }

  // jumping text (kinda distracting, opting not to use for now)
  // j = (i > selected_p1 && selected_opt != -1) ? 4 : 1;
  // mvprintw(6 + j,0,"%s", m);

  mvprintw(10,0,"%s", m);

  // scroller debug
  // mvprintw(8+j,0,"p1: %d  opt: %d p2: %d", 
  //            selected_p1, selected_opt, selected_p2);

  refresh();
  return;
}

void render_party_getch(int32_t selected_p1, int32_t selected_p2, 
                        int32_t selected_opt, 
                        const char *m, const char *o1, const char *o2) {
  render_party(selected_p1, selected_p2, selected_opt, m, o1, o2);

  usleep(FRAMETIME);
  flushinp();
  getch_next();
}

void render_summary(Pokemon *p) {
  clear();

  move(0, 0);
  attron(A_BOLD);
  if (p->is_shiny())
    addch('*');
  printw("%s ", p->get_nickname());
  attroff(A_BOLD);
  
  if (p->get_gender() == gender_male) {
    attron(COLOR_PAIR(COLOR_BLUE));
    addch('m');
    attroff(COLOR_PAIR(COLOR_BLUE));
  } else {
    attron(COLOR_PAIR(COLOR_MAGENTA));
    addch('f');
    attroff(COLOR_PAIR(COLOR_MAGENTA));
  }

  mvprintw(0, 19 - digits(p->get_level()), "Lv.%d", p->get_level());

  mvprintw(1, 0, "DEX NO.: %d %s", p->get_pd_entry()->id, 
                                   p->get_pd_entry()->identifier);
  mvprintw(2, 0, "TYPE   : %s %s", type_name(p->get_type(0)), 
                                   type_name(p->get_type(1)));
  mvprintw(4, 0, "STATS");
  mvprintw(5, 2, "HP     : %3d/%3d", p->get_current_hp(), p->get_stat(stat_hp));
  mvprintw(6, 2, "ATTACK : %d", p->get_stat(stat_attack));
  mvprintw(7, 2, "DEFENSE: %d", p->get_stat(stat_defense));
  mvprintw(8, 2, "SP. ATK: %d", p->get_stat(stat_sp_atk));
  mvprintw(9, 2, "SP. DEF: %d", p->get_stat(stat_sp_def));
  mvprintw(10,2, "SPEED  : %d", p->get_stat(stat_speed));

  #ifdef POKEMON_SUMMARY_SHOW_IVS
  mvprintw(3, 17, "IVs");
  mvprintw(4, 17, "%d", p->get_iv(stat_hp));
  mvprintw(5, 17, "%d", p->get_iv(stat_attack));
  mvprintw(6, 17, "%d", p->get_iv(stat_defense));
  mvprintw(7, 17, "%d", p->get_iv(stat_sp_atk));
  mvprintw(8, 17, "%d", p->get_iv(stat_sp_def));
  mvprintw(9, 17, "%d", p->get_iv(stat_speed));
  #endif

  mvprintw(12,0, "EXP");
  mvprintw(13,2, "EXP POINTS: %d", p->get_exp());
  mvprintw(14,2, "NEXT LV.  : %d", p->get_exp_next_level());

  mvprintw(16,0, "MOVES");
  if (p->get_num_moves() > 0) {
    mvprintw(17,2, "%s", p->get_move(0)->identifier);
    mvprintw(17,22, "PP");
    mvprintw(17,29 - digits(p->get_current_pp(0)) - digits(p->get_move(0)->pp), 
             "%d/%d  type/%s", p->get_current_pp(0), 
                               p->get_move(0)->pp,
                               type_name(p->get_move(0)->type_id));
  }
  if (p->get_num_moves() > 1) {
    mvprintw(18,2, "%s", p->get_move(1)->identifier);
    mvprintw(18,22, "PP");
    mvprintw(18,29 - digits(p->get_current_pp(1)) - digits(p->get_move(1)->pp), 
             "%d/%d  type/%s", p->get_current_pp(1), 
                               p->get_move(1)->pp,
                               type_name(p->get_move(1)->type_id));
  }
  if (p->get_num_moves() > 2) {
    mvprintw(19,2, "%s", p->get_move(2)->identifier);
    mvprintw(19,22, "PP");
    mvprintw(19,29 - digits(p->get_current_pp(2)) - digits(p->get_move(2)->pp), 
             "%d/%d  type/%s", p->get_current_pp(2), 
                               p->get_move(2)->pp,
                               type_name(p->get_move(2)->type_id));
  }
  if (p->get_num_moves() > 3) {
    mvprintw(20,2, "%s", p->get_move(3)->identifier);
    mvprintw(20,22, "PP");
    mvprintw(20,29 - digits(p->get_current_pp(3)) - digits(p->get_move(3)->pp), 
             "%d/%d  type/%s", p->get_current_pp(3), 
                               p->get_move(3)->pp,
                               type_name(p->get_move(3)->type_id));
  }

  mvprintw(22,0,"Press ESC to exit summary view");

  refresh();

  // wait for back input
  int32_t key;
  flushinp();
  while (true) {
    key = getch();
    if (CTRL_BACK) {
      return;
    } else if (CTRL_QUIT_GAME) {
      quit_game();
    }
  }
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

    mvaddch(i + 1,0, CHAR_SCROLL_BAR);
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

void render_bag_message(const char *m) {
  int32_t i = pc->num_bag_slots() < MAX_ROW ? 
              pc->num_bag_slots() + 2 : MAX_ROW - 2;
  move(i, 0);
  clrtoeol();
  printw(m);
  refresh();
  // also wait for keypress
  getch_next();
}

/*
 * Renders the players bag to the screen
 */
void render_bag(int32_t page_index, int32_t scroller_pos) { 
  int32_t i;
  bag_slot_t s, selected_slot;

  clear(); 
  attron(A_BOLD);
  mvprintw(0,0,"Bag");
  attroff(A_BOLD);

  for (i = 0; (i < pc->num_bag_slots() && i < MAX_ROW - 3); ++i) {
    s = pc->peek_bag_slot(i + page_index);
    if (i == scroller_pos - page_index) {
      mvaddch(i + 1,0, CHAR_CURSOR);
      selected_slot = pc->peek_bag_slot(i + page_index); 
    } else {
      mvaddch(i + 1,0, CHAR_SCROLL_BAR);
    }
    
    mvprintw(i + 1, 2, "%3dx %s", s.cnt, item_name_txt[s.item]);
  }

  mvprintw(i + 2,0, item_desc_txt[selected_slot.item * 2]);
  mvprintw(i + 3,0, item_desc_txt[selected_slot.item * 2 + 1]);

  mvprintw(i + 5,0,"Choose an ITEM.");
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
  

  mvaddch(2,0, CHAR_SCROLL_BAR);
  mvprintw(2, 2, "%s  Lv.%d", p1->get_nickname(), p1->get_level());
  mvaddch(3,0, CHAR_SCROLL_BAR);
  mvprintw(3, 2, "%s  Lv.%d", p2->get_nickname(), p2->get_level());
  mvaddch(4,0, CHAR_SCROLL_BAR);
  mvprintw(4, 2, "%s  Lv.%d", p3->get_nickname(), p3->get_level());

  mvaddch(2 + scroller_pos,0, CHAR_CURSOR);

  mvprintw(6,0,"Press Enter to select a pokemon");
  refresh();
  return;
}

/*
 * Process user input while in a battle
 */
void process_input_battle(Pokemon *p_pc, int32_t *scroller_pos, 
                          bool *selected_fight, bool *pc_turn) {
  uint32_t no_op = 1;
  int32_t key = 0;

  /* 
   * Pokemon battle menu scroller layout
   * 0 Fight    
   * 1 Bag
   * 2 Pokemon  
   * 3 Run
   * 
   * Moveslot layout:
   * move 0   
   * move 1
   * move 2   
   * move 3
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
    } else if (CTRL_SELECT) {
      if (*scroller_pos != 0 || *selected_fight) {
        *pc_turn = false; 
        no_op = 0;
      } else {
        *selected_fight = true;
        if (!p_pc->has_pp()) {
          *pc_turn = false; 
        }
        no_op = 0;
      }
    } else if (CTRL_BACK) {
      if (*selected_fight) {
        *selected_fight = false;
        *scroller_pos = 0;
        no_op = 0;
      }
    } else if (CTRL_QUIT_GAME) {
      quit_game();
    }
  }
  return;
}

/*
 * Render teach move screen
 * new_move is null then the new_move info won't be displayed
 * If scroller_pos is -1, then scroller will not be displayed
 */
void render_select_move(Pokemon *p, pd_move_t *new_move, int32_t scroller_pos, 
                       const char *m1, const char *m2, const char *m_cancel) {
  clear();
  if (m1 != NULL)
    mvprintw(0, 0, m1);
  if (m2 != NULL)
    mvprintw(2, 0, m2);

  if (p->get_num_moves() > 0) {
    mvprintw(3,2, "%s", p->get_move(0)->identifier);
    mvprintw(3,22, "PP");
    mvprintw(3,29 - digits(p->get_current_pp(0)) - digits(p->get_move(0)->pp), 
             "%d/%d  type/%s", p->get_current_pp(0), 
                               p->get_move(0)->pp,
                               type_name(p->get_move(0)->type_id));
  }
  if (p->get_num_moves() > 1) {
    mvprintw(4,2, "%s", p->get_move(1)->identifier);
    mvprintw(4,22, "PP");
    mvprintw(4,29 - digits(p->get_current_pp(1)) - digits(p->get_move(1)->pp), 
             "%d/%d  type/%s", p->get_current_pp(1), 
                               p->get_move(1)->pp,
                               type_name(p->get_move(1)->type_id));
  }
  if (p->get_num_moves() > 2) {
    mvprintw(5,2, "%s", p->get_move(2)->identifier);
    mvprintw(5,22, "PP");
    mvprintw(5,29 - digits(p->get_current_pp(2)) - digits(p->get_move(2)->pp), 
             "%d/%d  type/%s", p->get_current_pp(2), 
                               p->get_move(2)->pp,
                               type_name(p->get_move(2)->type_id));
  }
  if (p->get_num_moves() > 3) {
    mvprintw(6,2, "%s", p->get_move(3)->identifier);
    mvprintw(6,22, "PP");
    mvprintw(6,29 - digits(p->get_current_pp(3)) - digits(p->get_move(3)->pp), 
             "%d/%d  type/%s", p->get_current_pp(3), 
                               p->get_move(3)->pp,
                               type_name(p->get_move(3)->type_id));
  }
  if (new_move != NULL) {
    mvprintw(9,0, "%s", new_move->identifier);
    mvprintw(9,20, "PP");
    mvprintw(9,27 - digits(p->get_current_pp(3)) - digits(new_move->pp), 
             "%d/%d  type/%s", p->get_current_pp(3), 
                               p->get_move(3)->pp,
                               type_name(new_move->type_id));
  }

  if (scroller_pos != -1) {
    if (m_cancel != NULL) {
      mvprintw(3 + p->get_num_moves(),2, m_cancel);
    }

    for (int32_t i = 0; i <= p->get_num_moves(); ++i) {
      if (scroller_pos == i) {
        mvaddch(i + 3, 0, CHAR_CURSOR);
      } else {
        mvaddch(i + 3, 0, CHAR_SCROLL_BAR);
      }
    }
  }

  refresh();
  return;
}

void render_select_move_getch(Pokemon *p, pd_move_t *new_move, 
                             int32_t scroller_pos, 
                             const char *m1, const char *m2, 
                             const char *m_cancel) {
  render_select_move(p, new_move, scroller_pos, m1, m2, m_cancel);

  usleep(FRAMETIME);
  flushinp();
  getch_next();
}

void process_input_select_move(Pokemon *p, int32_t *scroller_pos, 
                               int32_t *close_view) {
  uint32_t no_op = 1;
  int32_t key = 0;

  /* Scroller layout
   * 0 move 0
   * 1 move 1
   * 2 move 2
   * 3 move 3
   * 4 Give up trying to teach a new move to %s?
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
      if (*scroller_pos < 4)
       {
        ++(*scroller_pos); 
        no_op = 0;
      }
    } else if (CTRL_SELECT) {
      *close_view = true; 
      no_op = 0;
    } else if (CTRL_BACK) {
      *scroller_pos = 4; 
      *close_view = true; 
      no_op = 0;
    } else if (CTRL_QUIT_GAME) {
      quit_game();
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
      quit_game();
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
      quit_game();
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
      quit_game();
    }
  }
  return;
}

/*
 * Process user input while in the pokemon party view menu
 *
 * scenario:
 * 0 = wandering          (allows unlimited switches)
 *   o1 = Switch, 
 *   o2 = Summary
 * 1 = battle             (allows 1 switch)
 *   o1 = Switch, 
 *   o2 = Summary
 * 2 = battle select next (select 1 pokemon)
 *   o1 = Switch, 
 *   o2 = Summary
 * 3 = use item           (select 1 pokemon)
 * 
 * Returns 1 if turn was used, 0 otherwise
 */
void process_input_party(int32_t scenario,
                         int32_t *selected_p1, int32_t *selected_p2, 
                         int32_t *selected_opt, int32_t *close_party) {
  uint32_t no_op = 1;
  int32_t key = 0;

  // TODO: block poke switch attempts if there is only 1 pokemon

  flushinp();
  while (no_op)  {
    key = getch();
    if ((CTRL_CLOSE_PARTY) && scenario == 0 && *selected_opt == -1) {
      *selected_p1 = -1;
      *close_party = 1;
      no_op = 0;
    } else if (CTRL_DOWN) {
      if (*selected_opt == -1 && *selected_p2 == -1
       && *selected_p1 < pc->get_party_size() - 1) {
        ++(*selected_p1);
        no_op = 0;
      } else if (*selected_opt != -1 && *selected_opt < 2 
         && *selected_p2 == -1) {
        ++(*selected_opt);
        no_op = 0;
      } else if (*selected_p2 != -1 
         && *selected_p2 < pc->get_party_size() - 1) {
        if (*selected_p2 + 1 != *selected_p1) {
          ++(*selected_p2);
          no_op = 0;
        } else if (*selected_p2 + 2 < pc->get_party_size() - 1) {
          *selected_p2 += 2;
          no_op = 0;
        }
      }
    } else if (CTRL_UP) {
      if (*selected_opt == -1 && *selected_p2 == -1 && *selected_p1 > 0) {
        --(*selected_p1);
        no_op = 0;
      } else if (*selected_opt != -1 && *selected_opt > 0 
         && *selected_p2 == -1) {
        --(*selected_opt);
        no_op = 0;
      } else if (*selected_p2 != -1 && *selected_p2 > 0) {
        if (*selected_p2 - 1 != *selected_p1) {
          --(*selected_p2);
          no_op = 0;
        } else if (*selected_p2 - 2 >= 0) {
          *selected_p2 -= 2;
          no_op = 0;
        }
      }
    } else if (CTRL_SELECT) {
      if (scenario == 3) {
        // selected pokemon to use item on
        *close_party = 1;
        no_op = 0;
      } else if (*selected_opt == -1 && *selected_p2 == -1) {
        *selected_opt = 0;
        no_op = 0;
      } else if (*selected_opt == 0 && *selected_p2 == -1) {
        if (scenario == 1 || scenario == 2) {
          // selected SHIFT/SEND OUT
          // int32_t active_p = pc->get_active_pokemon_index();
          // assume position 0 is active pokemon
          if (pc->get_pokemon(*selected_p1)->is_fainted()) {
            char m[MAX_COL];
            sprintf(m, "%s has no energy left to battle!", 
                    pc->get_pokemon(*selected_p1)->get_nickname());
            render_party_message(m);
            no_op = 0;
          } else if (0 == *selected_p1 && scenario == 1) {
            char m[MAX_COL];
            sprintf(m, "%s is already in battle!", 
                    pc->get_pokemon(0)->get_nickname());
            render_party_message(m);
            // pokemon is already active
            no_op = 0;
          } else {
            // swap active and selected
            pc->switch_pokemon(0, *selected_p1);
            *close_party = 1;
            no_op = 0;
          }
        } else {
          // selected SHIFT
          *selected_p2 = (*selected_p1 == 0 ? 1 : 0);
          no_op = 0;
        }
      } else if (*selected_opt == 0 && *selected_p2 != -1) {
        // selected second pokemon to shift
        if (pc->get_party_size() < 2) {
          // No pokemon to switch with
          *selected_p2 = -1;
          no_op = 0;
        } else if (pc->get_pokemon(*selected_p1)->is_fainted() 
                    && *selected_p2 == 0) {
          // trying to switch a fainted pokemon to slot 0.
          char m[MAX_COL];
          sprintf(m, "%s has no energy left!", 
                  pc->get_pokemon(*selected_p1)->get_nickname());
          render_party_message(m);
          *selected_p2 = -1;
          no_op = 0;
        } else if (pc->get_pokemon(*selected_p2)->is_fainted()
                    && *selected_p1 == 0) {
          // trying to switch a fainted pokemon to slot 0.
          char m[MAX_COL];
          sprintf(m, "%s has no energy left!", 
                  pc->get_pokemon(*selected_p2)->get_nickname());
          render_party_message(m);
          *selected_p2 = -1;
          no_op = 0;
        } else {  
          pc->switch_pokemon(*selected_p1, *selected_p2);
          *selected_opt = -1;
          *selected_p2 = -1;
          no_op = 0;
        }
      } else if (*selected_opt == 1 && *selected_p2 == -1) {
        // selected SUMMARY
        render_summary(pc->get_pokemon(*selected_p1));
        no_op = 0;
      } else if (*selected_opt == 2 && *selected_p2 == -1) {
        // selected CANCEL
        *selected_opt = -1;
        no_op = 0;
      }
    } else if (CTRL_BACK) {
      if (*selected_opt == -1 && scenario != 2) {
        // close menu
        *selected_p1 = -1;
        *close_party = 1;
        no_op = 0;
      } else if (*selected_p2 == -1) {
        // exit pokemon submenu
        *selected_opt = -1;
        no_op = 0;
      } else if (*selected_opt == 0) {
        // exit shift pokemon
        *selected_p2 = -1;
        no_op = 0;
      }
    } else if (CTRL_QUIT_GAME) {
      quit_game();
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
      quit_game();
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

  while (!exit_center) {
    render_center();
    process_input_center(&exit_center);
  }
}

/*
 * Drives interactions in a poke mart
 */
void mart_driver() {
  int32_t exit_mart = 0;

  while (!exit_mart) {
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

  while (!close_overlay) {
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
    } else if (CTRL_OPEN_BAG) {
      item_t selected_item = bag_driver();
      use_item(pc, NULL, NULL, selected_item);
      render_region(r);
    } else if (CTRL_VIEW_PARTY) {
      party_view_driver(0);
      render_region(r);
    } else if (CTRL_QUIT_GAME) {
      quit_game();
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

void getch_next() {
  int32_t key;
  flushinp();
  while (true) {
    key = getch();
    if (CTRL_SELECT) {
      return;
    } else if (CTRL_BACK) {
      return;
    } else if (CTRL_QUIT_GAME) {
      quit_game();
    }
  }
}

void quit_game() {
  endwin();
  heap_delete(&move_queue);
  free_all_regions();
  exit(1);
}