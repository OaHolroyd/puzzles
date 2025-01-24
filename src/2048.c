#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>

#include "core/logging.h"
#include "core/tui.h"
#include "core/game_2048.h"


#define CELL_WIDTH (7)
#define CELL_HEIGHT (3)
#define COLOR_START (200) // start of color pairs (mucks up colors from here up)


/**
 * User interface wrapper struct.
 */
struct UI {
  WINDOW *win_border; // allows for a border around the board
  WINDOW *win_board; // container for the grid
  WINDOW *grid[SIZE * SIZE]; // grid of cells
  int alignment; // 0 = horizontal, 1 = vertical
  WINDOW *win_info; // info plane
  int esc_mode; // 0 = normal, 1 = escape
  WINDOW *win_esc; // container for the escape menu
};


/**
 * Get the color pair for a given value.
 *
 * @param value The value to get the color pair for.
 * @return The color pair.
 */
static chtype color_pair_for_value(const int value) {
  return COLOR_PAIR(value + COLOR_START);
}


/**
 * Set up the colors for the user interface.
 *
 * @return 0 on success, non-zero on failure.
 */
static int ui_setup_colors(void) {
  if (tui_start_color() < 256) {
    LOG("ERROR: 2048 requires full color support");
    return 1;
  }

  static_assert(SIZE == 4, "if SIZE != 4 then the colors are not defined correctly");

  /* define a color array so we can use value as an index */
  // NOTE: the first color is the background color, the second is the text color
  const uint32_t hex_colors[19][2] = {
    // shades of grey up to 1024
    {0x000000, 0x000000}, // 0 (blank)
    {0x202020, 0xFFFFFF}, // 1 (2)
    {0x404040, 0xFFFFFF}, // 2 (4)
    {0x606060, 0xFFFFFF}, // 3 (8)
    {0x808080, 0xFFFFFF}, // 4 (16)
    {0xA0A0A0, 0xFFFFFF}, // 5 (32)
    {0xB0B0B0, 0xFFFFFF}, // 6 (64)
    {0xC0C0C0, 0x000000}, // 7 (128)
    {0xD0D0D0, 0x000000}, // 8 (256)
    {0xE0E0E0, 0x000000}, // 9 (512)
    {0xF0F0F0, 0x000000}, // 10 (1024)

    // colors for 2048 and above
    {0xFFA600, 0xFFFFFF}, // 11 (2048)
    {0xFF4500, 0xFFFFFF}, // 12 (4096)
    {0x8B0000, 0xFFFFFF}, // 13 (8192)
    {0x550A35, 0xFFFFFF}, // 14 (16384)
    {0x6A0DAD, 0xFFFFFF}, // 15 (32768)
    {0x5539EC, 0xFFFFFF}, // 16 (65536)
    {0x0909FF, 0xFFFFFF}, // 17 (131072)

    // special color that should never be reached
    {0x00FFFF, 0xFF0000}, // 18 error
  };

  /* create the colors and combine them into a pair */
  const short num_pairs = 19;
  for (short i = 0; i < num_pairs; i++) {
    init_hex_color(COLOR_START + 2 * i, hex_colors[i][1]);
    init_hex_color(COLOR_START + 2 * i + 1, hex_colors[i][0]);
    init_pair(COLOR_START + i, COLOR_START + 2 * i, COLOR_START + 2 * i + 1);
  }

  /* create escape mode colors */
  init_hex_color(250, 0x000000); // black
  init_hex_color(251, 0xFFFFFF); // white
  init_pair(250, 250, 251); // black on white (ESC mode on)

  init_hex_color(252, 0x606060); // mid grey
  init_pair(251, 252, 250); // mid grey on black (ESC mode off)

  return 0;
}


/**
 * Set up the user interface.
 *
 * @param ui The user interface to set up.
 * @return 0 on success, non-zero on failure.
 */
