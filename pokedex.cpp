#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>

#include "pokedex.h"

#define VAL(str) #str
#define TOSTRING(str) VAL(str)
#define CONCAT(a,b) VAL(a) VAL(b)

pd_pokemon_t pd_pokemon[POKEDEX_POKEMON_ENTRIES];
pd_move_t pd_moves[POKEDEX_MOVES_ENTRIES];
pd_pokemon_move_t pd_pokemon_moves[POKEDEX_POKEMON_MOVES_ENTRIES];
pd_pokemon_species_t pd_pokemon_species[POKEDEX_POKEMON_SPECIES_ENTRIES];
pd_pokemon_stat_t pd_pokemon_stats[POKEDEX_POKEMON_STATS_ENTRIES];
pd_experience_t pd_experience[POKEDEX_EXPERIENCE_ENTRIES];
char pd_type_names[POKEDEX_TYPE_NAMES_ENTRIES][11];
pd_pokemon_type_t pd_pokemon_types[POKEDEX_POKEMON_TYPES_ENTRIES];


void toupper(char arr[]) {
  for(char* c = arr; (*c = toupper(*c)); ++c);
}

static char *next_token(char *start, char delim)
{
  int32_t i;
  static char *s;

  if (start) {
    s = start;
  }

  start = s;

  for (i = 0; s[i] && s[i] != delim; i++)
    ;
  s[i] = '\0';
  s = s + i + 1;

  return start;
}

void init_pd_pokemon() {
  FILE *f;
  char line[800];
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  char *tmp[9];

  fname = CONCAT(POKEDEX_DB_PATH_1,POKEDEX_POKEMON_PATH);

  while (failed != 3 && !success) {
    if (failed == 1) {
      fname = getenv("HOME");
      fname.append(CONCAT(POKEDEX_DB_PATH_2,POKEDEX_POKEMON_PATH));
    }
    if (failed == 2) {
      fname = CONCAT(POKEDEX_DB_PATH_3,POKEDEX_POKEMON_PATH);
    }
    #ifdef VERBOSE_POKEDEX
    std::cout << "Checking for " << fname << std::endl;
    #endif

    if((f = fopen(fname.c_str(), "r")))
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      fgets(line, 800, f);
      #ifdef VERBOSE_POKEDEX
      std::cout << line;
      #endif

      while(fgets(line, 800, f))
      {
        #ifdef VERBOSE_POKEDEX
        std::cout << line;
        #endif

        tmp[0] = next_token(line, ',');
        tmp[1] = next_token(NULL, ',');
        tmp[2] = next_token(NULL, ',');
        tmp[3] = next_token(NULL, ',');
        tmp[4] = next_token(NULL, ',');
        tmp[5] = next_token(NULL, ',');
        tmp[6] = next_token(NULL, ',');
        tmp[7] = next_token(NULL, ',');
        tmp[8] = next_token(NULL, ',');

        if (atoi(tmp[0]) <= 386) {
          // we only care about cols where id <= 386 (Gen I-III)
          pd_pokemon[i].id = atoi(tmp[0]);
          toupper(tmp[1]);
          strncpy(pd_pokemon[i].identifier, tmp[1], 12);
          pd_pokemon[i].species_id = atoi(tmp[2]);
          pd_pokemon[i].height = atoi(tmp[3]);
          pd_pokemon[i].weight = atoi(tmp[4]);
          pd_pokemon[i].base_experience = atoi(tmp[5]);
          pd_pokemon[i].order = atoi(tmp[6]);
          pd_pokemon[i].is_default = atoi(tmp[7]);
          ++i;
        }
      }
    } else {
      ++failed;
    }
    if (success) {
      fclose(f);
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_POKEMON_PATH) 
              << std::endl;
    exit(-1);
  }
}

