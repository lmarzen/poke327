TARGET = poke
LIBS = -lm -lncurses
CC = gcc
CXX = g++
# CFLAGS = -Wall -O2
CFLAGS = -Wall -g

HEADERS = config.h heap.h region.h pathfinding.h trainer_events.h global_events.h character.h pokedex.h pokemon.h items.h
OBJECTS = main.o heap.o region.o pathfinding.o trainer_events.o global_events.o character.o pokedex.o pokemon.o
.PHONY: default all clean

all: $(TARGET)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c $< -o $@
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBS) -o $@

clean:
	rm -f $(TARGET) *.o core *.core.* vgcore.*
