poke: main.c
	gcc main.c -Wall -o poke -lm

clean:
	rm -f *- poke core vgcore.*