void init_pd_moves() {
  FILE *f;
  char line[800];
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  char *tmp;

  fname = CONCAT(POKEDEX_DB_PATH_1,POKEDEX_MOVES_PATH);

  while (failed != 3 && !success) {
    if (failed == 1) {
      fname = getenv("HOME");
      fname.append(CONCAT(POKEDEX_DB_PATH_2,POKEDEX_MOVES_PATH));
    }
    if (failed == 2) {
      fname = CONCAT(POKEDEX_DB_PATH_3,POKEDEX_MOVES_PATH);
    }
    #ifdef VERBOSE_POKEDEX
    std::cout << "Checking for " << fname << std::endl;
    #endif

    if((f = fopen(fname.c_str(), "r")))
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      fgets(line, 800, f);
      #ifdef VERBOSE_POKEDEX
      std::cout << line;
      #endif

      while(fgets(line, 800, f))
      {
        #ifdef VERBOSE_POKEDEX
        std::cout << line;
        #endif
  
        pd_moves[i].id = atoi((tmp = next_token(line, ',')));
        tmp = next_token(NULL, ',');
        toupper(tmp);
        strncpy(pd_moves[i].identifier, tmp, 32);
        tmp = next_token(NULL, ',');
        pd_moves[i].generation_id = *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].type_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].power =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].pp =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].accuracy =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].priority =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].target_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].damage_class_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].effect_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].effect_chance =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].contest_type_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].contest_effect_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_moves[i].super_contest_effect_id =  *tmp ? atoi(tmp) : -1;
        ++i;
      }
    } else {
      ++failed;
    }
    if (success) {
      fclose(f);
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_MOVES_PATH) 
              << std::endl;
    exit(-1);
  }
}

void init_pd_pokemon_moves() {
  FILE *f;
  char line[800];
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  char *tmp[6];

  fname = CONCAT(POKEDEX_DB_PATH_1,POKEDEX_POKEMON_MOVES_PATH);

  while (failed != 3 && !success) {
    if (failed == 1) {
      fname = getenv("HOME");
      fname.append(CONCAT(POKEDEX_DB_PATH_2,POKEDEX_POKEMON_MOVES_PATH));
    }
    if (failed == 2) {
      fname = CONCAT(POKEDEX_DB_PATH_3,POKEDEX_POKEMON_MOVES_PATH);
    }
    #ifdef VERBOSE_POKEDEX
    std::cout << "Checking for " << fname << std::endl;
    #endif

    if((f = fopen(fname.c_str(), "r")))
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      fgets(line, 800, f);
      #ifdef VERBOSE_POKEDEX
      std::cout << line;
      #endif

      while(fgets(line, 800, f))
      {
        #ifdef VERBOSE_POKEDEX
        std::cout << line;
        #endif

        tmp[0] = next_token(line, ',');
        tmp[1] = next_token(NULL, ',');
        tmp[2] = next_token(NULL, ',');
        tmp[3] = next_token(NULL, ',');
        tmp[4] = next_token(NULL, ',');
        tmp[5] = next_token(NULL, ',');

        if (!strcmp(tmp[1],"5") && !strcmp(tmp[3],"1")) {
          // we only care about cols where version_group_id == 5 and
          // pokemon_move_method_id == 1
          pd_pokemon_moves[i].pokemon_id = *tmp[0] ? atoi(tmp[0]) : -1;
          // pd_pokemon_moves[i].version_group_id = *tmp[1] ? atoi(tmp) : -1;
          pd_pokemon_moves[i].move_id = *tmp[2] ? atoi(tmp[2]) : -1;
          // pd_pokemon_moves[i].pokemon_move_method_id = *tmp[3] ? atoi(tmp) : -1;
          pd_pokemon_moves[i].level = *tmp[4] ? atoi(tmp[4]) : -1;
          pd_pokemon_moves[i].order = (*tmp[5] != '\n') ? atoi(tmp[5]) : -1;
          ++i;
        }
      }
    } else {
      ++failed;
    }
    if (success) {
      fclose(f);
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_POKEMON_MOVES_PATH) 
              << std::endl;
    exit(-1);
  }
}

