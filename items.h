#ifndef ITEMS_H
#define ITEMS_H

#include <cstdint>

typedef enum item {
  item_empty = -1,
  item_poke_ball,
  item_great_ball,
  item_ultra_ball,
  item_master_ball,
  item_potion,
  item_super_potion,
  item_hyper_potion,
  item_max_potion,
  item_ether,
  item_max_ether,
  item_elixir,
  item_max_elixir,
  item_revive,
  item_max_revive,
  item_rare_candy,
  num_items
} item_t;

const char item_name_txt[num_items][13] {
  "POKE BALL",
  "GREAT BALL",
  "ULTRA BALL",
  "MASTER BALL",
  "POTION",
  "SUPER POTION",
  "HYPER POTION",
  "MAX POTION",
  "ETHER",
  "MAX ETHER",
  "ELIXIR",
  "MAX ELIXIR",
  "REVIVE",
  "MAX REVIVE",
  "RARE CANDY"
};

const char item_desc_txt[num_items * 2][MAX_COL + 1] {
  // POKE BALL
  "A Ball thrown to catch a wild Pokemon. It is designed in a capsule style.",
  "",
  // GREAT BALL
  "A good, quality Ball that offers a higher Pokemon catch rate than a standard",
  "Poke Ball.",
  // ULTRA BALL
  "A better Ball with a higher catch rate than a Great Ball.",
  "",
  // MASTER BALL
  "The best Ball with the ultimate performance. It will catch any wild Pokemon",
  "without fail.",
  // POTION
  "A spray-type wound medicine. It restores the HP of one Pokemon by 20 points.",
  "",
  // SUPER POTION
  "A spray-type wound medicine. It restores the HP of one Pokemon by 50 points.",
  "",
  // HYPER POTION
  "A spray-type wound medicine. It restores the HP of one Pokemon by 200 points.",
  "",
  // MAX POTION
  "A spray-type wound medicine. It fully restores the HP of one Pokemon.",
  "",
  // ETHER
  "Restores a selected move's PP by 10 points for one Pokemon.",
  "",
  // MAX ETHER
  "Fully restores a selected move's PP for one Pokemon.",
  "",
  // ELIXIR
  "Restores the PP of all moves for one Pokemon by 10 points each.",
  "",
  // MAX ELIXIR
  "Fully restores the PP of all moves for one Pokemon.",
  "",
  // REVIVE
  "A medicine that revives a fainted Pokemon, restoring HP by half the maximum",
  "amount.",
  // MAX REVIVE
  "A medicine that revives a fainted Pokemon, restoring HP fully.",
  "",
  // RARE CANDY
  "A candy that is packed with energy. It raises the level of a Pokemon by one.",
  ""
};

typedef struct bag_slot {
  item_t item;
  int32_t cnt;
} bag_slot_t;

#endif