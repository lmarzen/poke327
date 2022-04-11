#ifndef CONFIG_H
#define CONFIG_H

#include <ncurses.h>

// Controls
#define CTRL_N     key == 'k' || key == '8'
#define CTRL_NE    key == 'u' || key == '9'
#define CTRL_E     key == 'l' || key == '6'
#define CTRL_SE    key == 'n' || key == '3'
#define CTRL_S     key == 'j' || key == '2'
#define CTRL_SW    key == 'b' || key == '1'
#define CTRL_W     key == 'h' || key == '4'
#define CTRL_NW    key == 'y' || key == '7'
#define CTRL_PASS  key == ' ' || key == '5' || key == '.'
#define CTRL_ENTER_BLDG     key == '>'
#define CTRL_EXIT_BLDG      key == '<'
#define CTRL_TNR_LIST_SHOW  key == 't'
#define CTRL_TNR_LIST_HIDE  key == 27 // ESC
#define CTRL_SCROLL_UP      key == KEY_UP
#define CTRL_SCROLL_DOWN    key == KEY_DOWN
#define CTRL_QUIT_GAME      key == 'Q'
#define CTRL_LEAVE_BATTLE   key == 27 // ESC
#define CTRL_OPEN_BAG       key == 'B'
#define CTRL_CLOSE_BAG      key == 27 // ESC

// Will print parsing debug info and parsed data to terminal if defined
// #define VERBOSE_POKEDEX

// Places that pokedex database will be looked for (ending with a slash)
// path 1 begins from root
#define POKEDEX_DB_PATH_1 /share/cs327/pokedex/
// path 2 begins from the $HOME directory
#define POKEDEX_DB_PATH_2 /.poke327/pokedex/
// path 3 begins from root
#define POKEDEX_DB_PATH_3 ./pokedex/

// pokedex database file paths
#define POKEDEX_POKEMON_PATH         pokedex/data/csv/pokemon.csv
#define POKEDEX_MOVES_PATH           pokedex/data/csv/moves.csv
#define POKEDEX_POKEMON_MOVES_PATH   pokedex/data/csv/pokemon_moves.csv
#define POKEDEX_POKEMON_SPECIES_PATH pokedex/data/csv/pokemon_species.csv
#define POKEDEX_POKEMON_STATS_PATH   pokedex/data/csv/pokemon_stats.csv
#define POKEDEX_EXPERIENCE_PATH      pokedex/data/csv/experience.csv
#define POKEDEX_TYPE_NAMES_PATH      pokedex/data/csv/type_names.csv
// pokedex database entry count 
// number of entries = line count - 1; because there is a header line
#define POKEDEX_POKEMON_ENTRIES         386    // 386 pokemon (Gen I-III)
#define POKEDEX_MOVES_ENTRIES           844    // 845 total lines
#define POKEDEX_POKEMON_MOVES_ENTRIES   3971   // move_method_id=1,
                                               // version_group_id=5 (Gen I-III)
#define POKEDEX_POKEMON_SPECIES_ENTRIES 898    // 899 total lines
#define POKEDEX_POKEMON_STATS_ENTRIES   6552   // 6553 total lines
#define POKEDEX_EXPERIENCE_ENTRIES      600    // 601 total lines
#define POKEDEX_TYPE_NAMES_ENTRIES      20     // 20 english types

// World dimensions, world is made up of regions (WORLD_SIZE * WORLD_SIZE)
#define WORLD_SIZE 399

#define FRAMETIME 125000 // in microseconds
#define TICKS_PER_SEC 40
#define FRAMES_PER_SEC (1000000/FRAMETIME)
#define TICKS_PER_FRAME (TICKS_PER_SEC/FRAMES_PER_SEC)

// Printed region dimensions 
#define MAX_ROW 21
#define MAX_COL 80

// The number of biomes that will populate each region
// The first 2 seeds will be grass the next 2 seeds will be clearings after that
// it is randomized.
#define MIN_SEEDS_PER_REGION 6
#define MAX_SEEDS_PER_REGION 12

// Symbols
#define CHAR_BORDER       '%'
#define CHAR_BOULDER      '%'
#define CHAR_TREE         '^'
#define CHAR_CENTER       'C'
#define CHAR_MART         'M'
#define CHAR_PATH         '#'
#define CHAR_GRASS        ':'
#define CHAR_CLEARING     '.'
#define CHAR_MOUNTAIN     '%'
#define CHAR_FOREST       '^'
#define CHAR_PC           '@'
#define CHAR_HIKER        'h'
#define CHAR_RIVAL        'r'
#define CHAR_PACER        'p'
#define CHAR_WANDERER     'w'
#define CHAR_STATIONARY   's'
#define CHAR_RAND_WALKER  'n'
#define CHAR_UNDEFINED    'U'

// Colors
#define CHAR_COLOR_BORDER       COLOR_WHITE
#define CHAR_COLOR_BOULDER      COLOR_WHITE
#define CHAR_COLOR_TREE         COLOR_GREEN 
#define CHAR_COLOR_CENTER       COLOR_RED
#define CHAR_COLOR_MART         COLOR_BLUE
#define CHAR_COLOR_PATH         COLOR_YELLOW
#define CHAR_COLOR_GRASS        COLOR_GREEN
#define CHAR_COLOR_CLEARING     COLOR_GREEN
#define CHAR_COLOR_MOUNTAIN     COLOR_WHITE
#define CHAR_COLOR_FOREST       COLOR_GREEN
#define CHAR_COLOR_PC           COLOR_RED
#define CHAR_COLOR_HIKER        COLOR_YELLOW 
#define CHAR_COLOR_RIVAL        COLOR_RED
#define CHAR_COLOR_PACER        COLOR_CYAN
#define CHAR_COLOR_WANDERER     COLOR_MAGENTA
#define CHAR_COLOR_STATIONARY   COLOR_CYAN
#define CHAR_COLOR_RAND_WALKER  COLOR_MAGENTA
#define CHAR_COLOR_UNDEFINED    COLOR_RED
#define CHAR_COLOR_BACKGROUND   COLOR_BLACK

// The number of trainers that will populate each region
#define NUM_TRAINERS -1 // -1 is used to indicate random number of trainers
#define MIN_TRAINERS 4
#define MAX_TRAINERS 8

#define POKEMON_MIN_LEVEL 1
#define POKEMON_MAX_LEVEL 100
// rates below are 1 in X odds
#define POKEMON_ENCOUNTER_RATE 10
#define POKEMON_SHINY_RATE 8192

// maximum items per item type
#define MAX_ITEMS 999

#endif
