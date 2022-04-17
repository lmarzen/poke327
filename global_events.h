#ifndef GLOBAL_EVENTS_H
#define GLOBAL_EVENTS_H

#include <cstdint>

#include "region.h"
#include "trainer_events.h"
#include "pokemon.h"

void pc_next_region(int32_t to_rx,   int32_t to_ry, 
                    int32_t from_rx, int32_t from_ry);
void load_region(int32_t region_x, int32_t region_y, int32_t num_tnr);
void free_all_regions();
void init_terminal();
void render_region(Region *r);
void render_battle_message(const char* m);
void render_battle(Pokemon *p_pc, Pokemon *p_opp,
                   const char* message, bool show_menu,
                   int32_t scroller_pos, bool selected_fight);
void render_party(int32_t selected_p1, int32_t selected_p2, 
                  int32_t selected_opt, 
                  const char *m, const char *o1, const char *o2);
void render_party_message(const char* m);
void render_summary(Pokemon *p);
void render_bag(int32_t page_index, int32_t scroller_pos);
void render_pick_starter(int32_t scroller_pos, 
                         Pokemon *p1, Pokemon *p2, Pokemon *p3);
void process_input_battle(Pokemon *p_pc, int32_t *scroller_pos, 
                          bool *selected_fight, bool *pc_turn);
void process_input_party(int32_t scenario,
                         int32_t *selected_p1, int32_t *selected_p2, 
                         int32_t *selected_opt, int32_t *close_party);
void process_input_bag(int32_t *page_index, int32_t *scroller_pos, 
                       int32_t *close_bag, int32_t *item_selected);
void process_input_pick_starter(int32_t *scroller_pos, 
                                int32_t *selected_pokemon);
void process_input_nav();
void exit_w_message(const char* message);
void getch_select();
void quit_game();

#endif