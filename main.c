#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_ROW 21
#define MAX_COL 80

enum tile {
  ter_boulder,
  ter_tree,
  ter_center,
  ter_mart,
  ter_path,
  ter_grass0,
  ter_grass1,
  ter_grass2,
  ter_grass3,
  ter_clearing0,
  ter_clearing1,
  ter_clearing2,
  ter_clearing3,
  ter_mixed0,
  ter_mixed1,
  ter_mixed2,
  ter_mixed3,
};

void print_region (enum tile (*region)[MAX_COL]) {
  int i,j;

  for (i = 0; i < MAX_ROW; i++) {
    for (j = 0; j < MAX_COL; j++) {
      switch (region[i][j]) {
        case ter_boulder:
          printf("%%");
          break;
        case ter_tree:
          printf("^");
          break;
        case ter_center:
          printf("C");
          break;
        case ter_mart:
          printf("M");
          break;
        case ter_path:
          printf("#");
          break;
        case ter_grass0:
        case ter_grass1:
        case ter_grass2:
        case ter_grass3:
          printf(":");
          break;
        case ter_clearing0:
        case ter_clearing1:
        case ter_clearing2:
        case ter_clearing3:
          printf(".");
          break;
        case ter_mixed0:
        case ter_mixed1:
        case ter_mixed2:
        case ter_mixed3:
          printf(" "); // TODO random  boulder/tree/grass/clearing
          break;
        default:
          printf("U"); // Undefined terrain, should never occur
          break;
      }
    }
    printf("\n");
  }
}

void init_region (enum tile (*region)[MAX_COL]) {
  int i,j;
  struct timeval t;
  gettimeofday(&t, NULL);
  srand(t.tv_usec * t.tv_sec);

  for (i = 0; i < MAX_ROW; i++) {
    for (j = 0; j < MAX_COL; j++) {
      region[i][j] = ter_boulder;
    }
  }
}

int main (int argc, char *argv[])
{

  enum tile region_0[MAX_ROW][MAX_COL];

  init_region(region_0);
  print_region(region_0);


  printf("%d\n", rand());


  return 0;
}