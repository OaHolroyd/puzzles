#include <stdio.h>

#include "core/game_tileset.h"
#include "core/logging.h"
#include "core/nc-utils.h"


#define CELL_WIDTH (7)
#define CELL_HEIGHT (3)


/**
 * User interface wrapper struct.
 */
struct UI {
  struct notcurses *nc;
  struct ncplane *pln_std;
  struct ncplane *pln_tiles; // container for the tiles
  struct ncplane *grid[SIZE]; // grid of tiles
};


/**
 * Set up the user interface.
 *
 * @param nc The notcurses context.
 * @param ui The user interface to set up.
 * @return 0 on success, non-zero on failure.
 */
static int ui_setup(struct notcurses *nc, struct UI *ui) {
  LOG("INFO: ui_setup start ");
  ui->nc = nc;
  ui->pln_std = notcurses_stdplane(nc);
  ui->pln_tiles = NULL;
  ncplane_options opts = {0};
  LOG("INFO: got standard plane");

  /* get the dimensions of the standard plane */
  unsigned int rows, cols;
  ncplane_dim_yx(ui->pln_std, &rows, &cols);

  LOG("pln_std size: %d, %d", rows, cols);

  // TODO: check that the terminal is large enough

  /* create the plane holding the tile cells */
  opts.y = 1;
  opts.x = 1;
  opts.rows = CELL_HEIGHT;
  opts.cols = CELL_WIDTH * SIZE;
  LOG("INFO: about to create tile plane");
  ui->pln_tiles = ncplane_create(ui->pln_std, &opts);
  LOG("INFO: tried to created tile plane");
  if (!ui->pln_tiles) {
    LOG("ERROR: failed to create tile plane");
    return 1;
  }
  LOG("INFO: created tile plane");

  /* add a grid over the top */
  const int err = ncutil_grid(ui->pln_tiles, ui->grid, 1, SIZE, CELL_WIDTH, CELL_HEIGHT);
  if (err) {
    LOG("ERROR: failed to create grid");
    ncplane_destroy(ui->pln_tiles);
    ui->pln_tiles = NULL;
    return 2;
  }
  LOG("INFO: created grid plane");

  return 0;
}


/**
 * Free the memory used by the user interface.
 *
 * @param ui The user interface.
 */
static void ui_destroy(struct UI *ui) {
  for (int k = 0; k < SIZE; k++) {
    if (ui->grid[k]) {
      ncplane_destroy(ui->grid[k]);
      ui->grid[k] = NULL;
    }
  }

  if (ui->pln_tiles) {
    ncplane_destroy(ui->pln_tiles);
  }
  ui->pln_tiles = NULL;
}


/**
 * Render the game board.
 *
 * @param ui The user interface.
 * @param game The game state.
 */
static void ui_render(const struct UI *ui, const struct Game *game) {
  /* set the color and text of the tile cells */
  for (int i = 0; i < SIZE; i++) {
    struct ncplane *tile = ui->grid[i];
    unsigned height, width;
    ncplane_dim_yx(tile, &height, &width);

    LOG("INFO: rendering %c to tile %d", game->letters[i], i);

    /* alternate cell colors */
    // TODO: maybe color by score?
    // if (i % 2) {
    //   ncplane_set_bchannel(tile, 0x40222222);
    //   ncplane_set_fchannel(tile, 0x40FFFFFF);
    // } else {
    //   ncplane_set_bchannel(tile, 0x40000000);
    //   ncplane_set_fchannel(tile, 0x40FFFFFF);
    // }
    // ncutil_fill(tile, ' ');

    /* display the character and the points value */
    const char letter = toupper(game->letters[i]);
    ncplane_putchar_yx(tile, 1, 1, 'A');
  }
}


int main(int argc, char const *argv[]) {
  /* set up logging */
  log_start("tileset.log");

  /* set up a tileset game state */
  srand(time(NULL));
  struct Game game;
  reset_tileset(&game);

  /* start notcurses */
  struct notcurses *nc = ncutil_start();
  LOG("INFO: started notcurses");
  if (nc == NULL) {
    return 1;
  }

  /* create the game board */
  struct UI ui;
  if (ui_setup(nc, &ui)) {
    LOG("ERROR: failed to set up UI");
    notcurses_leave_alternate_screen(nc);
    notcurses_stop(nc);
    return 1;
  }
  LOG("INFO: set up UI");

  /* render the screen before starting the game */
  ui_render(&ui, &game);
  LOG("INFO: first render");

  /* game loop */
  while (1) {
    ncinput input = {0};
    const uint32_t key = notcurses_get_blocking(nc, &input);
    LOG("INFO: key pressed: %d [%c, %s]", key, key, ncu_keystr(key));

    /* special key handling */
    if (key == 'q' || key == 'Q') {
      break;
    }

    /* handle keypress */
    int result = 0;
    switch (key) {
      // // four move directions
      // case NCKEY_DOWN:
      //   result = turn_2048(&game, DOWN);
      // break;
      // case NCKEY_UP:
      //   result = turn_2048(&game, UP);
      // break;
      // case NCKEY_LEFT:
      //   result = turn_2048(&game, LEFT);
      // break;
      // case NCKEY_RIGHT:
      //   result = turn_2048(&game, RIGHT);
      // break;
      // reset the game
      case 'R':
      case 'r':
        reset_tileset(&game);
        result = 1;
        break;
      default:
        // do nothing if key is not valid
        break;
    }

    /* render the screen if there were any updates */
    if (result) {
      ui_render(&ui, &game);
    }
  }

  /* clean up resources */
  ui_destroy(&ui);
  ncutil_end(nc);

  return 0;
}
