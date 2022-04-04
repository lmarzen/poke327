#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "pokedex.h"

#define VAL(str) #str
#define TOSTRING(str) VAL(str)
#define CONCAT(a,b) VAL(a) VAL(b)

std::vector<pd_pokemon_t> pd_pokemon;
std::vector<pd_move_t> pd_moves;
std::vector<pd_pokemon_move_t> pd_pokemon_moves;
std::vector<pd_pokemon_species_t> pd_pokemon_species;
std::vector<pd_pokemon_stat_t> pd_pokemon_stats;
std::vector<pd_experience_t> pd_experience;
std::vector<pd_type_name_t> pd_type_name;

void init_pd_pokemon() {
  std::string fname;
  int32_t failed = 0;
  bool success = false;
 
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

        pd_pokemon_t new_pokemon;
        new_pokemon.id = stoi(col[0]);
        new_pokemon.identifier = col[1];
        new_pokemon.species_id = stoi(col[2]);
        new_pokemon.height = stoi(col[3]);
        new_pokemon.weight = stoi(col[4]);
        new_pokemon.base_experience = stoi(col[5]);
        new_pokemon.order = stoi(col[6]);
        new_pokemon.is_default = stoi(col[7]);
        pd_pokemon.push_back(new_pokemon);
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

        pd_move_t new_move;
        new_move.id = stoi(col[0]);
        new_move.identifier = col[1];
        new_move.generation_id = stoi(col[2]);
        new_move.type_id = stoi(col[3]);
        new_move.power = stoi(col[4]);
        new_move.pp = stoi(col[5]);
        new_move.accuracy = stoi(col[6]);
        new_move.priority = stoi(col[7]);
        new_move.target_id = stoi(col[8]);
        new_move.damage_class_id = stoi(col[9]);
        new_move.effect_id = stoi(col[10]);
        new_move.effect_chance = stoi(col[11]);
        new_move.contest_type_id = stoi(col[12]);
        new_move.contest_effect_id = stoi(col[13]);
        new_move.super_contest_effect_id = stoi(col[14]);
        pd_moves.push_back(new_move);
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

        pd_pokemon_move_t new_pokemon_move;
        new_pokemon_move.pokemon_id = stoi(col[0]);
        new_pokemon_move.version_group_id = stoi(col[1]);
        new_pokemon_move.move_id = stoi(col[2]);
        new_pokemon_move.pokemon_move_method_id = stoi(col[3]);
        new_pokemon_move.level = stoi(col[4]);
        new_pokemon_move.order = stoi(col[5]);
        pd_pokemon_moves.push_back(new_pokemon_move);
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

        pd_pokemon_species_t new_pokemon_species;
        new_pokemon_species.id = stoi(col[0]);
        new_pokemon_species.identifier = col[1];
        new_pokemon_species.generation_id = stoi(col[2]);
        new_pokemon_species.evolves_from_species_id = stoi(col[3]);
        new_pokemon_species.evolution_chain_id = stoi(col[4]);
        new_pokemon_species.color_id = stoi(col[5]);
        new_pokemon_species.shape_id = stoi(col[6]);
        new_pokemon_species.habitat_id = stoi(col[7]);
        new_pokemon_species.gender_rate = stoi(col[8]);
        new_pokemon_species.capture_rate = stoi(col[9]);
        new_pokemon_species.base_happiness = stoi(col[10]);
        new_pokemon_species.is_baby = stoi(col[11]);
        new_pokemon_species.hatch_counter = stoi(col[12]);
        new_pokemon_species.has_gender_differences = stoi(col[13]);
        new_pokemon_species.growth_rate_id = stoi(col[14]);
        new_pokemon_species.forms_switchable = stoi(col[15]);
        new_pokemon_species.is_legendary = stoi(col[16]);
        new_pokemon_species.is_mythical = stoi(col[17]);
        new_pokemon_species.order = stoi(col[18]);
        new_pokemon_species.conquest_order = stoi(col[19]);
        pd_pokemon_species.push_back(new_pokemon_species);
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

        pd_pokemon_stat_t new_pokemon_stat;
        new_pokemon_stat.pokemon_id = stoi(col[0]);
        new_pokemon_stat.stat_id = stoi(col[1]);
        new_pokemon_stat.base_stat = stoi(col[2]);
        new_pokemon_stat.effort = stoi(col[3]);
        pd_pokemon_stats.push_back(new_pokemon_stat);
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

        pd_experience_t new_experience;
        new_experience.growth_rate_id = stoi(col[0]);
        new_experience.level = stoi(col[1]);
        new_experience.experience = stoi(col[2]);
        pd_experience.push_back(new_experience);
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
          pd_type_name_t new_type_name;
          new_type_name.type_id = stoi(col[0]);
          // we don't care about col 1 since we know it will be english
          new_type_name.name = col[2];
          pd_type_name.push_back(new_type_name);
        }
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
  std::cout << "Parsing Pokemon Database..."  << std::endl;
  init_pd_pokemon();
  init_pd_moves();
  init_pd_pokemon_moves();
  init_pd_pokemon_species();
  init_pd_pokemon_stats();
  init_pd_experience();
  init_pd_type_name();
}