#ifndef GLOBAL_EVENTS_H
#define GLOBAL_EVENTS_H

#include <stdint.h>

#include "region.h"

void process_input (int32_t *region_x, int32_t *region_y, uint32_t *running);
void init_pc (character_t *pc, region_t *region);
void free_all_regions();

#endif