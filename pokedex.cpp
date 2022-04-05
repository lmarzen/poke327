#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>

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
char *pd_type_names[POKEDEX_TYPE_NAMES_ENTRIES];

void init_pd_pokemon() {
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  
	std::vector<std::string> col;
	std::string line, word;
 
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

    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      getline(file, line);
      #ifdef VERBOSE_POKEDEX
      std::cout << line << std::endl;
      #endif

      while(getline(file, line))
      {
        col.clear();
        std::stringstream str(line);

        #ifdef VERBOSE_POKEDEX
        std::cout << line << std::endl;
        #endif
  
        while(getline(str, word, ',')) {
          if(word == "")
            word = "-1";
          col.push_back(word);
        }
        // if last item is empty then the last col will not have gotten parsed
        // by the above while loop
        if (col.size() == 7)
          col.push_back("-1");

        pd_pokemon[i].id = stoi(col[0]);
        strcpy(pd_pokemon[i].identifier, col[1].c_str());
        pd_pokemon[i].species_id = stoi(col[2]);
        pd_pokemon[i].height = stoi(col[3]);
        pd_pokemon[i].weight = stoi(col[4]);
        pd_pokemon[i].base_experience = stoi(col[5]);
        pd_pokemon[i].order = stoi(col[6]);
        pd_pokemon[i].is_default = stoi(col[7]);
        ++i;
      }
    } else {
      ++failed;
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
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  
	std::vector<std::string> col;
	std::string line, word;
 
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

    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      getline(file, line);
      #ifdef VERBOSE_POKEDEX
      std::cout << line << std::endl;
      #endif

      while(getline(file, line))
      {
        col.clear();
        std::stringstream str(line);

        #ifdef VERBOSE_POKEDEX
        std::cout << line << std::endl;
        #endif
  
        while(getline(str, word, ',')) {
          if(word == "")
            word = "-1";
          col.push_back(word);
        }
        // if last item is empty then the last col will not have gotten parsed
        // by the above while loop
        if (col.size() == 14)
          col.push_back("-1");

        pd_moves[i].id = stoi(col[0]);
        strcpy(pd_moves[i].identifier, col[1].c_str());
        pd_moves[i].generation_id = stoi(col[2]);
        pd_moves[i].type_id = stoi(col[3]);
        pd_moves[i].power = stoi(col[4]);
        pd_moves[i].pp = stoi(col[5]);
        pd_moves[i].accuracy = stoi(col[6]);
        pd_moves[i].priority = stoi(col[7]);
        pd_moves[i].target_id = stoi(col[8]);
        pd_moves[i].damage_class_id = stoi(col[9]);
        pd_moves[i].effect_id = stoi(col[10]);
        pd_moves[i].effect_chance = stoi(col[11]);
        pd_moves[i].contest_type_id = stoi(col[12]);
        pd_moves[i].contest_effect_id = stoi(col[13]);
        pd_moves[i].super_contest_effect_id = stoi(col[14]);
        ++i;
      }
    } else {
      ++failed;
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
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
 
	std::vector<std::string> col;
	std::string line, word;
 
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

    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      getline(file, line);
      #ifdef VERBOSE_POKEDEX
      std::cout << line << std::endl;
      #endif

      while(getline(file, line))
      {
        col.clear();
        std::stringstream str(line);

        #ifdef VERBOSE_POKEDEX
        std::cout << line << std::endl;
        #endif
  
        while(getline(str, word, ',')) {
          if(word == "")
            word = "-1";
          col.push_back(word);
        }
        // if last item is empty then the last col will not have gotten parsed
        // by the above while loop
        if (col.size() == 5)
          col.push_back("-1");

        pd_pokemon_moves[i].pokemon_id = stoi(col[0]);
        pd_pokemon_moves[i].version_group_id = stoi(col[1]);
        pd_pokemon_moves[i].move_id = stoi(col[2]);
        pd_pokemon_moves[i].pokemon_move_method_id = stoi(col[3]);
        pd_pokemon_moves[i].level = stoi(col[4]);
        pd_pokemon_moves[i].order = stoi(col[5]);
        ++i;
      }
    } else {
      ++failed;
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
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  
	std::vector<std::string> col;
	std::string line, word;
 
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

    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      getline(file, line);
      #ifdef VERBOSE_POKEDEX
      std::cout << line << std::endl;
      #endif

      while(getline(file, line))
      {
        col.clear();
        std::stringstream str(line);

        #ifdef VERBOSE_POKEDEX
        std::cout << line << std::endl;
        #endif
  
        while(getline(str, word, ',')) {
          if(word == "")
            word = "-1";
          col.push_back(word);
        }
        // if last item is empty then the last col will not have gotten parsed
        // by the above while loop
        if (col.size() == 19)
          col.push_back("-1");

        pd_pokemon_species[i].id = stoi(col[0]);
        strcpy(pd_pokemon_species[i].identifier, col[1].c_str());
        pd_pokemon_species[i].generation_id = stoi(col[2]);
        pd_pokemon_species[i].evolves_from_species_id = stoi(col[3]);
        pd_pokemon_species[i].evolution_chain_id = stoi(col[4]);
        pd_pokemon_species[i].color_id = stoi(col[5]);
        pd_pokemon_species[i].shape_id = stoi(col[6]);
        pd_pokemon_species[i].habitat_id = stoi(col[7]);
        pd_pokemon_species[i].gender_rate = stoi(col[8]);
        pd_pokemon_species[i].capture_rate = stoi(col[9]);
        pd_pokemon_species[i].base_happiness = stoi(col[10]);
        pd_pokemon_species[i].is_baby = stoi(col[11]);
        pd_pokemon_species[i].hatch_counter = stoi(col[12]);
        pd_pokemon_species[i].has_gender_differences = stoi(col[13]);
        pd_pokemon_species[i].growth_rate_id = stoi(col[14]);
        pd_pokemon_species[i].forms_switchable = stoi(col[15]);
        pd_pokemon_species[i].is_legendary = stoi(col[16]);
        pd_pokemon_species[i].is_mythical = stoi(col[17]);
        pd_pokemon_species[i].order = stoi(col[18]);
        pd_pokemon_species[i].conquest_order = stoi(col[19]);
        ++i;
      }
    } else {
      ++failed;
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
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  
	std::vector<std::string> col;
	std::string line, word;
 
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

    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      getline(file, line);
      #ifdef VERBOSE_POKEDEX
      std::cout << line << std::endl;
      #endif

      while(getline(file, line))
      {
        col.clear();
        std::stringstream str(line);

        #ifdef VERBOSE_POKEDEX
        std::cout << line << std::endl;
        #endif
  
        while(getline(str, word, ',')) {
          if(word == "")
            word = "-1";
          col.push_back(word);
        }
        // if last item is empty then the last col will not have gotten parsed
        // by the above while loop
        if (col.size() == 2)
          col.push_back("-1");

        pd_pokemon_stats[i].pokemon_id = stoi(col[0]);
        pd_pokemon_stats[i].stat_id = stoi(col[1]);
        pd_pokemon_stats[i].base_stat = stoi(col[2]);
        pd_pokemon_stats[i].effort = stoi(col[3]);
        ++i;
      }
    } else {
      ++failed;
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
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  
	std::vector<std::string> col;
	std::string line, word;
 
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

    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      getline(file, line);
      #ifdef VERBOSE_POKEDEX
      std::cout << line << std::endl;
      #endif

      while(getline(file, line))
      {
        col.clear();
        std::stringstream str(line);

        #ifdef VERBOSE_POKEDEX
        std::cout << line << std::endl;
        #endif
  
        while(getline(str, word, ',')) {
          if(word == "")
            word = "-1";
          col.push_back(word);
        }
        // if last item is empty then the last col will not have gotten parsed
        // by the above while loop
        if (col.size() == 2)
          col.push_back("-1");

        pd_experience[i].growth_rate_id = stoi(col[0]);
        pd_experience[i].level = stoi(col[1]);
        pd_experience[i].experience = stoi(col[2]);
        ++i;
      }
    } else {
      ++failed;
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
  std::string fname;
  int32_t failed = 0;
  bool success = false;
  int32_t i = 0;
  
	std::vector<std::string> col;
	std::string line, word;
 
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

    std::fstream file (fname, std::ios::in);
    if(file.is_open())
    {
      success = true;
      std::cout << "  Using " << fname << std::endl;

      // skip first line
      getline(file, line);
      #ifdef VERBOSE_POKEDEX
      std::cout << line << std::endl;
      #endif

      while(getline(file, line))
      {
        col.clear();
        std::stringstream str(line);

        #ifdef VERBOSE_POKEDEX
        std::cout << line << std::endl;
        #endif
  
        while(getline(str, word, ',')) {
          if(word == "")
            word = "-1";
          col.push_back(word);
        }
        // if last item is empty then the last col will not have gotten parsed
        // by the above while loop
        if (col.size() == 2)
          col.push_back("-1");

        // only include english type names, language_id == 9
        if (col[1] == "9") {
          // we only care about col 2 since we are assuming the order 
          // corresponds with  type_id
          //strcpy(pd_type_names[i], col[2].c_str());
        }
        ++i;
      }
    } else {
      ++failed;
    }
  }

  if (!success) {
		std::cout << "Error: Failed to find(or open) " 
              << TOSTRING(POKEDEX_TYPE_NAMES_PATH) 
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
}