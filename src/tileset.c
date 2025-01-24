#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

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

#define COL_HIDE (214)
#define COL_SHOW (215)
#define COL_SHOWBLANK (216)

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

  WINDOW *win_score; // window for the score
  int has_submitted; // 0 = not submitted, 1 = submitted (coorect), -1 = submitted (incorrect)
  char submitted_word[SIZE + 1]; // submitted word (with space for null-terminator)

  WINDOW *win_target; // window with the target words
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
  init_pair(COL_TILEA, 201, 206);
  init_pair(COL_SCOREA, 204, 206);
  init_pair(COL_TILEA + 1, 200, 202);
  init_pair(COL_SCOREA + 1, 204, 202);

  init_pair(COL_TILEB, 201, 205);
  init_pair(COL_SCOREB, 204, 205);
  init_pair(COL_TILEB + 1, 200, 203);
  init_pair(COL_SCOREB + 1, 204, 203);

  init_pair(COL_INPUT, 200, 201);

  init_pair(COL_ESCOFF, 204, 201);
  init_pair(COL_ESCON, 201, 200);

  init_pair(COL_HIDE, 201, 201);
  init_pair(COL_SHOW, 200, 201);
  init_pair(COL_SHOWBLANK, 204, 201);

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
  ui->win_score = NULL;
  ui->has_submitted = 0;
  memset(ui->grid, 0, sizeof(ui->grid));
  memset(ui->selected, 0, sizeof(ui->selected));
  memset(ui->submitted_word, 0, sizeof(ui->submitted_word));
  for (int i = 0; i < SIZE; i++) {
    ui->input[i] = EMPTY;
  }

  /* get the dimensions of the standard screen */
  unsigned int rows, cols;
  getmaxyx(stdscr, rows, cols);;

  /* check that the terminal is large enough */
  if (cols < CELL_WIDTH * SIZE + 2 || rows < CELL_HEIGHT + 2 + 3 + (STORE + 1) / 2) {
    LOG("ERROR: terminal too small");
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

  /* score window right of the input */
  ui->win_score = win_create(3, SIZE * CELL_WIDTH - SIZE - 4, CELL_HEIGHT + 2, 5 + SIZE);
  LOG("INFO: created score window");

  /* target window beneath the score/intput */
  ui->win_target = win_create((STORE + 1) / 2, (SIZE + 4) * 2 + 2, CELL_HEIGHT + 5, 1);
  LOG("INFO: created target window");

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

  win_destroy(ui->win_score);
  ui->win_score = NULL;

  win_destroy(ui->win_target);
  ui->win_target = NULL;
}


/**
 * Render a target word.
 *
 * @param ui The user interface.
 * @param game The game state.
 * @param y The y position.
 * @param x The x position.
 * @param index The index of the target word.
 */
