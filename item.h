#ifndef ITEM_H
#define ITEM_H

#include <cstdint>

typedef enum item {
  item_pokeball,
  item_potion,
  item_revive,
  num_items
} item_t;

typedef struct bag_slot {
  item_t item;
  int32_t cnt;
} bag_slot_t;

const char item_name_txt[num_items][30] {
  "Pokeball",
  "Potion",
  "Revive"
};

#endif