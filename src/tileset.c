#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "core/game_tileset.h"
#include "core/logging.h"


int main(int argc, char const *argv[]) {
  /* set up logging */
  log_start("tileset.log");

  /* set up a 2048 game state */
  srand(time(NULL));
  struct Game game;
  reset_tileset(&game);

  fprintf(stderr, "%s\n", game.letters);

  for (int i = 0; i < 10; i++) {
    shuffle_tileset(&game);
    fprintf(stderr, "%s\n", game.letters);
  }

  return 0;
}
