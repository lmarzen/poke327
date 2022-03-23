#ifndef GLOBAL_EVENTS_H
#define GLOBAL_EVENTS_H

#include <stdint.h>

#include "region.h"
#include "trainer_events.h"

void process_input_nav(character_t *pc, int32_t *region_x, int32_t *region_y, int32_t *quit_game);
void process_input_battle (battle_t *battle, int32_t *quit_game);
void init_pc (character_t *pc, region_t *region);
void free_all_regions();
void init_terminal();
void render_region(region_t *region, character_t *pc);
void render_battle(battle_t *battle);

#endif