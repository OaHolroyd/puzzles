#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/game_tileset.h"
#include "core/logging.h"
#include "core/tui.h"


#define CELL_WIDTH (7)
#define CELL_HEIGHT (3)


/**
 * User interface wrapper struct.
 */
struct UI {
  WINDOW *win_tiles; // container for the tiles
  WINDOW *grid[SIZE]; // grid of tiles
};


/**
 * Set up the user interface.
 *
 * @param ui The user interface to set up.
 * @return 0 on success, non-zero on failure.
 */
static int ui_setup(struct UI *ui) {
  ui->win_tiles = NULL;
  memset(ui->grid, 0, sizeof(ui->grid));

  /* get the dimensions of the standard screen */
  unsigned int rows, cols;
  getmaxyx(stdscr, rows, cols);

  // TODO: check that the terminal is large enough

  /* set up the color pairs for the TUI */
  // ui_setup_colors();

  /* create the window holding the tile cells */
  ui->win_tiles = win_create(CELL_HEIGHT+2, CELL_WIDTH * SIZE + 2, 0, 0);
  LOG("INFO: created tile window");

  /* add a grid over the top */
  tui_grid(ui->grid, 1, SIZE, CELL_HEIGHT, CELL_WIDTH, 1, 1);
  LOG("INFO: created grid window");

  return 0;
}


/**
 * Shut down all windows handled by the user interface.
 *
 * @param ui The user interface.
 */
static void ui_destroy(struct UI *ui) {
  for (int k = 0; k < SIZE; k++) {
    win_destroy(ui->grid[k]);
    ui->grid[k] = NULL;
  }

  win_destroy(ui->win_tiles);
  ui->win_tiles = NULL;
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
    WINDOW *tile = ui->grid[i];

    LOG("INFO: rendering %c to tile %d", game->letters[i], i);

    /* alternate cell colors */
    // TODO: maybe color by score?
    if (i % 2) {
      //
    } else {
      //
    }
    werase(tile);

    /* display the character and the points value */
    const char letter = toupper(game->letters[i]);
    mvwprintw(tile, 1, (CELL_WIDTH - 1) / 2, "%c", letter);
    wrefresh(tile);
  }
}


int main(int argc, char const *argv[]) {
  /* set up logging */
  log_start("tileset.log");

  /* set up a tileset game state */
  srand(time(NULL));
  struct Game game;
  reset_tileset(&game);

  /* start up the TUI */
  tui_start();

  /* create the game board */
  struct UI ui;
  if (ui_setup(&ui)) {
    LOG("ERROR: failed to set up UI");
    tui_end();
    return 1;
  }

  // use an unimportant plane for key handling
  keypad(ui.grid[0], TRUE); // enable extra keyboard input (arrow keys etc.)

  /* render the screen before starting the game */
  ui_render(&ui, &game);

  /* game loop */
  while (1) {
    const int key = wgetch(ui.grid[0]);
    LOG("INFO: key pressed: %d [%c, %s]", key, key, nc_keystr(key));

    /* special key handling */
    if (key == 'q' || key == 'Q') {
      break;
    }

    /* handle keypress */
    int result = 0;
    switch (key) {
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
  tui_end();

  return 0;
}
