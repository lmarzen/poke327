#ifndef ITEMS_H
#define ITEMS_H

#include <cstdint>

typedef enum item {
  item_empty = -1,
  item_pokeball,
  item_potion,
  item_revive,
  num_items
} item_t;

const char item_name_txt[num_items][12] {
  "POKE BALL",
  "POTION",
  "REVIVE"
};

typedef struct bag_slot {
  item_t item;
  int32_t cnt;
} bag_slot_t;

#endif