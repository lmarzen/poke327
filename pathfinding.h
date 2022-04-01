#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <cstdint>

#include "config.h"
#include "region.h"

void print_dist_map(int32_t dist_map[][MAX_COL]);
void recalculate_dist_maps(Region *r, int32_t pc_i, int32_t pc_j);

#endif