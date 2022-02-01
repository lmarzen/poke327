poke: main.c
	gcc main.c -Wall -Werror -o poke

clean:
	rm -f *- poke core