void init_pd_pokemon_species() {
  FILE *f;
  char line[800];
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  char *tmp;

  fname = CONCAT(POKEDEX_DB_PATH_1,POKEDEX_POKEMON_SPECIES_PATH);

  while (failed != 3 && !success) {
    if (failed == 1) {
      fname = getenv("HOME");
      fname.append(CONCAT(POKEDEX_DB_PATH_2,POKEDEX_POKEMON_SPECIES_PATH));
    }
    if (failed == 2) {
      fname = CONCAT(POKEDEX_DB_PATH_3,POKEDEX_POKEMON_SPECIES_PATH);
    }
    #ifdef VERBOSE_POKEDEX
    std::cout << "Checking for " << fname << std::endl;
    #endif

    if((f = fopen(fname.c_str(), "r")))
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      fgets(line, 800, f);
      #ifdef VERBOSE_POKEDEX
      std::cout << line;
      #endif

      while(fgets(line, 800, f))
      {
        #ifdef VERBOSE_POKEDEX
        std::cout << line;
        #endif

        pd_pokemon_species[i].id = atoi((tmp = next_token(line, ',')));
        tmp = next_token(NULL, ',');
        toupper(tmp);
        strncpy(pd_pokemon_species[i].identifier, tmp, 12);
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].generation_id = *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].evolves_from_species_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].evolution_chain_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].color_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].shape_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].habitat_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].gender_rate =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].capture_rate =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].base_happiness =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].is_baby =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].hatch_counter =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].has_gender_differences =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].growth_rate_id =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].forms_switchable =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].is_legendary =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].is_mythical =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].order =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_species[i].conquest_order =  *tmp ? atoi(tmp) : -1;
        ++i;
      }
    } else {
      ++failed;
    }
    if (success) {
      fclose(f);
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_POKEMON_SPECIES_PATH) 
              << std::endl;
    exit(-1);
  }
}

void init_pd_pokemon_stats() {
  FILE *f;
  char line[800];
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  char *tmp;
 
  fname = CONCAT(POKEDEX_DB_PATH_1,POKEDEX_POKEMON_STATS_PATH);

  while (failed != 3 && !success) {
    if (failed == 1) {
      fname = getenv("HOME");
      fname.append(CONCAT(POKEDEX_DB_PATH_2,POKEDEX_POKEMON_STATS_PATH));
    }
    if (failed == 2) {
      fname = CONCAT(POKEDEX_DB_PATH_3,POKEDEX_POKEMON_STATS_PATH);
    }
    #ifdef VERBOSE_POKEDEX
    std::cout << "Checking for " << fname << std::endl;
    #endif

    if((f = fopen(fname.c_str(), "r")))
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      fgets(line, 800, f);
      #ifdef VERBOSE_POKEDEX
      std::cout << line;
      #endif

      while(fgets(line, 800, f))
      {
        #ifdef VERBOSE_POKEDEX
        std::cout << line;
        #endif

        pd_pokemon_stats[i].pokemon_id = atoi((tmp = next_token(line, ',')));
        tmp = next_token(NULL, ',');
        pd_pokemon_stats[i].stat_id = *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_stats[i].base_stat =  *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_stats[i].effort =  *tmp ? atoi(tmp) : -1;
        ++i;
      }
    } else {
      ++failed;
    }
    if (success) {
      fclose(f);
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_POKEMON_STATS_PATH) 
              << std::endl;
    exit(-1);
  }
}

void init_pd_experience() {
  FILE *f;
  char line[800];
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  char *tmp;
 
  fname = CONCAT(POKEDEX_DB_PATH_1,POKEDEX_EXPERIENCE_PATH);

  while (failed != 3 && !success) {
    if (failed == 1) {
      fname = getenv("HOME");
      fname.append(CONCAT(POKEDEX_DB_PATH_2,POKEDEX_EXPERIENCE_PATH));
    }
    if (failed == 2) {
      fname = CONCAT(POKEDEX_DB_PATH_3,POKEDEX_EXPERIENCE_PATH);
    }
    #ifdef VERBOSE_POKEDEX
    std::cout << "Checking for " << fname << std::endl;
    #endif

    if((f = fopen(fname.c_str(), "r")))
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      fgets(line, 800, f);
      #ifdef VERBOSE_POKEDEX
      std::cout << line;
      #endif

      while(fgets(line, 800, f))
      {
        #ifdef VERBOSE_POKEDEX
        std::cout << line;
        #endif

        pd_experience[i].growth_rate_id = atoi((tmp = next_token(line, ',')));
        tmp = next_token(NULL, ',');
        pd_experience[i].level = *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_experience[i].experience =  *tmp ? atoi(tmp) : -1;
        ++i;
      }
    } else {
      ++failed;
    }
    if (success) {
      fclose(f);
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_EXPERIENCE_PATH) 
              << std::endl;
    exit(-1);
  }
}

