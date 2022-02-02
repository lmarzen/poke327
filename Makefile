poke: main.c
	gcc main.c -Wall -o poke -lm -O2

clean:
	rm -f *- poke core
