#include <limits.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "config.h"
#include "heap.h"
#include "region.h"
#include "pathfinding.h"
#include "global_events.h"
#include "trainer_events.h"

// Global variables
// 2D array of pointers, each pointer points to one of the regions the world
region_t *region_ptr[WORLD_SIZE][WORLD_SIZE] = {NULL};
int32_t dist_map_hiker[MAX_ROW][MAX_COL];
int32_t dist_map_rival[MAX_ROW][MAX_COL];

void usage(const char *argv0) {
  fprintf(stderr, "Usage: %s [--numtrainers|--seed] <int>\n", argv0);
  exit(-1);
}

int main (int argc, char *argv[])
{
  int32_t seed;
  int32_t numtrainers_opt = NUM_TRAINERS;
  int32_t current_region_x = WORLD_SIZE/2;
  int32_t current_region_y = WORLD_SIZE/2;
  int32_t loaded_region_x = current_region_x;
  int32_t loaded_region_y = current_region_y;
  int32_t prev_pc_pos_i = -1;
  int32_t prev_pc_pos_j = -1;
  
   // generate random seed
  struct timeval t;
  gettimeofday(&t, NULL);
  seed = (t.tv_usec ^ (t.tv_sec << 20)) & 0xffffffff;

  // handle command line inputs
  if (argc != 1 && argc != 3) {
    usage(argv[0]);
  }
  if (argc == 3) {
    if (!strcmp(argv[1], "--numtrainers")) {
      numtrainers_opt = atoi(argv[2]);
    } else if (!strcmp(argv[1], "--seed")) {
      seed = atoi(argv[2]);
    } else {
      usage(argv[0]);
      return -1;
    }
  }
  srand(seed);

  init_terminal();

  // start in center of the world. 
  // The center of the world may also be referred to as (0,0)
  character_t pc;
  // Allocate memory for and generate the starting region
  region_t *new_region = (region_t *) malloc(sizeof(*new_region));
  region_ptr[current_region_x][current_region_y] = new_region;
  init_region(region_ptr[current_region_x][current_region_y], -1, -1, -1, -1, 1, 1, numtrainers_opt);

  init_pc(&pc, region_ptr[current_region_x][current_region_y]);
  
  heap_t move_queue;
  character_t *c;
  init_trainer_pq(&move_queue, &pc, region_ptr[current_region_x][current_region_y]);
  recalculate_dist_maps(region_ptr[current_region_x][current_region_y], pc.pos_i, pc.pos_j);

  render_region(region_ptr[current_region_x][current_region_y], &pc);
  usleep(FRAMETIME);

  // Run game
  int32_t quit_game = 0;
  while(!quit_game) { 
    if (current_region_x != loaded_region_x || current_region_y != loaded_region_y) {
      load_region(current_region_x, current_region_y, numtrainers_opt);
      init_trainer_pq(&move_queue, &pc, region_ptr[current_region_x][current_region_y]);
      pc_next_region(&pc, current_region_x, current_region_y, 
                          loaded_region_x,  loaded_region_y);
      loaded_region_x = current_region_x;
      loaded_region_y = current_region_y;
    }

    if (pc.pos_i != prev_pc_pos_i || pc.pos_j != prev_pc_pos_j) {
      recalculate_dist_maps(region_ptr[current_region_x][current_region_y], pc.pos_i, pc.pos_j);
      prev_pc_pos_i = pc.pos_i;
      prev_pc_pos_j = pc.pos_j;
    }

    int32_t ticks_since_last_frame = 0;
    while (ticks_since_last_frame <= TICKS_PER_FRAME) {
      int32_t step = ((character_t*)heap_peek_min(&move_queue))->movetime;
      if (step <= TICKS_PER_FRAME) {
        step_all_movetimes(&pc, region_ptr[loaded_region_x][loaded_region_y], step);
        while( ((character_t *) heap_peek_min(&move_queue))->movetime == 0 ) {
          c = (character_t *) heap_remove_min(&move_queue);
          process_movement_turn(c, &current_region_x, &current_region_y, &pc, &quit_game);
          c->hn = heap_insert(&move_queue, c);
          if (quit_game || current_region_x != loaded_region_x 
                        || current_region_y != loaded_region_y)
            break;
        }
        if (quit_game || current_region_x != loaded_region_x 
                      || current_region_y != loaded_region_y)
          break;
      } else {
        step = TICKS_PER_FRAME;
      }
      step_all_movetimes(&pc, region_ptr[loaded_region_x][loaded_region_y], step);
      ticks_since_last_frame += step;
    }
    render_region(region_ptr[loaded_region_x][loaded_region_y], &pc);
    usleep(FRAMETIME);
  }

  heap_delete(&move_queue);
  free_all_regions();
  endwin();

  return 0;
}