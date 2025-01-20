//
// Created by Oscar Holroyd on 19/01/2025.
//

#ifndef TUI_H
#define TUI_H

#ifdef NCURSES_WIDECHAR
#undef NCURSES_WIDECHAR
#endif
#define NCURSES_WIDECHAR 1

#include <ncurses.h>

#define KEY_ESC (27)
#define KEY_DEL (127)

typedef enum BoxStyle {
  BOXROUND,
  BOXLIGHT,
  BOXHEAVY,
  BOXDOUBLE,
  BOXASCII,
} BoxStyle;


/**
 * Start up NCURSES with the correct configuration for a TUI.
 */
void tui_start(void);


/**
 * Clean up NCURSES and leave the TUI.
 */
void tui_end(void);


/**
 * Start up color support in NCURSES.
 *
 * @return -1 if color is not supported, the number of available color pairs otherwise.
 */
int tui_start_color(void);


/**
 * Enable the keypad for a window, allowing for extra keyboard input (including the ESC key).
 *
 * @param win The window to enable the keypad for.
 */
void tui_keypad(WINDOW *win);


/**
 * Create a grid of cells inside a plane.
 *
 * @param grid (flattened) array of windows to populate.
 * @param rows The number of rows in the grid.
 * @param cols The number of columns in the grid.
 * @param width The width of each grid cell window.
 * @param height The height of each grid cell window.
 * @param offsety The y-coordinate of the top-left corner of the grid.
 * @param offsetx The x-coordinate of the top-left corner of the grid.
 * @return 0 on success, non-zero on failure.
 */
void tui_grid(WINDOW **grid, int rows, int cols, int height, int width, int offsety, int offsetx);


/**
 * Create a new window.
 *
 * @param height The height of the window.
 * @param width The width of the window.
 * @param starty The y-coordinate of the window.
 * @param startx The x-coordinate of the window.
 * @return The new window.
 */
WINDOW *win_create(int height, int width, int starty, int startx);


/**
 * Destroy a window.
 *
 * @param win The window to destroy.
 */
void win_destroy(WINDOW *win);


/**
 * Draw a border around a window.
 *
 * @param win The window to draw a border around.
 * @param style The style of the border.
 * @param attrs The attributes of the border.
 */
void win_border(WINDOW *win, BoxStyle style, attr_t attrs);


/**
 * Change the RGB value of a color.
 *
 * @param index The index of the color to change.
 * @param hex The new RGB value of the color.
 */
void init_hex_color(short index, uint32_t hex);


/**
 * Return a string representation of a key.
 *
 * @param key The key to get the string for.
 * @return The string representation of the key.
 */
const char *nc_keystr(int key);


#endif //TUI_H
