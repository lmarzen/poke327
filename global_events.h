#ifndef GLOBAL_EVENTS_H
#define GLOBAL_EVENTS_H

#include <stdint.h>

#include "region.h"

void process_input (character_t *pc, int32_t *region_x, int32_t *region_y, int32_t *quit);
void init_pc (character_t *pc, region_t *region);
void free_all_regions();
void init_terminal();
void render_region(region_t *region, character_t *pc);

#endif