static void render_target_word(const struct UI *ui, const struct Game *game, const int y, const int x,
                               const int index) {
  WINDOW *win = ui->win_target;
  const int score = game->top_scores[index];
  const char *word = game->top_words[index];
  const int found = game->has_found[index];

  mvwprintw(win, y, x, "%2d: %-7s", score, word);

  if (found) {
    mvwchgat(win, y, x, 11, A_NORMAL, COL_SHOW, NULL); // show the word

    // highligt any blanks in the word
    char blanks[SIZE];
    find_blanks_tileset(game, word, blanks);
    for (int i = 0; i < SIZE; i++) {
      if (blanks[i]) {
        mvwchgat(win, y, x + i + 4, 1, A_NORMAL, COL_SHOWBLANK, NULL); // show the blank
      }
    }
  } else {
    mvwchgat(win, y, x, 11, A_NORMAL, COL_HIDE, NULL); // hide the word
  }
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
  mvwprintw(ui->win_esc, 0, 0, "ESC: [R]eset [S]huffle [G]ive up [Q]uit");
  wrefresh(ui->win_esc);


  /* set the color and text of the tile cells */
  for (int i = 0; i < SIZE; i++) {
    WINDOW *tile = ui->grid[i];

    /* highlight selected tiles */
    const short modifier = ui->selected[i] ? 1 : 0;

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
      const short color_pair = COL_SCOREA + modifier;
      mvwchgat(tile, 2, 4, 2, A_NORMAL, color_pair, NULL); // score color
    } else {
      // short color_pair = COLOR_PAIR(COL_SCOREB + modifier);
      const short color_pair = COL_SCOREB + modifier;
      mvwchgat(tile, 2, 4, 2, A_NORMAL, color_pair, NULL); // score color
    }

    wrefresh(tile);
  }


  /* update the most recent submission */
  werase(ui->win_score);
  if (ui->has_submitted == 0) {
    // clear the most recent submission text
    for (int i = 0; i < SIZE * CELL_WIDTH - SIZE - 4; ++i) {
      mvwaddch(ui->win_score, 0, i, ' ');
    }
  } else if (ui->has_submitted == 1) {
    // display the most recent submission text
    mvwprintw(
      ui->win_score, 0, 1, "CORRECT: %s (%d)", ui->submitted_word, score_word_tileset(game, ui->submitted_word)
    );
  } else if (ui->has_submitted == -1) {
    // display the most recent submission text
    mvwprintw(ui->win_score, 0, 1, "INCORRECT: %s", ui->submitted_word);
  }

  /* update the high-score */
  if (game->score > 0) {
    mvwprintw(ui->win_score, 1, 1, "BEST: %s (%d)", game->word, game->score);
  }

  wrefresh(ui->win_score);


  /* update the target words */
  const int half_store = (STORE + 1) / 2;
  for (int i = 0; i < half_store; i++) {
    render_target_word(ui, game, i, 1, i);

    if (i + half_store < STORE) {
      render_target_word(ui, game, i, 13, i + half_store);
    }
  }
  wrefresh(ui->win_target);


  /* update intput window */
  for (int i = 0; i < SIZE; i++) {
    wmove(ui->win_input, 1, 2 + i);
    waddch(ui->win_input, toupper(ui->input[i]));

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
    const char letter = selected[i];
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
static void delete_index(struct UI *ui, const struct Game *game) {
  const char ch = ui->input[ui->input_index];

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
      if (game->letters[i] == BLANK && ui->selected[i]) {
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
static void enter_letter(struct UI *ui, const struct Game *game, const char letter) {
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
    if (game->letters[i] == BLANK && !ui->selected[i]) {
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


/**
 * Submit the word in the input buffer.
 *
 * @param ui The user interface.
 * @param game The game state.
 */
static void submit_word(struct UI *ui, struct Game *game) {
  /* extract the word from the input */
  memset(ui->submitted_word, 0, sizeof(ui->submitted_word));
  int k = 0;
  for (int i = 0; i < SIZE; i++) {
    if (ui->input[i] != EMPTY) {
      ui->submitted_word[k++] = ui->input[i];
    }
  }

  const int score = submit_word_tileset(game, ui->submitted_word);
  if (score <= 0) {
    /* incorrect word */
    LOG("INFO: incorrect word: %s", ui->submitted_word);
    ui->has_submitted = -1;
    return;
  }

  /* correct word */
  LOG("INFO: correct word: %s", ui->submitted_word);
  ui->has_submitted = 1;
}


/**
 * Parse the command line arguments.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @param seed Pointer to seed to use for the game.
 * @return -1 for help text, 0 on success, non-zero on failure.
 */
static int parse_args(const int argc, char *argv[], char **seed) {
  static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"seed", required_argument, 0, 's'},
    {0, 0, 0, 0}
  };

  const char *help_text = "TILESET: a word game\n"
      "  -h, --help      Display this help and exit\n"
      "  -s, --seed      Set the seed for the game.\n"
      "                  The seed must be a 14-digit hex number.\n"
      "\n"
      "  The aim of the game is to find as many of the top-10 best scoring\n"
      "  words as possible.\n";

  /* parse arguments */
  int c, opt_index;
  int bad_option = 0;
  *seed = NULL;
  while ((c = getopt_long(argc, argv, "hs:", long_options, &opt_index)) != -1) {
    switch (c) {
      case 'h':
        fprintf(stderr, help_text);
        return -1;
      case 's':
        *seed = optarg;
        break;
      case '?':
        bad_option = 1;
        break;
      default:
        break;
    }
  }

  /* check for bad options */
  if (bad_option) {
    fprintf(stderr, "\n%s", help_text);
    return 1;
  }

  /* check for extra (invalid) arguments */
  if (optind < argc) {
    while (optind < argc) {
      fprintf(stderr, "tileset: invalid extra argument: `%s'\n", argv[optind++]);
    }
    fprintf(stderr, "\n%s", help_text);
    return 2;
  }

  return 0;
}


/**
 * Receive user input in a loop.
 *
 * @param ui The user interface.
 * @param game The game state.
 */
static void game_loop(struct UI *ui, struct Game *game) {
  /* render the screen before starting the game */
  ui_render(ui, game);

  /* game loop */
  while (1) {
    const int key = wgetch(ui->win_input);
    if (key == ERR) {
      continue;
    }

    LOG("INFO: key pressed: %d [%c, %s]", key, key, nc_keystr(key));

    if (key == KEY_ESC) {
      /* escape key is special - it toggles between input modes */
      LOG("INFO: toggle escape mode");
      ui->esc_mode = !ui->esc_mode;
    } else if (ui->esc_mode) {
      /* ESC MODE ON */
      if (key == 'q' || key == 'Q') {
        break;
      }

      switch (key) {
        case 'R':
        case 'r':
          memset(ui->selected, 0, SIZE);
          reset_tileset(game, NULL, 1);
          break;
        case 'S':
        case 's':
          shuffle_tiles(ui, game);
          break;
        case 'G':
        case 'g':
          reveal_tileset(game);
          break;
        default:
          // do nothing if key is not valid
          break;
      }
    } else {
      /* ESC MODE OFF */
      /* handle keypress */
      switch (key) {
        case '\n':
        case KEY_ENTER:
          submit_word(ui, game);
          break;
        case KEY_LEFT:
          if (ui->input_index > 0) {
            ui->input_index--;
          }
          break;
        case KEY_RIGHT:
          if (ui->input_index < SIZE - 1) {
            ui->input_index++;
          }
          break;
        case KEY_DEL:
        case KEY_BACKSPACE:
          delete_index(ui, game);
          if (ui->input_index > 0) {
            ui->input_index--;
          }
          break;
        default:
          if (key >= 'a' && key <= 'z') {
            enter_letter(ui, game, key);
          }
          break;
      }
    }

    /* render the screen */
    ui_render(ui, game);
    ui->has_submitted = 0; // reset submition flag
  }
}


int main(const int argc, char *argv[]) {
  /* set up logging */
  log_start("tileset.log");

  /* parse the command line arguments */
  char *seed = NULL;
  if (parse_args(argc, argv, &seed)) {
    return EXIT_FAILURE;
  }

  /* set up a tileset game state */
  srand(time(NULL));
  struct Game game;
  if (reset_tileset(&game, seed, 1)) {
    fprintf(stderr, "tileset: bad seed `%s'\n", seed);
    return EXIT_FAILURE;
  }

  /* start up the TUI */
  tui_start();

  /* create the game board */
  struct UI ui;
  if (ui_setup(&ui)) {
    LOG("ERROR: failed to set up UI");
    tui_end();
    return EXIT_FAILURE;
  }

  // use an unimportant plane for key handling
  tui_keypad(ui.win_input); // enable extra keyboard input (arrow keys etc.)

  game_loop(&ui, &game);

  /* clean up resources */
  ui_destroy(&ui);
  tui_end();

  return EXIT_SUCCESS;
}
