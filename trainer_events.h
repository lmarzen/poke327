#ifndef TRAINER_EVENTS_H
#define TRAINER_EVENTS_H

#include <stdint.h>

#include "heap.h"
#include "region.h"

void init_trainer_pq(heap_t *queue, character_t *pc, region_t *region);
void move_trainer(character_t *c, region_t *region, character_t *pc);
void step_all_movetimes(character_t *pc, region_t *region, int32_t amount);

#endif