void init_pd_type_name() {
  FILE *f;
  char line[800];
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  char *tmp[3];
  int32_t len;

  fname = CONCAT(POKEDEX_DB_PATH_1,POKEDEX_TYPE_NAMES_PATH);

  while (failed != 3 && !success) {
    if (failed == 1) {
      fname = getenv("HOME");
      fname.append(CONCAT(POKEDEX_DB_PATH_2,POKEDEX_TYPE_NAMES_PATH));
    }
    if (failed == 2) {
      fname = CONCAT(POKEDEX_DB_PATH_3,POKEDEX_TYPE_NAMES_PATH);
    }
    #ifdef VERBOSE_POKEDEX
    std::cout << "Checking for " << fname << std::endl;
    #endif

    if((f = fopen(fname.c_str(), "r")))
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      fgets(line, 800, f);
      #ifdef VERBOSE_POKEDEX
      std::cout << line;
      #endif

      while(fgets(line, 800, f))
      {
        #ifdef VERBOSE_POKEDEX
        std::cout << line;
        #endif

        tmp[0] = next_token(line, ',');
        tmp[1] = next_token(NULL, ',');
        tmp[2] = next_token(NULL, ',');

        // remove newline character from type name string
        len = strlen(tmp[2]);
        if (len > 0 && tmp[2][len-1] == '\n') 
          tmp[2][len-1] = '\0';

        // only include english type names, language_id == 9
        if (!strcmp(tmp[1],"9") ) {
          // we only care about col 2 since we are assuming the order 
          // corresponds with type_id
          toupper(tmp[2]);
          strncpy(pd_type_names[i], tmp[2], 10);
          ++i;
        }
      }
    } else {
      ++failed;
    }
    if (success) {
      fclose(f);
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_TYPE_NAMES_PATH) 
              << std::endl;
    exit(-1);
  }
}

void init_pd_pokemon_types() {
  FILE *f;
  char line[800];
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  char *tmp;
 
  fname = CONCAT(POKEDEX_DB_PATH_1,POKEDEX_POKEMON_TYPES_PATH);

  while (failed != 3 && !success) {
    if (failed == 1) {
      fname = getenv("HOME");
      fname.append(CONCAT(POKEDEX_DB_PATH_2,POKEDEX_POKEMON_TYPES_PATH));
    }
    if (failed == 2) {
      fname = CONCAT(POKEDEX_DB_PATH_3,POKEDEX_POKEMON_TYPES_PATH);
    }
    #ifdef VERBOSE_POKEDEX
    std::cout << "Checking for " << fname << std::endl;
    #endif

    if((f = fopen(fname.c_str(), "r")))
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      fgets(line, 800, f);
      #ifdef VERBOSE_POKEDEX
      std::cout << line;
      #endif

      while(fgets(line, 800, f))
      {
        #ifdef VERBOSE_POKEDEX
        std::cout << line;
        #endif

        pd_pokemon_types[i].pokemon_id = atoi((tmp = next_token(line, ',')));
        tmp = next_token(NULL, ',');
        pd_pokemon_types[i].type_id = *tmp ? atoi(tmp) : -1;
        tmp = next_token(NULL, ',');
        pd_pokemon_types[i].slot =  *tmp ? atoi(tmp) : -1;
        ++i;
      }
    } else {
      ++failed;
    }
    if (success) {
      fclose(f);
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_POKEMON_TYPES_PATH) 
              << std::endl;
    exit(-1);
  }
}

void init_pd() {
  init_pd_pokemon();
  init_pd_moves();
  init_pd_pokemon_moves();
  init_pd_pokemon_species();
  init_pd_pokemon_stats();
  init_pd_experience();
  init_pd_type_name();
  init_pd_pokemon_types();
}