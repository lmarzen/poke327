TARGET = poke
LIBS = -lm
CC = gcc
CFLAGS = -Wall -ggdb -O3

HEADERS = heap.h
OBJECTS = main.o heap.o

.PHONY: default all clean

all: $(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $@

clean:
	rm -f $(TARGET) *.o core vgcore.*
