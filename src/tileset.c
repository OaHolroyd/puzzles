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
  int esc_mode; // 0 = normal, 1 = escape
  WINDOW *win_esc; // container for the escape menu
};


/**
 * Set up the colors for the user interface.
 *
 * @return 0 on success, non-zero on failure.
 */
static int ui_setup_colors(void) {
  if (tui_start_color() < 256) {
    LOG("ERROR: tileset requires full color support");
    return 1;
  }

  /* create the colors */
  init_hex_color(200, 0xFFFFFF); // white
  init_hex_color(201, 0x000000); // black
  init_hex_color(202, 0x101010); // dark grey
  init_hex_color(203, 0x151515); // less dark grey
  init_hex_color(204, 0x606060); // mid grey
  init_hex_color(205, 0xEEEEEE); // light grey

  /* create the color pairs */
  init_pair(200, 200, 202); // white on dark grey
  init_pair(201, 200, 203); // white on less dark grey
  init_pair(202, 205, 202); // light grey on dark grey
  init_pair(203, 205, 203); // light grey on less dark grey
  init_pair(204, 204, 201); // mid grey on black (ESC mode off)
  init_pair(205, 201, 200); // black on white (ESC mode on)

  return 0;
}


/**
 * Set up the user interface.
 *
 * @param ui The user interface to set up.
 * @return 0 on success, non-zero on failure.
 */
static int ui_setup(struct UI *ui) {
  ui->esc_mode = 0;
  ui->win_tiles = NULL;
  ui->win_esc = NULL;
  memset(ui->grid, 0, sizeof(ui->grid));

  /* get the dimensions of the standard screen */
  unsigned int rows, cols;
  getmaxyx(stdscr, rows, cols);

  // TODO: check that the terminal is large enough

  /* set up the color pairs for the TUI */
  ui_setup_colors();

  /* escape window at the top */
  ui->win_esc = win_create(1, cols, 0, 0);
  LOG("INFO: created esc window");

  /* create the window holding the tile cells */
  ui->win_tiles = win_create(CELL_HEIGHT + 2, CELL_WIDTH * SIZE + 2, 1, 0);
  LOG("INFO: created tile window");

  /* add a grid over the top */
  tui_grid(ui->grid, 1, SIZE, CELL_HEIGHT, CELL_WIDTH, 2, 1);
  LOG("INFO: created grid window");

  return 0;
}


/**
 * Shut down all windows handled by the user interface.
 *
 * @param ui The user interface.
 */
static void ui_destroy(struct UI *ui) {
  win_destroy(ui->win_esc);
  ui->win_esc = NULL;

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
  /* set the escape menu */
  if (ui->esc_mode) {
    wbkgd(ui->win_esc, COLOR_PAIR(205));
  } else {
    wbkgd(ui->win_esc, COLOR_PAIR(204));
  }
  werase(ui->win_esc);
  mvwprintw(ui->win_esc, 0, 0, "ESC: [R]eset [S]huffle [Q]uit");
  wrefresh(ui->win_esc);

  /* set the color and text of the tile cells */
  for (int i = 0; i < SIZE; i++) {
    WINDOW *tile = ui->grid[i];

    /* alternate cell colors */
    // TODO: maybe color by score?
    if (i % 2) {
      wbkgd(tile, COLOR_PAIR(200));
    } else {
      wbkgd(tile, COLOR_PAIR(201));
    }
    werase(tile);

    /* display the character and the points value */
    mvwprintw(tile, 1, (CELL_WIDTH - 1) / 2, "%c", toupper(game->letters[i]));
    mvwprintw(tile, 2, (CELL_WIDTH - 1) / 2, "%d", score_letter_tileset(game->letters[i]));
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
  tui_keypad(ui.grid[0]); // enable extra keyboard input (arrow keys etc.)

  /* render the screen before starting the game */
  ui_render(&ui, &game);

  /* game loop */
  while (1) {
    const int key = wgetch(ui.grid[0]);
    LOG("INFO: key pressed: %d [%c, %s]", key, key, nc_keystr(key));

    if (key == KEY_ESC) {
      /* escape key is special - it toggles between input modes */
      LOG("INFO: toggle escape mode");
      ui.esc_mode = !ui.esc_mode;
    } else if (ui.esc_mode) {
      /* ESC MODE ON */
      if (key == 'q' || key == 'Q') {
        break;
      }

      switch (key) {
        case 'R':
        case 'r':
          reset_tileset(&game);
          break;
        case 'S':
        case 's':
          shuffle_tileset(&game);
          break;
        default:
          // do nothing if key is not valid
          break;
      }
    } else {
      /* ESC MODE OFF */
      /* handle keypress */
      switch (key) {
        case KEY_UP: // escape key
          LOG("INFO: up key pressed");
          break;
        default:
          // do nothing if key is not valid
          break;
      }
    }

    /* render the screen */
    ui_render(&ui, &game);
  }

  /* clean up resources */
  ui_destroy(&ui);
  tui_end();

  return 0;
}
