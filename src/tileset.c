#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/game_tileset.h"
#include "core/logging.h"
#include "core/tui.h"


#define CELL_WIDTH (7)
#define CELL_HEIGHT (3)

#define COL_TILEA (200)
#define COL_TILEB (202)
#define COL_SCOREA (204)
#define COL_SCOREB (206)

#define COL_INPUT (212)

#define COL_ESCOFF (208)
#define COL_ESCON (210)

#define EMPTY (' ')


/**
 * User interface wrapper struct.
 */
struct UI {
  WINDOW *win_tiles; // window for the tiles
  WINDOW *grid[SIZE]; // grid of tiles
  char selected[SIZE]; // 0 = not selected, 1 = selected

  int esc_mode; // 0 = normal, 1 = escape
  WINDOW *win_esc; // window for the escape menu

  WINDOW *win_input; // window for the input
  char input[SIZE]; // input letters
  int input_index; // index of the input
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
  init_hex_color(200, 0xFFFFFF);
  init_hex_color(201, 0x000000);
  init_hex_color(202, 0x101010);
  init_hex_color(203, 0x151515);
  init_hex_color(204, 0x606060);
  init_hex_color(205, 0xEEEEEE);
  init_hex_color(206, 0xF8F8F8);

  /* create the color pairs */
  init_pair(COL_TILEA, 200, 202);
  init_pair(COL_SCOREA, 204, 202);
  init_pair(COL_TILEA + 1, 201, 206);
  init_pair(COL_SCOREA + 1, 204, 206);

  init_pair(COL_TILEB, 200, 203);
  init_pair(COL_SCOREB, 204, 203);
  init_pair(COL_TILEB + 1, 201, 205);
  init_pair(COL_SCOREB + 1, 204, 205);

  init_pair(COL_INPUT, 200, 201);

  init_pair(COL_ESCOFF, 204, 201);
  init_pair(COL_ESCON, 201, 200);

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
  ui->win_input = NULL;
  ui->input_index = 0;
  memset(ui->grid, 0, sizeof(ui->grid));
  memset(ui->selected, 0, sizeof(ui->selected));
  for (int i = 0; i < SIZE; i++) {
    ui->input[i] = EMPTY;
  }

  /* get the dimensions of the standard screen */
  unsigned int rows, cols;
  getmaxyx(stdscr, rows, cols);

  // TODO: check that the terminal is large enough
  if (cols < CELL_WIDTH * SIZE + 2 || rows < CELL_HEIGHT + 2) {
    LOG("ERROR: terminal too narrow");
    return 1;
  }

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

  /* input window beneath the tiles */
  ui->win_input = win_create(3, SIZE + 4, CELL_HEIGHT + 2, 1);
  LOG("INFO: created input window");
  win_border(ui->win_input, BOXLIGHT, A_NORMAL);

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

  win_destroy(ui->win_input);
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
    wbkgd(ui->win_esc, COLOR_PAIR(COL_ESCON));
  } else {
    wbkgd(ui->win_esc, COLOR_PAIR(COL_ESCOFF));
  }
  werase(ui->win_esc);
  mvwprintw(ui->win_esc, 0, 0, "ESC: [R]eset [S]huffle [Q]uit");
  wrefresh(ui->win_esc);

  /* set the color and text of the tile cells */
  for (int i = 0; i < SIZE; i++) {
    WINDOW *tile = ui->grid[i];

    /* highlight selected tiles */
    short modifier = ui->selected[i] ? 1 : 0;

    /* alternate cell colors to distinguish between cells */
    if (i % 2) {
      wbkgd(tile, COLOR_PAIR(COL_TILEA + modifier)); // main tile color
    } else {
      wbkgd(tile, COLOR_PAIR(COL_TILEB + modifier)); // main tile color
    }
    werase(tile);

    /* display the character and the points value */
    mvwprintw(tile, 1, 3, "%c", toupper(game->letters[i]));
    mvwprintw(tile, 2, 4, "%2d", score_letter_tileset(game->letters[i]));

    /* adjust the score color */
    if (i % 2) {
      // short color_pair = COLOR_PAIR(COL_SCOREA + modifier);
      short color_pair = COL_SCOREA + modifier;
      mvwchgat(tile, 2, 4, 2, A_NORMAL, color_pair, NULL); // score color
    } else {
      // short color_pair = COLOR_PAIR(COL_SCOREB + modifier);
      short color_pair = COL_SCOREB + modifier;
      mvwchgat(tile, 2, 4, 2, A_NORMAL, color_pair, NULL); // score color
    }

    wrefresh(tile);
  }

  /* update intput window */
  for (int i = 0; i < SIZE; i++) {
    wmove(ui->win_input, 1, 2 + i);
    waddch(ui->win_input, ui->input[i]);

    // default color
    wchgat(ui->win_input, 1, A_NORMAL, COL_INPUT, NULL); // score color
  }
  // highlight cursor
  mvwchgat(ui->win_input, 1, 2 + ui->input_index, 1, A_REVERSE, COL_INPUT, NULL); // score color

  wrefresh(ui->win_input);
}


