#include <cstdint>
#include <cstdio>
#include <climits>

#include "character.h"
#include "config.h"
#include "heap.h"
#include "region.h"
#include "pathfinding.h"

extern int32_t dist_map_hiker[MAX_ROW][MAX_COL];
extern int32_t dist_map_rival[MAX_ROW][MAX_COL];

typedef struct path {
  heap_node_t *hn;
  int32_t pos_i, pos_j;
  int32_t from_i, from_j;
  int32_t cost;
} path_t;

static int32_t path_cmp(const void *key, const void *with) {
  return ((path_t *) key)->cost - ((path_t *) with)->cost;
}

/*
 * Uses dijkstra's algorithm to find an optimal path to a specified location
 *
 * Returns distance in number of steps between the points.
 * INT_MAX for no valid route.
 */
static void dijkstra(Region *r, trainer_t tnr, int32_t pc_i, int32_t pc_j) {

  static path_t path[MAX_ROW][MAX_COL], *p;
  static uint32_t initialized = 0;
  heap_t h;
  uint32_t i, j;
  terrain_t ter;
  int32_t neighbor_cost;


  if (!initialized) {
    for (i = 0; i < MAX_ROW; i++) {
      for (j = 0; j < MAX_COL; j++) {
        path[i][j].pos_i = i;
        path[i][j].pos_j = j;
      }
    }
    initialized = 1;
  }

  for (i = 0; i < MAX_ROW; i++) {
    for (j = 0; j < MAX_COL; j++) {
      path[i][j].cost = INT_MAX;
    }
  }

  path[pc_i][pc_j].cost = 0;

  heap_init(&h, path_cmp, NULL);

  for (i = 1; i < MAX_ROW - 1; i++) {
    for (j = 1; j < MAX_COL - 1; j++) {
      if (turn_times[r->get_ter(i, j)][tnr]  != INT_MAX) {
        path[i][j].hn = heap_insert(&h, &path[i][j]);
      } else {
        path[i][j].hn = NULL;
      }
    }
  }

  while ((p = (path_t *) heap_remove_min(&h))) {
    p->hn = NULL;

    // North
    ter = r->get_ter(p->pos_i - 1, p->pos_j    );
    neighbor_cost = (p->cost == INT_MAX || turn_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + turn_times[ter][tnr];
    if ((path[p->pos_i - 1][p->pos_j    ].hn) && 
        (path[p->pos_i - 1][p->pos_j    ].cost > neighbor_cost)) {
      path[p->pos_i - 1][p->pos_j    ].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i - 1][p->pos_j    ].hn);
    }
    // South
    ter = r->get_ter(p->pos_i + 1, p->pos_j    );
    neighbor_cost = (p->cost == INT_MAX || turn_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + turn_times[ter][tnr];
    if ((path[p->pos_i + 1][p->pos_j    ].hn) &&
        (path[p->pos_i + 1][p->pos_j    ].cost > neighbor_cost)) {
      path[p->pos_i + 1][p->pos_j    ].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i + 1][p->pos_j    ].hn);
    }
    // East
    ter = r->get_ter(p->pos_i    , p->pos_j + 1);
    neighbor_cost = (p->cost == INT_MAX || turn_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + turn_times[ter][tnr];
    if ((path[p->pos_i    ][p->pos_j + 1].hn) &&
        (path[p->pos_i    ][p->pos_j + 1].cost > neighbor_cost)) {
      path[p->pos_i    ][p->pos_j + 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i    ][p->pos_j + 1].hn);
    }
    // West
    ter = r->get_ter(p->pos_i    , p->pos_j - 1);
    neighbor_cost = (p->cost == INT_MAX || turn_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + turn_times[ter][tnr];
    if ((path[p->pos_i    ][p->pos_j - 1].hn) &&
        (path[p->pos_i    ][p->pos_j - 1].cost > neighbor_cost)) {
      path[p->pos_i    ][p->pos_j - 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i    ][p->pos_j - 1].hn);
    }
    // North East
    ter = r->get_ter(p->pos_i - 1, p->pos_j + 1);
    neighbor_cost = (p->cost == INT_MAX || turn_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : (p->cost + turn_times[ter][tnr]);
    if ((path[p->pos_i - 1][p->pos_j + 1].hn) && 
        (path[p->pos_i - 1][p->pos_j + 1].cost > neighbor_cost)) {
      path[p->pos_i - 1][p->pos_j + 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i - 1][p->pos_j + 1].hn);
    }
    // North West
    ter = r->get_ter(p->pos_i - 1, p->pos_j - 1);
    neighbor_cost = (p->cost == INT_MAX || turn_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + turn_times[ter][tnr];
    if ((path[p->pos_i - 1][p->pos_j - 1].hn) && 
        (path[p->pos_i - 1][p->pos_j - 1].cost > neighbor_cost)) {
      path[p->pos_i - 1][p->pos_j - 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i - 1][p->pos_j - 1].hn);
    }
    // South East
    ter = r->get_ter(p->pos_i + 1, p->pos_j + 1);
    neighbor_cost = (p->cost == INT_MAX || turn_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + turn_times[ter][tnr];
    if ((path[p->pos_i + 1][p->pos_j + 1].hn) && 
        (path[p->pos_i + 1][p->pos_j + 1].cost > neighbor_cost)) {
      path[p->pos_i + 1][p->pos_j + 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i + 1][p->pos_j + 1].hn);
    }
    // South West
    ter = r->get_ter(p->pos_i + 1, p->pos_j - 1);
    neighbor_cost = (p->cost == INT_MAX || turn_times[ter][tnr] == INT_MAX) ? 
                     INT_MAX : p->cost + turn_times[ter][tnr];
    if ((path[p->pos_i + 1][p->pos_j - 1].hn) && 
        (path[p->pos_i + 1][p->pos_j - 1].cost > neighbor_cost)) {
      path[p->pos_i + 1][p->pos_j - 1].cost = neighbor_cost;
      heap_decrease_key_no_replace(&h, path[p->pos_i + 1][p->pos_j - 1].hn);
    }
  }

  if (tnr == tnr_hiker) {
    for (int32_t i = 0; i < MAX_ROW; i++) {
      for (int32_t j = 0; j < MAX_COL; j++) {
        dist_map_hiker[i][j] = path[i][j].cost;
      }
    }
  } else if (tnr == tnr_rival) {
    for (int32_t i = 0; i < MAX_ROW; i++) {
      for (int32_t j = 0; j < MAX_COL; j++) {
        dist_map_rival[i][j] = path[i][j].cost;
      }
    }
  }

  heap_delete(&h);
  return;
}

void print_dist_map(int32_t dist_map[][MAX_COL]) {
  for (int32_t i = 0; i < MAX_ROW; i++) {
    for (int32_t j = 0; j < MAX_COL; j++) {
      if (dist_map[i][j] != INT_MAX) {
        printf("%02d ", dist_map[i][j] % 100);
      } else {
        printf("   ");
      }
    }
    printf("   \n");
  }
}

void recalculate_dist_maps(Region *r, int32_t pc_i, int32_t pc_j) {
  dijkstra(r, tnr_hiker, pc_i, pc_j);
  dijkstra(r, tnr_rival, pc_i, pc_j);
}