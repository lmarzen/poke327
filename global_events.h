#ifndef GLOBAL_EVENTS_H
#define GLOBAL_EVENTS_H

#include <cstdint>

#include "region.h"
#include "trainer_events.h"

void pc_next_region(int32_t to_rx,   int32_t to_ry, 
                    int32_t from_rx, int32_t from_ry);
void load_region(int32_t region_x, int32_t region_y, int32_t num_tnr);
void free_all_regions();
void init_terminal();
void render_region(Region *r);
void render_battle(battle_t *battle);
void process_input_battle(battle_t *battle);
void render_encounter(encounter_t *encounter);
void process_input_encounter(encounter_t *encounter);
void process_input_nav();
void exit_w_message(const char* message);

#endif