static int ui_setup(struct UI *ui) {
  ui->win_border = NULL;
  ui->win_board = NULL;
  ui->alignment = -1;
  ui->win_info = NULL;
  memset(ui->grid, 0, sizeof(ui->grid));

  /* get the dimensions of the standard screen */
  unsigned int rows, cols;
  getmaxyx(stdscr, rows, cols);

  /* check if the terminal is large enough for the TUI */
  if (cols >= 2 + CELL_WIDTH * SIZE + 9 && rows >= 2 + CELL_HEIGHT * SIZE) {
    ui->alignment = 0;
  } else if (rows >= 1 + CELL_HEIGHT * SIZE + 4 && cols >= 3 + CELL_WIDTH * SIZE) {
    ui->alignment = 1;
  } else {
    LOG("ERROR: screen is too small for the TUI");
    return 1;
  }

  /* set up the color pairs for the TUI */
  ui_setup_colors();

  /* escape window at the top */
  ui->win_esc = win_create(1, cols, 0, 0);
  LOG("INFO: created esc window");

  /* border around the board */
  ui->win_border = win_create(CELL_HEIGHT * SIZE + 2, CELL_WIDTH * SIZE + 2, 1, 0);
  LOG("INFO: created border window");
  win_border(ui->win_border, BOXLIGHT, A_BOLD);
  wrefresh(ui->win_border);

  /* create the game board */
  ui->win_board = win_create(CELL_HEIGHT * SIZE, CELL_WIDTH * SIZE, 2, 1);
  LOG("INFO: created board window");

  /* decide on horizontal or vertical alignment for the info window (prefer horizontal) */
  static_assert(SIZE == 4, "if SIZE != 4 then the max score width is different");
  if (ui->alignment == 0) {
    ui->win_info = win_create(10, 9, 1, 1 + CELL_WIDTH * SIZE + 1);
  } else {
    ui->win_info = win_create(4, 23, 2 + CELL_HEIGHT * SIZE + 1, 0);
  }
  LOG("INFO: created info window");
  win_border(ui->win_info, BOXLIGHT, A_BOLD);


  /* add a grid over the top */
  tui_grid(ui->grid, SIZE, SIZE, CELL_HEIGHT, CELL_WIDTH, 2, 1);
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

  for (int k = 0; k < SIZE * SIZE; k++) {
    win_destroy(ui->grid[k]);
    ui->grid[k] = NULL;
  }

  win_destroy(ui->win_border);
  ui->win_border = NULL;

  win_destroy(ui->win_board);
  ui->win_board = NULL;

  win_destroy(ui->win_info);
  ui->win_info = NULL;
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
    wbkgd(ui->win_esc, COLOR_PAIR(250));
  } else {
    wbkgd(ui->win_esc, COLOR_PAIR(251));
  }
  werase(ui->win_esc);
  mvwprintw(ui->win_esc, 0, 0, "ESC: [R]eset [Q]uit");
  wrefresh(ui->win_esc);

  /* set the color and text of the grid cells */
  for (int i = 0; i < SIZE; i++) {
    for (int j = 0; j < SIZE; j++) {
      WINDOW *cell = ui->grid[i * SIZE + j];
      const int value = game->grid[i * SIZE + j];

      /* set the color based on the value */
      wbkgd(cell, color_pair_for_value(value));
      werase(cell);

      /* set the text for non-zero cells */
      if (value != 0) {
        // find length of label
        char text[CELL_WIDTH + 1];
        snprintf(text, CELL_WIDTH, "%d", 1 << value);
        mvwprintw(cell, 1, (CELL_WIDTH - (int)strlen(text)) / 2, text);
      }
      wrefresh(cell);
    }
  }

  /* refresh score and turn count */
  mvwprintw(ui->win_info, 1, 1, "%7s", "score");
  mvwprintw(ui->win_info, 2, 1, "%7d", game->score);

  if (ui->alignment == 0) {
    mvwprintw(ui->win_info, 4, 1, "%7s", "turn");
    mvwprintw(ui->win_info, 5, 1, "%7d", game->turn);
  } else {
    mvwprintw(ui->win_info, 1, 9, "%6s", "turn");
    mvwprintw(ui->win_info, 2, 9, "%6d", game->turn);
  }

  /* info window might need game over message */
  if (game->status != PLAYING) {
    if (ui->alignment == 0) {
      mvwprintw(ui->win_info, 7, 2, "GAME");
      mvwprintw(ui->win_info, 8, 2, "OVER");
    } else {
      mvwprintw(ui->win_info, 1, 17, "GAME");
      mvwprintw(ui->win_info, 2, 17, "OVER");
    }
  } else {
    // hide message after game reset
    if (ui->alignment == 0) {
      mvwprintw(ui->win_info, 7, 2, "    ");
      mvwprintw(ui->win_info, 8, 2, "    ");
    } else {
      mvwprintw(ui->win_info, 1, 17, "    ");
      mvwprintw(ui->win_info, 2, 17, "    ");
    }
  }
  wrefresh(ui->win_info);
}


/**
 * Parse the command line arguments.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return -1 for help text, 0 on success, non-zero on failure.
 */
static int parse_args(const int argc, char *argv[]) {
  static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  const char *help_text = "2048: a sliding-tile game\n"
      "  -h, --help      Display this help and exit\n"
      "\n"
      "  Use the arrow keys to slide the tiles. Merge matching tiles together\n"
      "  to get the 2048 tile.\n";

  /* parse arguments */
  int c, opt_index;
  int bad_option = 0;while ((c = getopt_long(argc, argv, "hs:", long_options, &opt_index)) != -1) {
    switch (c) {
      case 'h':
        fprintf(stderr, "%s", help_text);
      return -1;
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


int main(const int argc, char **argv) {
  /* set up logging */
  log_start("2048.log");

  if (parse_args(argc, argv)) {
    return EXIT_FAILURE;
  }

  /* set up a 2048 game state */
  srand((unsigned int)time(NULL));
  struct Game game;
  reset_2048(&game);

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
  tui_keypad(ui.win_info); // enable extra keyboard input (arrow keys etc.)

  /* render the screen before starting the game */
  ui_render(&ui, &game);

  /* game loop */
  while (1) {
    const int key = wgetch(ui.win_info);
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
          reset_2048(&game);
          break;
        default:
          // do nothing if key is not valid
          break;
      }
    } else {
      /* ESC MODE OFF */
      /* handle keypress */
      switch (key) {
        // four move directions
        case KEY_DOWN:
          turn_2048(&game, DOWN);
          break;
        case KEY_UP:
          turn_2048(&game, UP);
          break;
        case KEY_LEFT:
          turn_2048(&game, LEFT);
          break;
        case KEY_RIGHT:
          turn_2048(&game, RIGHT);
          break;
        default:
          // do nothing if key is not valid
          break;
      }
    }

    /* render the screen at the end of every loop */
    ui_render(&ui, &game);
  }

  /* clean up resources */
  ui_destroy(&ui);
  tui_end();

  return EXIT_SUCCESS;
}
