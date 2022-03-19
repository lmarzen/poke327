TARGET = poke
LIBS = -lm
CC = gcc
CFLAGS = -Wall -ggdb -O2

HEADERS = config.h heap.h region.h pathfinding.h trainer_events.h global_events.h
OBJECTS = main.o heap.o region.o pathfinding.o trainer_events.o global_events.c
.PHONY: default all clean

all: $(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $@

clean:
	rm -f $(TARGET) *.o core vgcore.*