/**
 * Shuffle the tiles in the game state, maintaining the selected tiles.
 *
 * @param ui The user interface.
 * @param game The game state.
 */
static void shuffle_tiles(struct UI *ui, struct Game *game) {
  /* record the currently selected tiles */
  char selected[SIZE];
  memset(selected, 0, sizeof(selected));
  int k = 0;
  for (int i = 0; i < SIZE; i++) {
    if (ui->selected[i]) {
      selected[k++] = game->letters[i];
    }
  }

  shuffle_tileset(game);

  /* restore the selected tiles */
  memset(ui->selected, 0, sizeof(ui->selected));
  for (int i = 0; i < k; i++) {
    char letter = selected[i];
    for (int j = 0; j < SIZE; j++) {
      if (game->letters[j] == letter && !ui->selected[j]) {
        ui->selected[j] = 1;
        break;
      }
    }
  }
}


/**
 * Remove the letter under the cursor.
 *
 * @param ui The user interface.
 * @param game The game state.
 */
void delete_index(struct UI *ui, struct Game *game) {
  char ch = ui->input[ui->input_index];

  // can't delete if there is no letter under the index
  if (ch == EMPTY) {
    return;
  }

  // unselect the deleted letter
  int is_blank = 1;
  for (int i = 0; i < SIZE; i++) {
    if (game->letters[i] == ch && ui->selected[i]) {
      ui->selected[i] = 0;
      is_blank = 0;
      break;
    }
  }

  // handle the case that the letter used was a blank
  if (is_blank) {
    for (int i = 0; i < SIZE; i++) {
      if (game->letters[i] == ' ' && ui->selected[i]) {
        ui->selected[i] = 0;
        break;
      }
    }
  }

  // remove it from the inputs
  ui->input[ui->input_index] = EMPTY;
}


/**
 * Enter a letter into the input buffer if it permitted.
 *
 * @param ui The user interface.
 * @param game The game state.
 * @param letter The letter to enter.
 */
void enter_letter(struct UI *ui, struct Game *game, char letter) {
  /* check normal letters */
  for (int i = 0; i < SIZE; i++) {
    if (game->letters[i] == letter && !ui->selected[i]) {
      // letter is available in the tiles so add
      delete_index(ui, game); // remove the letter if it is already in the input
      ui->selected[i] = 1;
      ui->input[ui->input_index] = letter;

      // move to the next input index if possible
      if (ui->input_index < SIZE - 1) {
        ui->input_index++;
      }

      return;
    }
  }

  /* if not found, check for blanks */
  for (int i = 0; i < SIZE; i++) {
    if (game->letters[i] == ' ' && !ui->selected[i]) {
      // letter is available in the tiles so add
      delete_index(ui, game); // remove the letter if it is already in the input
      ui->selected[i] = 1;
      ui->input[ui->input_index] = letter;

      // move to the next input index if possible
      if (ui->input_index < SIZE - 1) {
        ui->input_index++;
      }

      return;
    }
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
  tui_keypad(ui.win_input); // enable extra keyboard input (arrow keys etc.)

  /* render the screen before starting the game */
  ui_render(&ui, &game);

  /* game loop */
  while (1) {
    const int key = wgetch(ui.win_input);
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
          shuffle_tiles(&ui, &game);
          break;
        default:
          // do nothing if key is not valid
          break;
      }
    } else {
      /* ESC MODE OFF */
      /* handle keypress */
      switch (key) {
        case KEY_ENTER:
          // submit_word(&ui, &game);
          break;
        case KEY_LEFT:
          if (ui.input_index > 0) {
            ui.input_index--;
          }
          break;
        case KEY_RIGHT:
          if (ui.input_index < SIZE - 1) {
            ui.input_index++;
          }
          break;
        case KEY_DEL:
        case KEY_BACKSPACE:
          delete_index(&ui, &game);
          if (ui.input_index > 0) {
            ui.input_index--;
          }
          break;
        default:
          if (key >= 'a' && key <= 'z') {
            enter_letter(&ui, &game, key);
          }
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
